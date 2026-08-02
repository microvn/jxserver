# FND-RUNTIME-INITIALIZATION — candidate P3 provenance

This manifest deliberately distinguishes a runnable artifact from a fresh,
source-reproducible build. It is not a claim that the current dirty source tree
produced the candidate.

## Artifact identity

| Item | Value |
|---|---|
| Candidate name | `SO3GameServer_w1_p3_candidate` |
| Active path | `/deploy/SO3GameServer_w1_p3_candidate` inside the `jx3gs-wave3` container |
| Remote copy | `/root/jx3/镜像端/extracted/root/SO3GameServer_w1_p3_candidate` |
| SHA-256 | `1854b8aa5be8fe933d8b7e2fdcfb80a04667675b9e2588e7567459cfab441c10` |
| Size | `7,093,896` bytes |
| Remote mtime | `2026-07-22 18:39:44 UTC` |
| Active executable | `/proc/10/exe` resolves to `/deploy/SO3GameServer_w1_p3_candidate` |
| Active-process SHA | same as candidate SHA above |

## Historical build location

The matching build output was found at:

```text
/root/jx3/linux-build-wave3/SO3GameServer
```

That directory is not a Git worktree (`git rev-parse HEAD` fails), and its
`BUILD-TECHNICAL.md` describes the general native-x86 build pipeline but does
not contain a complete source-file hash closure for this exact output.

Therefore the provenance status is:

```text
artifact_status: RUNNABLE_HASH_PINNED
source_reproducibility: NOT_PROVEN
canonical_source_link: OBSERVED_NOT_LINKED
acceptance_use: runtime evidence only
```

## Runtime evidence linked to this artifact

| Observation | Evidence |
|---|---|
| AI rows loaded | `25,627` in boot log |
| Skills loaded | `6,161` in boot log |
| Settings | `Load game settings ... [OK]` |
| Center | connection `[OK]` |
| Listener | service `3113` started `[OK]` |
| Startup | GameServer startup `[OK]` |
| Map | map creation `[OK]` |
| Fresh log | SHA-256 `cc553868b494f8a0d1ad3e729213cedd92f15d8b5b80812e8b0f83e9f8f2a3cb` |

## What is still required for build acceptance

1. Rebuild from the canonical accepted source closure (or a recorded isolated
   worktree) on native Linux x86.
2. Record the exact source HEAD, accepted-file manifest, overlay allowlist,
   compiler/link command, build log, Build ID, and produced SHA-256.
3. Re-run the same stock/candidate boot oracle and prove the active process
   hash equals the newly produced hash.

Until those items exist, this artifact may support a runtime comparison but may
not be cited as proof that the current source tree builds the accepted P3 code.
