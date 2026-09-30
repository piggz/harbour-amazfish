# A-GPS update with a button press

`harbour-amazfish-script.sh` downloads fresh A-GPS data from the Zepp servers and
sends it to the watch when you press the watch button three times. With current
A-GPS data the watch finds a GPS fix much faster.

It uses [huami-token](https://codeberg.org/argrento/huami-token) to download the
data with your Zepp account, and the Amazfish daemon to send it to the watch.

Tested with an Amazfit GTS (firmware V0.1.2.03) on Sailfish OS 5.1. The GTS takes
A-GPS data only as the bundled `gps_uihh.bin`, which is what the script sends.

## Requirements

- An Amazfish build that contains the fixes for sending files to Huami watches
  (committed together with this script). Older builds crash when a transfer
  starts, or report a successful transfer as failed.
- A Zepp (Amazfit) account. The watch does not have to be paired with the Zepp
  app; the account is only used to download the A-GPS data.
- A terminal on the phone (developer mode, or SSH).

## 1. Install huami-token

huami-token needs Python 3.10 or newer (`python3 --version`).

```sh
python3 -m venv ~/huami-token/.venv
~/huami-token/.venv/bin/pip install huami-token
```

This is the default location the script expects. If you install huami-token
somewhere else, set `HUAMI_TOKEN` in the settings file (step 4).

Zepp changes its login from time to time. If the download stops working, update
huami-token first: `~/huami-token/.venv/bin/pip install -U huami-token`.

## 2. Create the login file

The script reads your Zepp login from `~/.config/amazfish-agps/zepp-login`:
line 1 is the e-mail address, line 2 the password. Nothing else goes into the
file.

Create it with these commands in the terminal (they need bash, the default
shell of the Sailfish OS terminal). The password is not shown while you type it
and does not end up in the shell history. `umask 077` makes the file private
from the moment it is created; the `chmod` also fixes an existing file:

```sh
mkdir -p ~/.config/amazfish-agps
read -r -p "Zepp e-mail: " m
read -r -s -p "Zepp password: " p; echo
(umask 077; printf '%s\n%s\n' "$m" "$p" > ~/.config/amazfish-agps/zepp-login)
unset m p
chmod 600 ~/.config/amazfish-agps/zepp-login
```

Check the file: `ls -l ~/.config/amazfish-agps/zepp-login` must show
`-rw-------`, that is, readable only by you.

About the password:

- It is stored in plain text in this file, protected only by the file
  permissions. Use a password that you do not use anywhere else.
- huami-token takes the password as a command line argument. While it runs
  (a few seconds), other programs of your user can read its command line.
- The script never writes the password to its log. If huami-token prints it in
  an error message, the script replaces it with `<password>`.
- Never share or commit this file. If the password was exposed, change it in
  your Zepp account and update the file.

## 3. Test the download

Before involving the watch, check that huami-token works with your login:

```sh
mkdir -p ~/.local/share/amazfish-agps && cd ~/.local/share/amazfish-agps
~/huami-token/.venv/bin/huami-token -m amazfit -g \
    -e "$(sed -n 1p ~/.config/amazfish-agps/zepp-login)" \
    -p "$(sed -n 2p ~/.config/amazfish-agps/zepp-login)"
ls -l *gps_uihh.bin
```

A file ending in `gps_uihh.bin` must appear. If huami-token reports a login
error, check the login file and update huami-token.

## 4. Install the script

Amazfish runs `~/harbour-amazfish-script.sh` for the custom button action:

```sh
cp contrib/agps/harbour-amazfish-script.sh ~/harbour-amazfish-script.sh
```

Optional settings go into `~/.config/amazfish-agps/settings`, one `KEY=value`
per line. All of them have defaults:

| Setting | Default | Meaning |
|---|---|---|
| `AGPS_DIR` | `~/.local/share/amazfish-agps` | Download folder; also holds the log `agps.log` |
| `HUAMI_TOKEN` | `~/huami-token/.venv/bin/huami-token` | Path to huami-token |
| `LOGIN_FILE` | `~/.config/amazfish-agps/zepp-login` | Login file from step 2 |
| `AGPS_PRESSES` | `3` | Number of button presses that start the update |
| `MAX_AGE_DAYS` | `7` | A-GPS data older than this is not sent |
| `TRANSFER_TIMEOUT` | `300` | Seconds to wait for the transfer to the watch |

Example:

```sh
HUAMI_TOKEN=/home/defaultuser/Downloads/huami-token/.venv/bin/huami-token
AGPS_PRESSES=4
```

If you already use `~/harbour-amazfish-script.sh` for other button actions,
keep your script and call this one from it, for example as
`sh /path/to/agps-script.sh "$1"`. It only acts on its own press count.

## 5. Set the button action in Amazfish

In Amazfish open **Settings > Application Settings > Button Actions**, set
**Triple Press Action** (or the count you configured) to **Custom Script** and
tap **Save Settings**.

## 6. Use it

Press the watch button three times while the watch is connected. The script
downloads the data and sends it to the watch; the whole update takes about a
minute. After sending, the watch needs around 40 seconds to process the data
before it confirms. Amazfish shows "Update operation complete" when it is done.

Check the result in the log:

```sh
tail ~/.local/share/amazfish-agps/agps.log
```

A successful run ends with:

```
... huami-token downloaded new A-GPS data
... using gps_uihh.bin
... sent gps_uihh.bin (GPS_UIHH)
... A-GPS data updated
```

A-GPS data covers about a week. Update it every few days, or before you go out
with GPS.

## Troubleshooting

| Log message | Meaning and fix |
|---|---|
| `login file ... missing or not readable` | Create the login file (step 2). |
| `huami-token not found or not executable` | Install huami-token (step 1) or set `HUAMI_TOKEN`. |
| `huami-token failed (exit code ...)` | The next lines show its error. Usually a wrong login or a Zepp change: check the login file, update huami-token. |
| `... was not written` | huami-token ran but saved no `gps_uihh.bin`. Run it by hand (step 3) to see why. |
| `... days old, its data has expired` | The download failed and the file there is too old. Fix the download. |
| `was not accepted (got '', ...)` | The daemon does not know the file, or another transfer is still pending. Restart the daemon: `systemctl --user restart harbour-amazfish`. |
| `sending ... did not finish` | The transfer did not complete. Keep the watch close to the phone and try again. |
| `update already running` | A previous update is still in progress. Wait for it to finish. |
| No log at all | The button action is not set to Custom Script, or the script is not at `~/harbour-amazfish-script.sh`. |
