#!/bin/sh
# A-GPS update for the Amazfish custom button action.
#
# Install as ~/harbour-amazfish-script.sh and set a button action to
# "Custom Script" in Amazfish (Settings > Application Settings > Button
# Actions). Amazfish runs this script with the number of button presses; on
# the configured count (3 by default) it downloads fresh A-GPS data with
# huami-token and sends gps_uihh.bin to the watch.
#
# The Amazfit GTS (firmware 0.1.2.x) takes A-GPS data only as the bundled
# gps_uihh.bin; it answers the single cep_pak.bin and gps_alm.bin files with
# "file type not supported".
#
# Plain POSIX sh: Amazfish starts the script with /bin/sh.
# See contrib/agps/README.md for the setup, including the login file.

CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/amazfish-agps"

# Defaults; override them in $CONFIG_DIR/settings (KEY=value lines).
AGPS_DIR="$HOME/.local/share/amazfish-agps"   # download folder, also holds the log
HUAMI_TOKEN="$HOME/huami-token/.venv/bin/huami-token"
LOGIN_FILE="$CONFIG_DIR/zepp-login"           # line 1 e-mail, line 2 password
AGPS_PRESSES=3                                # button presses that start the update
MAX_AGE_DAYS=7                                # older A-GPS data is not sent
TRANSFER_TIMEOUT=300                          # seconds to wait for the transfer

# shellcheck source=/dev/null
[ -r "$CONFIG_DIR/settings" ] && . "$CONFIG_DIR/settings"

LOG="$AGPS_DIR/agps.log"
DBUS_DEST="uk.co.piggz.amazfish"
DBUS_PATH="/application"
DBUS_IFACE="uk.co.piggz.amazfish"

log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$LOG"
}

# D-Bus call to the Amazfish daemon; prints the plain reply value
daemon() {
    method=$1; shift
    dbus-send --session --print-reply=literal --dest="$DBUS_DEST" \
        "$DBUS_PATH" "$DBUS_IFACE.$method" "$@" 2>/dev/null \
        | sed -e 's/^[[:space:]]*//' -e 's/^boolean //' -e 's/^string //' -e 's/^"//' -e 's/"$//'
}

operation_running() {
    [ "$(daemon operationRunning)" = "true" ]
}

wait_idle() {   # wait until no transfer is running
    waited=0
    while operation_running; do
        sleep 2; waited=$((waited + 2))
        [ "$waited" -ge "$TRANSFER_TIMEOUT" ] && return 1
    done
    return 0
}

send_file() {   # send_file <path> <type Amazfish must report>
    file=$1; expected=$2; name=$(basename "$file")
    if ! wait_idle; then
        log "another transfer is still running, giving up"
        return 1
    fi
    version=$(daemon prepareFirmwareDownload "string:$file")
    if [ "$version" != "$expected" ]; then
        log "$name was not accepted (got '$version', expected '$expected')"
        return 1
    fi
    if [ "$(daemon startDownload)" != "true" ]; then
        log "the daemon refused to send $name"
        return 1
    fi
    # the transfer starts asynchronously: wait for it to begin, then to end
    waited=0
    while ! operation_running && [ "$waited" -lt 10 ]; do
        sleep 1; waited=$((waited + 1))
    done
    if ! wait_idle; then
        log "sending $name did not finish within $TRANSFER_TIMEOUT s"
        return 1
    fi
    log "sent $name ($expected)"
    return 0
}

