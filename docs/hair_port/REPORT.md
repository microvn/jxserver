# REPORT — KHairShop + KHairBox port (发型/hair)

**Status: grant-path DONE + verified.** Pilot cho quy trình DWARF-based.

## Kết quả
- 2 subsystem NEW port hoàn chỉnh grant-path: KHairShop (singleton config + grant + Lua) +
  KHairBox (per-player inventory + serialize + apply).
- Build host: **ok=194 fail=0, link 0 undefined**. Boot: **"Load game settings [OK]"** (no-regression).
- Coverage-diff sạch: ported 24 / folded 3 / deferred 8 (buy chain, depend coin) / forgotten 0.

## Verify nets đã chạy
- Oracle-mirror layout (S1): 6 struct sizeof/offset == DWARF. PASS.
- Oracle-mirror logic+byte (S2/S3): _Add sort/unique/cap 1024 + Save/Load roundtrip byte-exact +
  empty-box + blockSize=2+count*8 + trailing freecount. PASS.
- Build host ×3 (S1, wire, S5): luôn ok, link 0 undefined.
- Boot no-regression ×2: settings-OK, Lua register không crash.

## Files
Mới: KHairShopDef.h, KHairShop.h/.cpp, KHairBox.h/.cpp, KLuaHairShop.cpp.
Sửa (backup_source_20260712_010655): KRoleDBDataDef.h, KWorldSettings.h/.cpp, KPlayer.h/.cpp,
KScriptCenter.cpp, KBaseFuncList.cpp.

## Đo thời gian
- S2→S6 (code + oracle + 3 build host + 2 boot): **25 phút** wall-clock.
- ĐK: RE (RE-1..4, 3 sub-agent DWARF) + verify KExterior + S1 đã xong TRƯỚC mốc đo.
- Nhanh hơn ước tính (2-2.5h) vì: hair = song sinh KExterior (tái dùng khuôn), RE đã đủ,
  apply-hook tái dùng cơ chế 2010 (SetRepresentID) không phải transcribe offset, không vấp bug.

## Còn lại (ngoài pilot)
- Buy chain (8 hàm) — mở khi port currency/coin (cụm cash-shop-currency).
- Client sync packet cho hair (v246 FUN_08059668) — defer như KExterior (2010 client version-gap).
- Runtime end-to-end (login + đổi tóc in-game) — cần cluster + client, để lần chạy tích hợp.
