# Canonical P3 source baseline

## Purpose

This is the reproducible **source** foundation for future W1 entry overlays.
It replaces the unversioned remote `linux-build-wave3` tree as a source
authority. It does not assert that the resulting binary has passed enter-world.

## Pinned source

- Git commit: `1516b8b0832bb071f9d3cc0e773136a6e0962208`
- Commit subject: `build(wave-1b): enforce accepted source baseline`
- Canonical worktree: `.worktrees/canonical-w1-p3` (detached)
- Tree identity: identical to the current repository commit `4e60f169`; the
  two commits have the same Git tree, but this worktree intentionally pins the
  original Wave-1b acceptance commit.

## Verified closure

- Manifest: `docs/waves/wave-1b/BASELINE_MANIFEST.sha256`
- File list: `docs/waves/wave-1b/BASELINE_FILES.txt`
- Check: `BASELINE_OK checked=19`
- Protected closure: accepted P1/P2/P3 loader/AI source owners, including the
  P2 action/Lua-constant correction and P3 `AITypeList` loading path.

## Allowed use

Build future candidate trees as a small, reviewable overlay on this worktree.
Before source review, build, and overlay synchronization, rerun the manifest
check. An entry overlay may not silently replace or omit any of the 19 files.

The initial empty entry overlay is `.worktrees/overlay-w1-entry`, branch
`w1-entry-overlay`, based directly on this commit. Its baseline manifest also
passes `19/19`. It contains no B1--B4 source patch yet.

## Explicit limits

This baseline proves only the Wave-1b cold-boot/data-loader closure recorded in
its historical ledger. It is not evidence that the P3 binary, any later
transport build, or an entry-world candidate reaches `gsPlaying`. Those require
separate source-overlay provenance and paired runtime acceptance.
