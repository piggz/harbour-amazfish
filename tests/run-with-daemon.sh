#!/bin/bash
# Start the daemon, then the UI, on the current session bus. Stop the daemon when the UI ends.
# Exits with the exit code of the UI. Used by the ui_screenshots test inside dbus-run-session.
# Usage: run-with-daemon.sh DAEMON UI
set -u
DAEMON=$1
UI=$2

# A new home folder for each run, so the UI starts with the first-run page and an empty database.
HOME=$(mktemp -d "${TMPDIR:-/tmp}/amazfish-home.XXXXXX")
export HOME

# The KDE desktop style reads its colors from kdeglobals; Plasma copies the color scheme there.
SCHEME_FILE=/usr/share/color-schemes/${AMAZFISH_COLOR_SCHEME:-}.colors
if [[ -n ${AMAZFISH_COLOR_SCHEME:-} && -f $SCHEME_FILE ]]; then
    mkdir -p "$HOME/.config"
    cp "$SCHEME_FILE" "$HOME/.config/kdeglobals"
fi

"$DAEMON" &
DAEMON_PID=$!
# The demo device fills the database when it connects; give it time before the UI reads it.
sleep 5
"$UI"
RESULT=$?
kill "$DAEMON_PID"
wait "$DAEMON_PID" 2> /dev/null
exit "$RESULT"
