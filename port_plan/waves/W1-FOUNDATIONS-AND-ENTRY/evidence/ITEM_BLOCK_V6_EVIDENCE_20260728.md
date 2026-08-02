# Item role block V6 evidence

Role: `ThatTu`, ID `6`, account `90175com`

Evidence source: live VPS MySQL `jx3_25.role.ExtInfo`, queried before any
source change.

```text
ExtInfo length: 15325 bytes
role block: type=2, version=6, length=3510 bytes
```

The block sequence was parsed from the role header using the target/source
12-byte `KRoleBlockHeader`:

```text
global header: ver=0, len=15313
item block:   type=2, ver=6, len=3510
```

Target DWARF/decompile proves:

- `KItemList::LoadItemList @ 0x082e4d90` dispatches version 6 to
  `LoadItemList_V6 @ 0x082e10e4`;
- V6 consumes a version-specific prefix and five-byte item records before
  calling the same item manager/inventory side effects;
- the legacy candidate calls `KItemList::Load(data,len)` and treats every
  record as the three-byte `KITEM_DB_HEADER`, causing the observed leftover
  bytes failure.

This evidence authorizes only the V6 loader boundary. It does not authorize
silently accepting unknown versions or discarding remaining bytes.
