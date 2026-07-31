# Specs

Plane (project SGOTCHI) is the board of record for what's in progress and who owns it — see `CLAUDE.md` § Plane Workflow. This directory holds the *versioned technical detail* for issues that are more than a small, self-contained fix: new mini-apps, changes that touch multiple core files, or anything where the design isn't obvious from the code.

Trivial fixes (typo, single-file bug fix, sprite tweak) don't need a folder here — just reference the Plane issue when you do the work.

## Structure

```text
specs/
└── NNN-short-name/
    ├── spec.md    # what must happen and why — required
    ├── plan.md    # how it will be implemented — only if the design isn't obvious
    └── tasks.md   # breakdown into small, verifiable steps — only for multi-step work
```

Number folders sequentially (`001-`, `002-`, ...). Link the Plane issue ID at the top of `spec.md` so the two stay connected.

## `spec.md` template

```markdown
# Feature: Name

## Status
Draft | Ready | In progress | Completed

## Plane issue
SGOTCHI-NNN

## Objective
What this must accomplish.

## Functional requirements
- FR-001: verifiable requirement.

## Acceptance criteria
- Observable criterion.

## Out of scope
- Explicitly excluded.
```

## `plan.md` template

```markdown
# Implementation plan

## Specification
See spec.md

## Current state
How it works today.

## Proposed design
What changes and why.

## Files likely to change
- path

## Test strategy
How this will be verified (pio run + what needs on-device checking).
```

## `tasks.md` template

```markdown
# Tasks

- [ ] T001 Description
  - Files: path
  - Validation: ./scripts/verify.sh + manual check on device
```

No specs exist yet — this directory is ready for the first non-trivial feature.
