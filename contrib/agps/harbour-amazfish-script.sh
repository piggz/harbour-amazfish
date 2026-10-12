#!/bin/sh
# A-GPS update for the Amazfish "Custom Script" button action.
#
# Amazfish runs ~/harbour-amazfish-script.sh with the number of button
# presses. On three presses this script downloads fresh A-GPS data with
# huami-token and hands gps_uihh.bin to the daemon, which sends it to the
# watch and reports the result. See contrib/agps/README.md for the setup.

AGPS_DIR="$HOME/.local/share/amazfish-agps"
HUAMI_TOKEN="$HOME/huami-token/.venv/bin/huami-token"
LOGIN_FILE="$HOME/.config/amazfish-agps/zepp-login"   # line 1 e-mail, line 2 password

[ "$1" = "3" ] || exit 0

mkdir -p "$AGPS_DIR" && cd "$AGPS_DIR" || exit 1
exec >> agps.log 2>&1
echo "$(date '+%F %T') updating A-GPS data"

# ignore further presses while an update is running
exec 9> .lock
flock -n 9 || { echo "update already running"; exit 0; }

email=$(sed -n 1p "$LOGIN_FILE") && password=$(sed -n 2p "$LOGIN_FILE") || exit 1
# huami-token output is not logged: it may contain the password
if ! "$HUAMI_TOKEN" -m amazfit -g -e "$email" -p "$password" > /dev/null 2>&1; then
    echo "huami-token failed, see README.md"
    exit 1
fi

# huami-token may put a timestamp in front of the file name
agps=$(ls -t ./*gps_uihh.bin 2>/dev/null | head -n 1)
[ -n "$agps" ] || { echo "no gps_uihh.bin downloaded"; exit 1; }

dbus-send --session --print-reply --dest=uk.co.piggz.amazfish /application \
    uk.co.piggz.amazfish.prepareFirmwareDownload "string:$AGPS_DIR/${agps#./}"
dbus-send --session --print-reply --dest=uk.co.piggz.amazfish /application \
    uk.co.piggz.amazfish.startDownload
