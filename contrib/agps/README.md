# A-GPS update with a button press

`harbour-amazfish-script.sh` downloads fresh A-GPS data from the Zepp servers
when you press the watch button three times, and hands it to the Amazfish
daemon, which sends it to the watch. With current A-GPS data the watch finds a
GPS fix much faster.

The data is downloaded with [huami-token](https://codeberg.org/argrento/huami-token)
and your Zepp account.

Tested with an Amazfit GTS (firmware V0.1.2.03) on Sailfish OS 5.1. The GTS
takes A-GPS data only as the bundled `gps_uihh.bin`, which is what the script
sends.

## Requirements

- A Zepp (Amazfit) account. The watch does not have to be paired with the Zepp
  app; the account is only used to download the A-GPS data.
- A terminal on the phone (developer mode, or SSH).

## 1. Install huami-token

huami-token needs Python 3.10 or newer (`python3 --version`):

```sh
python3 -m venv ~/huami-token/.venv
~/huami-token/.venv/bin/pip install huami-token
```

If you install it somewhere else, change `HUAMI_TOKEN` at the top of the
script. If the download stops working, update huami-token first:
`~/huami-token/.venv/bin/pip install -U huami-token`.

## 2. Create the login file

The script reads your Zepp login from `~/.config/amazfish-agps/zepp-login`:
line 1 is the e-mail address, line 2 the password.

Create it in the terminal (bash). The password is not shown while you type it
and does not end up in the shell history:

```sh
mkdir -p ~/.config/amazfish-agps
read -r -p "Zepp e-mail: " m
read -r -s -p "Zepp password: " p; echo
(umask 077; printf '%s\n%s\n' "$m" "$p" > ~/.config/amazfish-agps/zepp-login)
unset m p
```

`ls -l ~/.config/amazfish-agps/zepp-login` must show `-rw-------`.

The password is stored in plain text, protected only by the file permissions,
so use one that you do not use anywhere else. huami-token takes it as a command
line argument, so other programs of your user can see it for the few seconds
huami-token runs. The script does not log huami-token's output. Never share
this file; if the password was exposed, change it in your Zepp account.

## 3. Test the download

```sh
mkdir -p ~/.local/share/amazfish-agps && cd ~/.local/share/amazfish-agps
~/huami-token/.venv/bin/huami-token -m amazfit -g \
    -e "$(sed -n 1p ~/.config/amazfish-agps/zepp-login)" \
    -p "$(sed -n 2p ~/.config/amazfish-agps/zepp-login)"
ls -l *gps_uihh.bin
```

A file ending in `gps_uihh.bin` must appear. On a login error, check the login
file and update huami-token.

## 4. Install the script and set the button action

```sh
cp contrib/agps/harbour-amazfish-script.sh ~/harbour-amazfish-script.sh
```

In Amazfish open **Settings > Application Settings > Button Actions**, set
**Triple Press Action** to **Custom Script** and tap **Save Settings**.

If you already use `~/harbour-amazfish-script.sh` for other actions, call this
script from yours instead: `sh /path/to/agps-script.sh "$1"`.

## 5. Use it

Press the watch button three times while the watch is connected. The download
takes a few seconds; after the transfer the watch needs about 40 seconds to
process the data. Amazfish shows "Update operation complete" when it is done.

The script logs to `~/.local/share/amazfish-agps/agps.log`, including the
daemon's replies:

- `prepareFirmwareDownload` answers with the detected file type, which must be
  `GPS_UIHH`. An empty answer means the daemon did not accept the file.
- `startDownload` answers `true` when the transfer was started.

A-GPS data covers about a week. Update it every few days, or before you go out
with GPS.

## Troubleshooting

| Log or behaviour | Fix |
|---|---|
| `huami-token failed` | Run the download by hand (step 3) to see why: wrong login or a Zepp change. |
| `no gps_uihh.bin downloaded` | huami-token ran but saved no `gps_uihh.bin`. Run it by hand (step 3). |
| `update already running` | A previous press is still being handled. Wait a minute. |
| `startDownload` answers `false` | Another transfer is still running, or the watch is not connected. Try again later. |
| No log at all | Set the button action to Custom Script; the script must be `~/harbour-amazfish-script.sh`. |