# newest file whose name ends in gps_uihh.bin (a timestamp prefix is allowed)
newest_uihh() {
    # shellcheck disable=SC2012
    ls -t "$AGPS_DIR"/*gps_uihh.bin 2>/dev/null | head -n 1
}

update_agps() {
    cd "$AGPS_DIR" || return 1

    # 1. download with huami-token (login read from LOGIN_FILE, not stored here)
    if [ ! -r "$LOGIN_FILE" ]; then
        log "login file $LOGIN_FILE missing or not readable, using the gps_uihh.bin already there"
    elif [ ! -x "$HUAMI_TOKEN" ]; then
        log "huami-token not found or not executable: $HUAMI_TOKEN"
    else
        email=$(sed -n 1p "$LOGIN_FILE" | tr -d '\r')
        password=$(sed -n 2p "$LOGIN_FILE" | tr -d '\r')
        agps=$(newest_uihh)
        before=$( [ -n "$agps" ] && stat -c %Y "$agps" || echo 0)
        "$HUAMI_TOKEN" -m amazfit -g -e "$email" -p "$password" > huami-token.out 2>&1
        rc=$?
        agps=$(newest_uihh)
        after=$( [ -n "$agps" ] && stat -c %Y "$agps" || echo 0)
        if [ "$rc" -eq 0 ] && [ "$after" != "$before" ]; then
            log "huami-token downloaded new A-GPS data"
        else
            if [ "$rc" -eq 0 ]; then
                log "huami-token finished, but gps_uihh.bin was not written"
            else
                log "huami-token failed (exit code $rc)"
            fi
            # last lines of its output, with the password hidden
            tail -n 8 huami-token.out | awk -v pw="$password" '
                { if (pw != "") while ((i = index($0, pw)) > 0)
                      $0 = substr($0, 1, i - 1) "<password>" substr($0, i + length(pw))
                  print "    huami-token: " $0 }' >> "$LOG"
            log "trying the gps_uihh.bin already there"
        fi
        rm -f huami-token.out
        unset password
    fi

    # 2. gps_uihh.bin must exist and still be recent
    agps=$(newest_uihh)
    if [ -z "$agps" ]; then
        log "no gps_uihh.bin in $AGPS_DIR; files there: $(ls "$AGPS_DIR" | tr '\n' ' ')"
        return 1
    fi
    age=$(( ( $(date +%s) - $(stat -c %Y "$agps") ) / 86400 ))
    if [ "$age" -ge "$MAX_AGE_DAYS" ]; then
        log "$(basename "$agps") is $age days old, its data has expired"
        return 1
    fi
    log "using $(basename "$agps")"

    # 3. send it to the watch (copied under a fixed name, the daemon reads it)
    mkdir -p upload && cp "$agps" upload/gps_uihh.bin || return 1
    send_file "$AGPS_DIR/upload/gps_uihh.bin" "GPS_UIHH" || return 1
    log "A-GPS data updated"
    return 0
}

# The daemon starts this script, so its session bus is normally inherited.
# Otherwise take it from the running daemon.
if [ -z "${DBUS_SESSION_BUS_ADDRESS:-}" ]; then
    pid=$(pidof harbour-amazfishd 2>/dev/null | cut -d' ' -f1)
    if [ -n "$pid" ]; then
        DBUS_SESSION_BUS_ADDRESS=$(tr '\0' '\n' < "/proc/$pid/environ" | sed -n 's/^DBUS_SESSION_BUS_ADDRESS=//p')
        export DBUS_SESSION_BUS_ADDRESS
    fi
fi

if [ "$1" = "$AGPS_PRESSES" ]; then
    mkdir -p "$AGPS_DIR"
    # Ignore further presses while an update is running. The lock holds the
    # process id: if that process is gone (e.g. killed together with the
    # daemon when it restarts), the lock is left over and removed right away.
    lock="$AGPS_DIR/.running"
    if [ -d "$lock" ]; then
        holder=$(cat "$lock/pid" 2>/dev/null)
        if [ -n "$holder" ] && kill -0 "$holder" 2>/dev/null; then
            log "update already running (process $holder)"
            exit 0
        fi
        log "removing lock left over from an interrupted run"
        rm -rf "$lock"
    fi
    if ! mkdir "$lock" 2>/dev/null; then
        log "update already running"
        exit 0
    fi
    echo $$ > "$lock/pid"
    trap 'rm -rf "$lock"' EXIT
    log "button pressed $1 times: updating A-GPS data"
    update_agps
fi
