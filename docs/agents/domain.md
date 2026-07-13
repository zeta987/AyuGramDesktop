# Domain docs

This repository uses a single-context domain-document layout.

## Before exploring

Read the following files when they exist and are relevant to the task:

- `CONTEXT.md` at the repository root for project vocabulary and domain rules.
- `docs/adr/` for architectural decisions that affect the area being changed.
- `docs/ayugram-local-development.md` for the local Windows build, branch,
  fork, tag, and release procedures.

If `CONTEXT.md` or `docs/adr/` does not exist, proceed silently. The producer
skill creates those files only when project terminology or an architectural
decision has actually been established.

## Vocabulary

Use terms exactly as defined in `CONTEXT.md` in issue titles, hypotheses,
tests, refactor proposals, and pull request text. If a required concept has no
documented term, note the gap for a future domain-document session instead of
inventing competing terminology.

## ADR conflicts

If proposed work contradicts an existing ADR, identify that ADR explicitly
and explain the conflict before changing the implementation.
