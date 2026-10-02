# AGENTS.md

**This file contains specific instructions for AI agents when contributing to this project.**

Amazfish is a companion app for smartwatches: Huami/Zepp, InfiniTime, Bangle.js, AsteroidOS, Pebble and DK08.

- `daemon/` talks to the watch over Bluetooth LE and exposes D-Bus.
- `ui/` is QML, with one component set per platform.
- `lib/` is shared between daemon and UI.
- `qble/`, `daemon/libwatchfish` and `daemon/Qt-AES` are submodules (see Submodules below).


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

## Before writing code

- Read the relevant files first. Match the surrounding patterns.
- If the contributor does not understand the problem, do not implement. Ask questions, point at the code and docs, and let them form the approach. Go on only when they could explain the result to a reviewer without you.
- If the change is large, or adds a pattern the project does not use yet, stop and say so first. Large undiscussed changes are usually rejected.
- Keep the scope closed. A second feature is a follow-up change, not part of this one.

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

## Commits

- **Write code that most contributors can read.** No advanced C++ such as coroutines without a strong reason, stated in the pull request.
- **One concern per commit.** Imperative subject up to 50 characters, blank line, body wrapped at 72.
- **Split by change, never by file.** Each commit must do one thing a reviewer can understand.
- **No "address review comments" commit.** Fold each fix into the commit that introduced the defect.
- **Every new file is wired into the build in the same commit.** Each commit must compile on its own; the final tree hides this.
- **One name per concept, project-wide.**
- **Consolidate before you add.** Look for existing code to reuse or unify first.
- **Do not describe removed code in comments.**

## Language and comments

- **Use simple English in comments, commit messages and docs.** Not all contributors are native speakers. Short sentences, common words.
- **Write the code first, then comment only where needed.** One or two lines, not hard-wrapped.
- **Point out spelling and grammar mistakes you notice.** A fix to a `qsTr()` or `tr()` string goes in a separate issue or pull request, because it needs a manual Weblate sync.
- **Put each explanation in one place:** user-facing behaviour in the documentation repository, a trap in the code in a short comment, the reason for a change in the commit message.

## Verification

- **Nothing is committed before it ran.** "It compiles" is not enough. On a cross-compiled target, build with its own toolchain and run on hardware or an emulator.
- **Label every behavioural claim: confirmed, inferred, or recalled.** *Confirmed*: observed this session; name the command or log line. *Inferred*: read from the code, not run. *Recalled*: from memory, not checked. An unlabelled claim counts as recalled.
- **Never assert a visual or audible result.** Name the screen, the gesture and the expected result, and let the human report back.
- **Trust a new test only after it has failed** against a deliberately reintroduced copy of the bug.
- **Hunt boundaries:** malformed input, duplicates, empty values, concurrency, off-by-one at each end. No tests for trivial code.
- **Stop after two failed fixes and ask for diagnostics.**

## Read-back gate

- Nothing is pushed until the contributor has seen the exact final series, diff by diff, and approved it. A vague "continue" or "go ahead" continues the walkthrough; it does not start the push.
- Give the walkthrough as commands the contributor runs: `git show <sha>`, `git range-diff <base>...<head>`. Not as your own summary.
- When a reviewer comment arrives, the contributor first says in their own words what is asked. Only then write code.
- If the contributor does not fully understand a part of the change, tell them which part, so they can label it as a proof of concept.

## Prohibited actions

- Do NOT write pull request descriptions, commit messages, issue comments or replies to reviewers, even to save time. You may give key points and evidence. See `AI-Policy.md` for why.
- Do NOT run `git push` or create a pull request without explicit approval **for that specific action**. Approval of one push is not approval of the next.
- Do NOT implement what the contributor does not understand.
- Do NOT produce a change too large for the contributor to review honestly.
- Do NOT rebase, force-push or resolve conflicts on a shared branch without confirming the base first.
- If you commit at the contributor's explicit request, use `Assisted-by: <agent name>`, not `Co-authored-by:`.

When uncertain, do less.

## Style

- **ASCII only in code, comments and commit messages.** No em dash, no unicode
  arrows, no ellipsis character. Use `-`, `->`, `...`.
- **Do not break a sentence across lines to hit a column count**, and do not
  hard-wrap comments or documentation prose to a fixed width.
- **Match the file you are editing** its naming, its comment density, its idiom.
  A change that reads as written by a different hand costs review time even
  when it is correct.
- **New 'code' lines up to 120 columns.**
- New files and new functions: follow `.clang-format` (WebKit).
- Old code: keep the style around it. Never reformat existing lines. `git blame` must stay useful.
- The clang-format CI job may then fail on old code. That is known and accepted.


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
