#!/bin/sh

UNIT_FILE=$HOME/.config/systemd/user/harbour-amazfish.service

if [ ! -f "$UNIT_FILE" ]; then

    echo "Creating service $UNIT_FILE"
    mkdir -p "$(dirname "$UNIT_FILE")"
    cat <<"EOF" > $UNIT_FILE
[Unit]
Description=Amazfish daemon
After=graphical.target

[Service]
ExecStart=/opt/click.ubuntu.com/harbour-amazfish/current/bin/harbour-amazfishd
Restart=always
RestartSec=5
Environment=LD_LIBRARY_PATH=/opt/click.ubuntu.com/harbour-amazfish/current/lib:/opt/click.ubuntu.com/harbour-amazfish/current/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
Environment=HOME=%h XDG_CONFIG_HOME=/home/%u/.config DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/%U/bus XDG_RUNTIME_DIR=/run/user/%U

EOF

    /usr/bin/systemctl --user enable harbour-amazfish.service
    /usr/bin/systemctl --user start harbour-amazfish.service

else
    /usr/bin/systemctl --user restart harbour-amazfish.service

fi

export LD_LIBRARY_PATH=$PWD/lib:$PWD/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export QT_QUICK_CONTROLS_MOBILE=true
export QT_QUICK_CONTROLS_STYLE=Suru

exec $PWD/bin/harbour-amazfish-ui \
    --desktop_file_hint=${HOME}/.local/share/applications/${APP_ID}.desktop "$@"
