# W1 workflow retrospective — 2026-07-27

## Verdict

W1 did produce useful target facts and rejected unsafe changes, but the
delivery loop is not yet efficient.  It has repeatedly treated a broad
end-to-end static closure as the prerequisite to measuring the next runtime
boundary.  That made research documents and state synchronization grow faster
than tested source changes, while the visible result remains short of a
hash-pinned candidate enter-world pass.

This is a workflow finding, not a claim that the target evidence or
cross-model review requirements were unnecessary.

## Evidence from the recorded run

1. The historical P3 candidate reached boot/map milestones, but its source
   provenance was `OBSERVED_NOT_LINKED`; this correctly prevented acceptance.
2. The 11-file B1--B4 overlay was reviewed as `REQUEST_CHANGES` because the
   Relay enum and a 3-byte section-check contract were wrong.  The corrected
   R1 static review was only `PASS WITH CAVEATS`, so a broad overlay was not a
   reliable route to a live result.
3. A selected-role run exposed a missing resident `(16,1)` scene.  Exact stock
   later entered world with the valid hash; therefore earlier plain-path VPS
   "stock" diagnoses were invalid.  This shows that provenance and a paired
   runtime arm must precede behavioural conclusions.
4. The proposed KSG codec change was rejected by Claude: target and candidate
   have the same decrement-before-loop behaviour and the proposed extra dword
   would write out of bounds.  The earlier root-cause claim was withdrawn.
5. The current State frontmatter says to investigate KSG receive framing,
   while its Resume packet says to run the `(1,1)` scene probe.  The existing
   validator accepted that contradiction, leaving the next session without a
   single frontier.
6. The W1 Atlas audit record names Codex as both producer and reviewer, despite
   the current skill requiring an opposite-family Atlas audit.  It is therefore
   orientation material, not a completed cross-model coherence gate.
7. The Timing table still reports patch/review/build/runtime as zero or not
   started although the checkpoint history records each.  It cannot support
   the requested wall-clock optimization until it records elapsed work by
   experiment rather than by broad phase.

## Root causes

- The Wave is modeled as five independently accepted foundations before the
  integrated journey, but several of those boundaries are only observable
  during the journey.  Diagnosis was therefore implicitly blocked by the
  acceptance graph.
- Static evidence expanded from the desired terminal state backwards instead
  of from the first measured divergence forwards.
- The B1--B4 overlay joined several hypotheses in shared Relay/player/protocol
  hotspots.  Review could reject concrete ABI faults, but runtime could not
  attribute success or failure to one semantic transition.
- The ledger has duplicate historical checkpoints and status/frontier updates
  that do not always converge.  It accurately preserves history but is too
  permissive as an execution cursor.
- Effort accounting is phase-shaped rather than experiment-shaped, hiding time
  spent in invalid provenance, stale captures, failed review dispatches, and
  withdrawn hypotheses.

## Adopted operating change

For W1 and later Waves, use a **first-divergence expedition** before broad
research whenever a live client failure is available.  Preserve one paired
stock/candidate arm, record the last shared and first divergent milestones,
then research and patch only the immediate boundary.  Each patch is a vertical
fault slice and must pass the existing opposite-model review, build provenance,
and paired probe.  This does not weaken any acceptance gate; it removes
unmeasured hypotheses from the implementation queue.

The skill and roadmap now make this loop explicit.  The validator now rejects
a State whose frontmatter and Resume packet nominate different next actions.

## Immediate W1 recovery

1. Revalidate the exact stock/candidate/runtime topology and decide which is
   actually earlier: the retained scene-resolution probe or a fresh first-GS-
   packet capture.  Update State, WAVE, and the owning card to one frontier.
2. Run one paired selection using the retained/proven role seed; retain raw
   GS/Center/client-boundary artifacts and active binary hashes.
3. If scene lookup is the first divergence, make the next target closure only
   `LoadBaseInfo -> GetScene` and the owning topology/data decision.  If the
   first packet fails earlier, make it only the receive framing/dispatch
   closure.  Do not reopen account/ready/initial-sync dossiers first.
4. Only after that boundary is proven divergent, issue one small write lease,
   review, build, and repeat the same paired probe.
