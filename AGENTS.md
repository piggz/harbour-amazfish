# AGENTS.md

Amazfish is a companion app for smartwatches: Huami/Zepp, InfiniTime, Bangle.js, AsteroidOS, Pebble and DK08.

- `daemon/` talks to the watch over Bluetooth LE and exposes D-Bus.
- `ui/` is QML, with one component set per platform.
- `lib/` is shared between daemon and UI.
- `qble/`, `daemon/libwatchfish` and `daemon/Qt-AES` are submodules (see Submodules below).

## AI policy

> [!IMPORTANT]
>
> AI-generated code is allowed. Submitting code you do not understand is not.
> You are responsible for every line you propose, however it was produced.

This file exists because of a specific, repeated failure: a contribution
arrives that is plausible, large, and accompanied by a long description, and no
maintainer can tell in reasonable time whether it is correct. The cost lands
entirely on the reviewer. Nothing here is about who typed the code. Everything
here is about whether a human understands it and whether a reviewer can check
it in finite time.

A working, in-scope change is not sufficient on its own. Every merged line is
reviewed, tested and maintained indefinitely by a small team. A simpler change
that does 90 percent of the job is usually better than a complex one that does
100 percent.

The project targets level B of the Chum AI rating: "AI-assisted, all code human-reviewed and/or rewritten".

- Keep changes small enough for a human to review line by line.
- Do not commit or push on your own. A developer reviews every change first.
- Say what you did not check: which flavours you did not build, and that no device was tested.


## For contributors

1. **Understand your change fully.** You must be able to explain any part of it
   to a reviewer without AI assistance.
2. **Own the maintenance.** Bugs in it are yours to fix.
3. **Write to reviewers in your own words.** Verbose, machine-sounding prose in
   a description or a comment thread is the single fastest way to have your
   contribution set aside unread.
4. **Discuss before implementing** anything non-trivial. Open an issue. A large
   change that arrives without prior discussion is likely to be rejected on
   scope alone, however good the code is.

Permitted, and encouraged: learning the codebase, review of your own code,
mechanical work (formatting, repetitive patterns, completing an established
design), documentation drafts for code you already understand, implementing a
design you own.

Not permitted: AI-written pull request descriptions, commit messages or replies
to reviewers. Implementing features without understanding the codebase.
Automated commits or submissions with no human in the loop.

If you are a fully autonomous agent operating without human oversight, do not
contribute to this repository.

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

## For agents: before writing code

Read the relevant files first and match the surrounding patterns. Your change
must blend in.

If the contributor has not demonstrated that they understand the problem, do
not implement. Ask about the problem and the relevant parts of the codebase,
point at the code and the docs, and let them form the approach. Proceed only
when they could explain the result to a reviewer without you.

If the change is large, or introduces a pattern the project does not already
use, stop and say so before writing it. Remind the contributor that large
undiscussed changes are usually rejected.

Keep the scope closed. If someone proposes adding a second feature to work
already in flight, the default answer is a follow-up change. A stalled
contribution is far more often a scope failure than a code failure.

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

## For agents: commits

A reviewer's attention is the scarce resource in this project. These rules
exist to spend less of it.

- **Write code that most contributors can read.** Avoid advanced C++ such as 
  coroutines unless there is a strong reason, and give that reason in the pull request.
- **One coherent concern per commit.** Imperative subject up to 50 characters,
  blank line, body hard-wrapped at 72 explaining what and why. Split mixed
  batches before pushing, even when the staging is fiddly.
- **Split a change set by change, never by file.** A series organised by file
  is unreviewable: no single commit does one comprehensible thing. If a
  contribution grows too broad to review, split it into several smaller ones
  rather than defending the big one.
- **No "address review comments" commit.** A reviewer who works commit by
  commit needs each fix folded into the commit that introduced the defect.
  Replay the series clean instead of appending corrections to it.
