#!/bin/sh

export LD_LIBRARY_PATH=$PWD/lib:$PWD/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export QT_QUICK_CONTROLS_MOBILE=true
export QT_QUICK_CONTROLS_STYLE=Suru

exec $PWD/bin/harbour-amazfish-ui \
    --desktop_file_hint=${HOME}/.local/share/applications/${APP_ID}.desktop "$@"