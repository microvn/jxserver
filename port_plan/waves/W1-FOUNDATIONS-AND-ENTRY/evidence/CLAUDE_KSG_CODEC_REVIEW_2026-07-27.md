# Claude review: KSG codec hypothesis

Reviewer: Claude, read-only session `a3303275`.

Verdict: `FAIL`.

The target loop decrements its local word count before its body, making its
first table index `key + word_count - 1`, the same as candidate. Claude
independently checked the target/candidate table and divisor and ran 400
bit-exact randomized comparisons with no mismatch. The proposed patch would
write an extra four bytes for non-empty inputs and is rejected.

The owner correction is also material: the code is in
`libcommon.a(KSG_EncodeDecode.o)`, while `KG_Socket.o` only references decode
helpers. The old pcap/replay artifacts are not present locally, so their
historical result is not reproducible evidence.