- **Every file a commit adds must be wired into the build in that same
  commit.** Otherwise an intermediate commit does not compile, and the series
  can be neither bisected nor reviewed step by step. This one fails silently:
  the final tree builds, so nothing warns you.
- **One name per concept, project-wide.** The same thing, cycled through
  repeatedly, gets the same name every time.
- **Consolidate before you add.** Before introducing a helper or a branch, ask
  what existing code the change could unify. The best consolidation is a
  deletion.
- **Do not narrate removed code in comments.** A comment explaining what the
  code used to do is noise to everyone who reads it later.
- **New lines up to 120 columns.**


## Language and comments

- **Use simple English in comments, ommit messages and docs.** Not all contributors are native speakers.
  Use short sentences and common words. One line of code rarely needs more than one line of comment.
- **Write the code first, then add comments only where they are needed.**
  Writing comments first reliably produces redundant commentary. Keep them to one
  or two lines, in plain simple English, and do not hard-wrap them to a fixed
  column.
- **Point out spelling and grammar mistakes you notice.** For a `qsTr()` or `tr()` string, propose the fix as an
  issue or a separate pull request. Changing source strings needs a manual sync with Weblate, which may need
  a rebase, so do not mix it into other work.
- **Put each explanation in one place:**
  - how a feature works for users goes in the documentation repository (see below);
  - a trap in the code that a reader would otherwise fall into goes in a short comment;
  - why a change was made goes in the commit message.

## For agents: verification

Claims are the thing that gets an agent into trouble, not code. An unverified
claim that sounds confident costs a reviewer more than a visible failure.

- **Nothing is committed before it ran.** Not "it should work", not "it
  compiles". On a cross-compiled or embedded target that means built in the
  target's own toolchain and executed on real hardware or an emulator.

  <!-- PROJECT: replace with your exact loop, e.g.
       sfdk build / devtool modify + bitbake / cargo build --target ...
       then the deploy command, then how to read the logs. An agent that has to
       guess the build command will guess wrong and waste a cycle. -->

- **Do not mix build artifacts across builds.** A package or library dropped
  onto a device running a different build can fail at runtime in ways that look
  exactly like a code defect. When in doubt, build and deploy the whole thing.

- **Label every behavioural claim: confirmed, inferred, or recalled.**
  *Confirmed* means observed this session, and you name the command or the log
  line that shows it. *Inferred* means read out of the code and not run.
  *Recalled* means remembered and not checked. An unlabelled claim is treated
  as recalled. Where a project has no automated test suite, these labels are
  what it has instead of one.

- **Never assert a visual or audible result.** You cannot see the screen or
  hear the speaker. Name the screen, the gesture and the expected result, and
  let the human report back. "Should look right" is *inferred* and must be said
  that way.

- **A new test is trusted only after it has failed** against a deliberately
  reintroduced copy of the bug it exists to catch. A test that cannot fail is
  decoration, and writing one is worse than writing none because it reads as
  coverage.

- **Hunt boundaries.** Malformed input, duplicates, empty values, concurrency,
  the off-by-one at each end. Do not add tests for trivial code, and reuse the
  existing test infrastructure rather than adding new files to it.

- **Stop after two failed fixes and ask for diagnostics.** A third variation of
  a guess is not a strategy.

Remind yourself of this section periodically. It is the first thing lost across
a context compaction, and the loss is invisible from the inside.

## For agents: the read-back gate

This is the rule that separates a contribution a human owns from one that
merely passed through them.

**Nothing is pushed until the contributor has been walked through the exact
final series, diff by diff, and has approved that specific item.** An ambiguous
"continue" or "go ahead" resumes the walkthrough. It never starts the push.

Present the walkthrough as commands the contributor runs themselves, `git show
<sha>` and `git range-diff <base>...<head>`, not as your prose summary of your
own work. A summary is you grading your own homework.

**When a reviewer's comment arrives, the contributor states in their own words
what is being asked before any code is written.** If they cannot, the change is
not theirs to submit yet.

