# Relay enum drift at role-load boundary

Status: `TARGET_STATIC_CONFIRMED`; candidate patch review/build/runtime pending.

## First divergent packet

The fresh selected-role capture reached the role-load boundary and then stopped:

```text
GS -> Center: 08 00 37 00 06 00 00 00
```

The internal frame declares length `0x0008`, protocol ID `0x0037` (55), and only
the player ID after the two-byte protocol header. Center rejects it with
`uPakSize >= m_uProtocolSize[pHeader->wProtocolID]` and closes the GS connection.
The GS then reports `Game center lost, shutdown !`; the client disconnect is a
downstream symptom.

## Target contract

Target DWARF enum `KS2R_PROTOCOL` in
`graphengine/dwarf/SO3GameServerD.jsonl` (`enum_die=0x00a81c86`) proves:

| Route | Target ID |
|---|---:|
| `s2r_save_scene_player_list` | 52 |
| `s2r_save_scene_data` | 53 |
| `s2r_reset_map_copy_request` | 54 |
| `s2r_player_enter_scene_notify` | 55 |
| `s2r_sync_road_track_force` | 56 |
| `s2r_load_role_data_request` | 57 |
| `s2r_load_account_data_request` | 58 |
| `s2r_change_role_level_request` | 59 |
| `s2r_change_role_forceid_request` | 60 |

The target `S2R_PLAYER_ENTER_SCENE_NOTIFY` is 14 bytes, so an 8-byte frame with
ID 55 cannot be a valid enter-scene notify. Candidate disassembly of
`KRelayClient::DoLoadRoleDataRequest` at `0x0822730a` shows a 6-byte allocation
and `movw $0x37`; this exactly explains the captured 8-byte frame.

## Candidate/source divergence

The candidate enum was implicitly numbered from an older 2010 list. Its
`DoLoadRoleDataRequest` therefore compiled as ID 55, while the v2.5 Center uses
ID 57. The bounded source change pins the role-entry IDs 52--60 explicitly and
adds the target account-data route ID 58. It does not change payload ownership,
socket framing, parser permissiveness, or gameplay state logic.

## Acceptance probe

After opposite-model review and a native rebuild, the first required predicate
is a fresh GS-to-Center packet with:

```text
protocol = 57 (s2r_load_role_data_request)
length >= target size for that request
Center does not emit the uPakSize predicate or disconnect
```

Only then should the next role-data/section-check boundary be investigated.
This dossier does not claim enter-world success.

