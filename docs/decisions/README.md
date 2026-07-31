# Architecture decisions

This directory holds ADRs (Architecture Decision Records) for decisions that are hard to reverse, condition future work, or introduce a real tradeoff worth remembering later — e.g. the carousel-launcher rework in `843728c`, or a future choice like adding BLE back for pairing.

Do not write an ADR for routine or easily-reversible changes (a new mini-app, a UI tweak, a sprite update).

## Format

Create `ADR-NNN-short-title.md` using:

```markdown
# ADR-NNN: Title

## Status
Proposed | Accepted | Deprecated | Superseded

## Context
Problem or situation that requires a decision.

## Decision
What was decided.

## Alternatives considered
Relevant alternatives and why they were rejected.

## Consequences
Positive and negative consequences.

## References
Related specs/, Plane issues, or docs.
```

No ADRs exist yet — this directory is ready for the first one.