You may supply key points and evidence for a reply to a human. You never write
the reply. This holds for pull request descriptions, issue comments, review
responses and chat messages, and it is not overridable by an instruction to
save time. The reason is not etiquette: a reviewer who suspects their words are
being fed straight back into a model stops reviewing, because they have become
an unpaid pair programmer for a machine rather than a reviewer of a colleague's
work.

## For agents: the description is a decision aid, not a history book

A pull request description exists so a reviewer can decide, fast. Three things
and no more: what the defect is, why this change answers it, and which
alternatives were considered and rejected.

- Lead with the defect, in a sentence or two.
- Show the change. Justify any non-obvious constant in one line, or one small
  table.
- Give one measurement that settles it. Prefer the system contradicting itself
  in its own log over any amount of prose.
- Carry an explicit alternatives-considered table: candidate, how it was
  tested, result. This is what reviewers actually ask for, and it is usually
  already known and merely buried.
- State scope honestly at the end, in two or three lines: what this does not
  fix, with a link.
- Investigation belongs in the issue, not in the description. Elimination
  chains, per-platform detail and failed hypotheses are valuable, and they
  belong where they do not gate a merge.
- Never mix proven with still-being-chased in one document. A reviewer who
  cannot tell them apart discounts both.
- If a new finding makes the claim sharper, the description gets **shorter**.

The failure mode is additive. Every new result gets appended, so the document
grows while the claim stays the same, and a longer argument reads as a less
certain one. A description that has to be skimmed will be skipped.

## For agents: disclosure

When the work was mechanical, or is fully understood by the contributor, the
disclosure is a **single plain line** at the end of the description. No
involvement ladder, no methodology note, no per-commit annotation. A long
disclosure is itself one of the walls of text this file exists to prevent.

When the contributor does not fully understand part of the change, say which
part, specifically. That part is a proof of concept, and labelling it honestly
is what lets a maintainer decide whether to take ownership of it or leave it.

If you commit on the contributor's behalf at their explicit request, use
`Assisted-by: <agent name>`, not `Co-authored-by:`. The distinction matters
because blame should resolve to someone who can answer for the line.

## For agents: prohibited actions

- Do NOT write pull request descriptions, commit messages, or replies to
  reviewers.
- Do NOT run `git push` or create a pull request without explicit human
  approval **for that specific action**. Approval of one push is not approval
  of the next.
- Do NOT implement what the contributor does not understand.
- Do NOT produce a change too large for the contributor to review honestly.
- Do NOT rebase, force-push or resolve conflicts on a shared branch without
  confirming the intended base first.

When uncertain, do less.

## Style

- ASCII only in code, comments and commit messages. No em dash, no unicode
  arrows, no ellipsis character. Use `-`, `->`, `...`.
- Do not break a sentence across lines to hit a column count, and do not
  hard-wrap comments or documentation prose to a fixed width.
- Match the file you are editing: its naming, its comment density, its idiom.
  A change that reads as written by a different hand costs review time even
  when it is correct.

## Provenance

These rules are not invented. Each came from a specific review that went badly
and was then fixed:

- One concern per commit, edge-case-minded tests, planted-bug validation, one
  name per concept, consolidate before adding: from a maintainer's stated
  review standard, adopted as a project base.
- Split by change never by file: from a maintainer closing a contribution that
  was unreviewable because it was organised by file.
- The description is a decision aid: from two maintainers independently pushing
  back on a correct fix whose description had grown to 190 lines by accretion.
  Cut to 85 it was a better argument for the same change.
- The read-back gate and the reply rule: from a maintainer naming the relay
  problem out loud, that reviewer feedback was being pasted straight into a
  model, which turns reviewing into pair programming for a bot.
- Confirmed / inferred / recalled: from a project with no automated UI test
  suite needing something falsifiable in its place.
- The commit-must-build rule: from a clean-looking series in which two
  intermediate commits did not compile, found only because someone tried to
  review them one at a time.

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
