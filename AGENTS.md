# AGENTS.md

Amazfish is a companion app for smartwatches: Huami/Zepp, InfiniTime, Bangle.js, AsteroidOS, Pebble and DK08.

- `daemon/` talks to the watch over Bluetooth LE and exposes D-Bus.
- `ui/` is QML, with one component set per platform.
- `lib/` is shared between daemon and UI.
- `qble/`, `daemon/libwatchfish` and `daemon/Qt-AES` are submodules (see Submodules below).

## AI policy

The project targets level B of the Chum AI rating: "AI-assisted, all code human-reviewed and/or rewritten".

- Keep changes small enough for a human to review line by line.
- Do not commit or push on your own. A developer reviews every change first.
- Say what you did not check: which flavours you did not build, and that no device was tested.

## Platforms and Qt versions

| Flavour    | Platform                | Qt                                        |
|------------|-------------------------|-------------------------------------------|
| `silica`   | Sailfish OS 4.6         | 5.6                                       |
| `uuitk`    | Ubuntu Touch 24.04-1.x  | 5.15 (5.12 on 20.04), moving to Qt 6      |
| `kirigami` | Flatpak, KDE 5.15-24.08 | 5.15, moving to Qt 6                      |

`qtcontrols` is only a fallback set of QML components. It is not a build target; do not build or test with it.

- C++ must build with every Qt from 5.6 to 6. Use `#if QT_VERSION >= QT_VERSION_CHECK(...)` for newer APIs.
- QML must run on Qt 5.6: use `var`, not `let` or `const`. No arrow functions, no template strings.
- Include every header you use. A missing include may build on one target and fail on another.
- A change in shared QML or in `lib/` must work in all three flavours.
- Platform-specific QML goes in `ui/qml/components/platform.<flavour>/`.

## Build and check

CMake only. The flavour is required: `cmake -DFLAVOR=silica|uuitk|kirigami`.

Run the CI jobs locally in Docker. The commands are in `.github/workflows/`:

- `sailfishos.yml` — Sailfish OS SDK image, `mb2 build` for armv7hl, aarch64, i486.
- `ubuntu-touch.yml` — `clickable build --all` in the `clickable/ci-ut24.04-1.x-<arch>` image.
- `flatpak.yml` — `flatpak-builder` with the KDE 5.15 SDK.
- `clang-format-check.yml` — formatting of changed lines.

Build every flavour your change touches. There is no test suite yet. New tests are welcome.

Testing on a real watch is often not possible. When you cannot test, say so, and describe what a
developer should check on the device.

## Formatting

- New files and new functions: follow `.clang-format` (WebKit).
- Old code: keep the style around it. Never reformat existing lines. `git blame` must stay useful.
- The clang-format CI job may then fail on old code. That is known and accepted.

## Code

- Write code that most contributors can read. Avoid advanced C++ such as coroutines unless there is a strong
  reason, and give that reason in the pull request.
- New lines up to 120 columns.

## Language and comments

- Use simple English in comments, commit messages and docs. Not all contributors are native speakers.
  Short sentences, common words.
- Point out spelling and grammar mistakes you notice. For a `qsTr()` or `tr()` string, propose the fix as an
  issue or a separate pull request. Changing source strings needs a manual sync with Weblate, which may need
  a rebase, so do not mix it into other work.
- Keep comments short. One line of code rarely needs more than one line of comment.
- Put each explanation in one place:
  - how a feature works for users goes in the documentation repository (see below);
  - a trap in the code that a reader would otherwise fall into goes in a short comment;
  - why a change was made goes in the commit message.

## Documentation

User documentation lives in https://github.com/amazfish/amazfish.github.io. When a change affects users,
also prepare the matching change there, or say what needs updating.

The device feature tables come from `harbour-amazfishd --features`
(`DeviceFactory::printAvailableFeatures()` in `daemon/src/devicefactory.cpp`). It writes one `<Device>.md` per
device into the current directory, with `Y` or `N` for each feature and data type.

The output only shows what `supportedFeatures()` and `supportedDataTypes()` declare. It cannot show partial
support: a watch firmware may implement a feature only in part, or only in some versions. When a feature is
added or changed, check what really works on each watch and correct the tables by hand. If you cannot check
it, say so.

## Submodules

Submodules may be changed, but the change must go as a pull request to the submodule's own project first.
Then update the submodule pointer here in a separate commit.

- `qble` (https://github.com/piggz/qble) and `libwatchfish` (https://github.com/piggz/libwatchfish) have the
  same maintainers as Amazfish.
- `Qt-AES` (https://github.com/bricke/Qt-AES) is a third-party project. A change there can take a long time.

## Resources

- After adding or removing a QML file or PNG under `ui/qml`, run `./update-qrc.sh` in `ui/`.
- Do not edit `ui/platform-*.qrc` or `ui/icons.qrc` by hand.

## Adding a device

A new device usually touches:

- `daemon/src/devices/<x>device.{h,cpp}`
- its services in `daemon/src/services/`
- `daemon/CMakeLists.txt`
- `daemon/src/devicefactory.cpp`
- `ui/qml/components/DeviceListModel.qml`
- `ui/qml/pics/devices/<x>.png`, then `update-qrc.sh`

## Translations

- Wrap user-visible strings in `qsTr()` or `tr()`.
- Do not edit `*/translations/*.ts`. Weblate and the maintainers run `lupdate` in separate commits.

## Git

- Main branch: `master`. One topic branch per pull request.
- Split work into several pull requests, one per feature. Each should be reviewable on its own.
- Every commit subject starts with a component in brackets:
  `[Bangle.js] Limit notification length`, `[InfiniTime] ...`, `[ui] ...`, `[daemon] ...`, `[Ubuntu Touch] ...`.
  The Chum build makes the changelog with `git-change-log`, which only takes subjects with a `[component]` prefix.
  A commit without it is missing from the changelog.
- After the prefix: a short, imperative sentence. Check the spelling.
- Submodule updates go in their own commit, e.g. `[qble] Update submodule`.
- Do not edit `rpm/*.changes.in`.
