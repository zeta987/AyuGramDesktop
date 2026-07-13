# Issue tracker: GitHub

Issues and PRDs for this repository live in GitHub Issues at
`zeta987/AyuGramDesktop`. Use the `gh` CLI and always pass the repository
explicitly so a command cannot target an upstream remote by accident.

## Repository boundary

- Writable issue tracker: `zeta987/AyuGramDesktop` only.
- Never create an issue or pull request in `AyuGram/AyuGramDesktop`,
  `telegramdesktop/tdesktop`, `desktop-app/*`, or another upstream project.
- Never push to an upstream remote. For code that belongs to another
  repository or submodule, create or reuse a `zeta987` fork first.
- Create a pull request only when the user explicitly requests one. A pull
  request in a `zeta987` repository must state that it was AI generated.

## Conventions

- Create an issue:
  `gh issue create --repo zeta987/AyuGramDesktop --title "..." --body-file <file>`
- Read an issue:
  `gh issue view <number> --repo zeta987/AyuGramDesktop --comments`
- List issues:
  `gh issue list --repo zeta987/AyuGramDesktop --state open`
- Comment on an issue:
  `gh issue comment <number> --repo zeta987/AyuGramDesktop --body-file <file>`
- Apply or remove labels:
  `gh issue edit <number> --repo zeta987/AyuGramDesktop --add-label "..."`
  or `--remove-label "..."`
- Close an issue:
  `gh issue close <number> --repo zeta987/AyuGramDesktop --comment "..."`

Use a temporary UTF-8 Markdown file and `--body-file` for multi-line issue
bodies. Do not place a PowerShell here-string directly in a one-line command.

## Skill terminology

When a skill says "publish to the issue tracker", create a GitHub issue in
`zeta987/AyuGramDesktop`. When it says "fetch the relevant ticket", run
`gh issue view <number> --repo zeta987/AyuGramDesktop --comments`.
