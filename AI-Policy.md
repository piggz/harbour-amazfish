# AI-Assisted Contributions Policy

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


## For Contributors

1. You **MAY** use AI assistance for contributing to this project, as long as you follow the
   principles described below.
2. Accountability: You **MUST** take the responsibility for your contribution. Contributing means
   vouching for the quality, license compliance, and utility of your submission. All contributions,
   whether from a human author or assisted by large language models (LLMs) or other generative AI
   tools, must meet the project’s standards for inclusion. The contributor is always the author
   and is fully accountable for the entirety of these contributions.
   - **Understand your change fully.** You must be able to explain any part of it
   to a reviewer without AI assistance.
   - **Own the maintenance.** Bugs in it are yours to fix.
3. Transparency: You **MUST** disclose the use of AI tools when a significant part of the contribution
   is taken from a tool without changes. You SHOULD disclose the other uses of AI tools, where it
   might be useful. Routine use of assistive tools for correcting grammar and spelling, or for
   clarifying language, does not require disclosure.
   Disclosures are made where authorship is normally indicated. For contributions tracked in git, the recommended method is an Assisted-by: commit message trailer. For other contributions, disclosure may include document preambles, design file metadata, translation notes, or wiki page categories.
   Examples:
    - Assisted-by: generic LLM chatbot
    - Assisted-by: ChatGPTv5
4. **Write to reviewers in your own words.** Verbose, machine-sounding prose in
   a description or a comment thread is the single fastest way to have your
   contribution set aside unread.
5. **Discuss before implementing** anything non-trivial. Open an issue. A large
   change that arrives without prior discussion is likely to be rejected on
   scope alone, however good the code is.
6. **Permitted, and encouraged:** learning the codebase, review of your own code,
   mechanical work (formatting, repetitive patterns, completing an established
   design), documentation drafts for code you already understand, implementing a
   design you own.
7. **Not permitted:** AI-written pull request descriptions, commit messages or replies
   to reviewers. Implementing features without understanding the codebase.
   Automated commits or submissions with no human in the loop.
8. **Fully autonomous agents operating without human oversight are not to
   contribute to this repository.**

## Pull request descriptions

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

### Disclosure

When the work was mechanical, or is fully understood by the contributor, the
disclosure is a **single plain line** at the end of the description. No
involvement ladder, no methodology note, no per-commit annotation.

When the contributor does not fully understand part of the change, say which
part, specifically. That part is a proof of concept, and labelling it honestly
is what lets a maintainer decide whether to take ownership of it or leave it.

`Assisted-by:` is used instead of `Co-authored-by:` because blame should
resolve to someone who can answer for the line.

## Why the agent rules exist

A reviewer's attention is the scarce resource in this project. The rules in
AGENTS.md exist to spend less of it. An unverified claim that sounds confident
costs a reviewer more than a visible failure.

Agents do not write replies to humans. The reason is not etiquette: a reviewer
who suspects their words are being fed straight back into a model stops
reviewing, because they have become an unpaid pair programmer for a machine
rather than a reviewer of a colleague's work.

Each rule came from a specific review that went badly and was then fixed:

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

## For Agents

1. Use AGENTS.md for instructions to an agent.
