# FND-CLIENT-GS-SECURE-TRANSPORT — current evidence boundary

## What is proven

The historical Wave 1b transport artifact
`SO3GameServer_wave1b_transport` (SHA-256
`f1a81adf20a62b277730fff9cb4de373bce302a73d76d07294a986c673a39512`) was
clean-built natively for i386 with the real `libcommon.a` security socket
path. Its bounded oracle called `ConnectSecurity(..., KSG_ENCODE_DECODE)`,
sent a correctly-sized `C2S_HANDSHAKE_REQUEST` (`sizeof=26`), and produced:

- `SECURITY_CONNECT_OK SEND=1`;
- GS `CheckPackage` and protocol-size validation;
- no ciphertext/out-of-range protocol rejection;
- only the expected synthetic invalid role/GUID `Apply data error`.

Target security symbols are pinned in `docs/waves/wave-1b/STATE.md`:
`ConnectSecurity`, `_RecvSecurityKey`, `KG_SecuritySocketStream`, `Encrypt`,
`Decrypt`, `KSG_DecodeEncode`, and `_SendSecurityKey`.

## Combined P3 + transport arm

The P3 tree was clean-built again with its current `build.sh`. The build log
reported `202/202`, `fail=0`, link exit `0`, and `0` undefined references. The
link command uses the same SHA-pinned `libs/libcommon.a` as the accepted
historical transport build; only `crc32_shim` is compiled from
`src/common_recon`.

```text
0df22836e682954d2606adef9b7ff6c0c4209ee783cdeeb540f194d27782833a
```

It is active as `/proc/9/exe` inside `jx3gs-wave3` under the supervisor's
candidate path. The fresh boot log is
`SO3GameServer_2026_07_24_17_12_17.log`, SHA-256
`5368d512f29da7b2d2baccf1e3021a04fd6c07abad4e93d60d85a0267209ef93`.
It reaches `25627 AI`, `6161 skill`, settings `[OK]`, listener `3113`, and
GameServer startup `[OK]`.

This closes the build/link/provider and cold-boot portions of the transport
card. A real client packet capture is still required for the final
client-to-GS decoded-protocol assertion; the historical synthetic oracle is
not silently reused as a current packet capture.

## Required next measurement

Run the bounded client/security oracle against active hash
`0df22836…82833a` and record the first server frame, handshake result, decoded
client protocol ID, and GS route. Do not modify protocol IDs or add permissive
parsing before that capture.

## 2026-07-28 static frontier

The follow-up target/candidate comparison did not find a decisive difference in
raw socket framing or `KG_Packager::PackUp`. It did find concrete contract drift
at `KPlayerServer::ProcessPackage -> CheckPackage`: protocol bounds,
registration-table bases, and surrounding object/state offsets differ between
target and candidate. See
`transport/KPLAYER_CHECKPACKAGE_DRIFT_2026-07-28.md` and its raw disassembly
files. This is static evidence only. The next measurement must record the
decoded length/protocol and first failed predicate at that boundary before any
protocol-table or ABI edit.
