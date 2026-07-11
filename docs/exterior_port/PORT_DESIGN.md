# PORT DESIGN — KExterior / KExteriorBox (外观) — v246 → build 2010

> Nguồn: RE binary `SO3GameServer` v246 (pyghidra local `/SO3GameServer-3c8199`). Địa chỉ/decompile: `WORKLOG.md`.
> Trạng thái: **RE ĐỦ để code trọn** (4 follow-up đã giải, RE-6). Mọi offset/struct/enum lấy từ binary, không đoán.

## 0. Bối cảnh
- Build 2010 **KHÔNG có** KExterior/KExteriorBox. Chỉ có `KPlayer::m_wRepresentId[perRepresentCount]` + `LuaGet/SetRepresentID` (set thẳng ID hiển thị).
- v2.5.2 thêm **tủ đồ ngoại trang** ngồi TRÊN represent-ID. Port = **dựng class mới**, móc vào represent sẵn có.
- Chủ trương (user): **port cho ĐỦ** (kể cả tương thích DB v246 + packet client), **port bé→to** (leaf trước, tránh TODO treo).

## 1. LINKED / RELEVANT (liên kết thành phần — đọc trước khi code)
Ma trận phụ thuộc (mũi tên = "cần trước"):

```
[settings/Exterior/*.tab]  --> KExterior::Load*Table  --> KExteriorInfo table (singleton)
                                                             |
KEXTERIOR_ITEM / DB structs --> KExteriorBox (box@player+0xb850) <-- m_dwApplyExteriorFlag@player+0xb89c
       |                            |            |
       |                            v            v
  m_wRepresentId[] (DA CO 2010) <-- ApplyExteriorRepresent <-- GetEquip{Represent,Color,SubType}Index (map slot->index)
                                     |
KPlayer persistence (ext-data) --> Save/Load --> SaveExteriorBox(role-block) / LoadExteriorBox
                                     |
KPlayerServer::DoSyncExteriorBoxData (protocol 0x11a) --> client v246/BVTLocal
                                     ^
KPlayer::LuaApplyExterior/LuaAddExterior/... (script goi) --> KExteriorBox / KExterior
```

**Liên kết chéo (cross-ref) với hệ khác:**
- `m_wRepresentId[]` — cơ chế 2010 ĐÃ CÓ; Apply ghi vào index {5,2,8,14,11} (represent) / {6,3,9,15,12} (color).
- KPlayer ext-data persistence — nối Save/Load vào chuỗi `OnExtDataLoadFinish`/`SaveXxx` (xem KDesignation/hệ khác làm mẫu).
- KLuaCharacter.cpp — bảng đăng ký Luna, thêm binding "ApplyExterior"/"AddExterior"/...
- Log: KLogClient::LogPlayerAddExterior{Request,Respond} (nếu cần log).
- **Version-gap (VERIFIED)**: PAP2 (jx3dev-master) = thế hệ **2010** (protocol r2s=112 vs v246=178; khác bản gốc 2010 của mình chỉ 6 dòng; 0 file KExterior/KDomesticate/KHairShop/KDesignation/KDiamond/KFellowPet). → PAP2 **không có** exterior để lấy; RE binary v246 là bắt buộc. Lưu ý: UI Lua của client nằm trong **PAK** chứ không phải C++ source PAP2 — nhưng PAK của client 2010 cũng không có exterior. Muốn render ngoại trang cần client **v246/BVTLocal**. Packet 0x11a từ server binary đủ cho phía server-emit.

## 2. Kiến trúc (2 class)
### KExterior — singleton (Src/KExterior.cpp): bảng config + logic MUA
Nạp 4 bảng ↔ `settings/Exterior/`: ExteriorInfo(1949), ExteriorSuitInfo(59), (index), Exteriorshop/ExteriorBuy.
Buy chain: `Buy->CanBuy->CanBuyOne->Pay/PayCoin->GetAllCost->IsMoneyEnough->Delivery->AddExterior`.
### KExteriorBox — instance mỗi player, **NHÚNG trong KPlayer @ 0xb850** (~0x4c byte)

## 3. Data structures (đã pin từ binary)

### KEXTERIOR_INFO (1 dòng ExteriorInfo.tab; ~72B / 18 dword; tra ID qua FUN_081f70d0)
Cột (thứ tự): `ID, ForceID, Genre, set, subtype, represetID, colorID,`
`7days0/1/Free, Permanent0/1/Free, Limittype, Limitprice0/1/Free, IconID, RepresetID1`
- +0x0c subtype | +0x10 represetID | +0x14 colorID
- **nPrice[3 timeType][3 payType]** @ +0x18 — memory-order timeType = **Permanent/7days/Limit** (SỬA: RE cũ ghi "7days/Permanent/Limit" SAI, xem PORT-1). payType idx {epctFree=0, epctMoney=1, epctCoin=2}. **giá tab x10000** ở cột epctMoney; validate nPrice[i][epctMoney]>0.
- Full 18-dword layout đã pin trong PORT-1 (subtype@0xc/represent@0x10/color@0x14, Limittype@0x3c, IconID@0x40, RepresetID1@0x44). ID = map KEY, không nằm trong struct.

### KEXTERIOR_ITEM (phần tử box; in-memory stride 0xc = 12B)
```cpp
struct KEXTERIOR_ITEM { DWORD dwID; DWORD nFlag; time_t nExpireTime; };  // @0 / @4 / @8
```
Add(id, timeType, param4): 0=vĩnh viễn(expire=0,flag=0) · 1=+604800s(7 ngày) · 2=expire=max(cũ,param4). Box max 8191.

### KExteriorBox members (từ Init FUN_08200104)
- 0x00 current-set-id/count · 0x04 set-list · 0x10 latest-buy-list · **0x38 BOX map chính** · 0x44 m_pPlayer · 0x48 counter
- 3 container STL (đề xuất: box = std::map<DWORD,KEXTERIOR_ITEM>; set/latest-buy = vector).

### KPlayer thêm 2 field
- `KExteriorBox m_ExteriorBox;`  @ **0xb850**
- `DWORD m_dwApplyExteriorFlag;` @ **0xb89c** (bit 0-4 = per-slot đang khoác; bit **0x80** = master apply-on)

## 4. Persistence (DB) — ĐÃ pin (port ĐỦ -> khớp v246)
- **Top Save** (FUN_08201100): `[WORD len][block1][WORD len][block2]`; *pOut = tổng byte. Load mirror, **leftover==0**.
- **Box block** (SaveExteriorBox FUN_08201476): chia khối 128 món, mỗi khối `KRoleBlockHeader 12B {DWORD type; DWORD key; DWORD dataLen}` + data. MAX 8192.
- **Box DB item** (LoadExteriorBox FUN_08201fee): `[WORD count][count x 16B]`; item 16B `{WORD id@0; WORD flag@2; DWORD expire@3; pad->16}`.
- **Latest-buy** item 8B `{WORD id; BYTE pad[6]}`; cap hiển thị 20.

## 5. Packet sync — ĐÃ pin (DoSyncExteriorBoxData FUN_08059bf0)
- **Protocol ID 0x11a (282)**; header 6B `{WORD id=0x11a; WORD size; WORD count}` + item 7B `{WORD id; BYTE flag; DWORD expire}` (size=count*7+6). Buffer @ player+0x2b68, MAX 0x8000.

## 6. Móc tích hợp represent (crux) — ApplyExteriorRepresent (FUN_08200a4e)
Map slot(0-4) -> chỉ số `m_wRepresentId[]`:
| Slot | RepresentIndex | ColorIndex | SubType |
|:--:|:--:|:--:|:--:|
| 0 | 5 | 6 | 2 |
| 1 | 2 | 3 | 3 |
| 2 | 8 | 9 | 6 |
| 3 | 14 | 15 | 9 |
| 4 | 11 | 12 | 10 |
```
ApplyExteriorRepresent(slot): id=set[slot]; info=Info[id]; validate info.subtype==GetEquipSubType(slot)
  if con han: rep=info.represetID, color=info.colorID
  if (m_dwApplyExteriorFlag >> slot)&1: m_wRepresentId[RepIdx[slot]]=rep; m_wRepresentId[ColIdx[slot]]=color
```
LuaApplyExterior: set flag|=0x80; loop 5 slot -> apply nếu box có; sync flag.

## 7. THỨ TỰ PORT (BÉ->TO, leaf trước — không TODO treo)
Mỗi bước compile sạch + verify trước khi sang bước sau:
1. [x] **Enums + structs leaf** — DONE (PORT-1). `Src/KExteriorDef.h`: EXTERIOR_TIME_TYPE, EXTERIOR_PAY_CURRENCY_TYPE, KEXTERIOR_ITEM, KEXTERIOR_INFO, KEXTERIOR_DB_DATA, KEXTERIOR_BUY_LATEST_DB_DATA. Layout static_assert PASS vs binary. (BUY_SOURCE/RESPOND enum + SET_DB_DATA deferred -> slice #8/#6, chưa pin member.)
2. [x] **KExterior singleton + 4 LoadTable** — DONE (PORT-2). member KWorldSettings::m_Exterior; Info/Suit/Index(ExteriorBuy.tab)/Shop nap sach tren data that -> "Load game settings [OK]".
3. [x] **GetEquip{Represent,Color,SubType}Index + KExteriorBox skeleton** — DONE (PORT-3). Verbatim v246 switch; enum-ordinal 2010==v246 (compile-guard); build ok=191. GetEquipPos/PosBySubType defer -> khi buy chain can (#8).
4. [x] **KExteriorBox Init + Add/_Add** — DONE (PORT-4). _Add time-type logic oracle PASS 6 cases; build ok=191. (client-sync trong Add defer #7; set-list defer #5/#6.)
5. [x] **ApplyExteriorRepresent + represent hook + m_dwApplyExteriorFlag + embed box** — DONE (PORT-5). setter=KItemList::SetRepresentID parity (m_bHideHat=player+0x9a30); apply-oracle PASS 6; build ok=191. (unApply/set-change re-drive defer #8.)
6. [x] **Save/Load persistence** — DONE (PORT-6). 1 role-block rbtExteriorData = owned(16B)+set(18B)+latest(8B); item-format pin binary; roundtrip-oracle PASS; build ok=191. (runtime relog defer #8.)
7. [x] **DoSyncExteriorBoxData packet + flag-sync** — DONE (PORT-7). 2010-native s2c protocols (item 7B payload pin v246); build ok=191; item-oracle PASS. (client render = v246 only, version-gap.)
8. [x] **Set-creation + un-apply + Lua bindings + grant** — DONE (PORT-8). 9 KPlayer Lua bindings + register; AddExteriorSet; unApply (2010-native GetEquipPos); KExterior::AddExterior grant; buy enums pinned. build ok=191; regression-boot 0 error. (client-purchase buy chain = coin/shop version-gapped, deferred; grant path covers give-exterior.)
> Nguyên tắc: node nào ở phần "cần trước" trong §1 thì port trước. Không port node to (KPlayer bindings) khi leaf (struct/Apply/Save) chưa có.

## 8. Verify (2 lưới)
- Log-parity build-mình vs v246 stock: nạp bảng Exterior + init box phải khớp log.
- Binary-oracle diff-test: IsMoneyEnough/CanBuyOne/GetEquip*Index/nPrice-calc — gọi vs v246 cùng input, so output.
- Observable: ApplyExteriorRepresent -> m_wRepresentId đổi.

## 9. Rủi ro (đã hạ nhờ RE đủ)
- DB byte-layout: **ĐÃ pin** (§4) -> port khớp v246 được.
- Packet client: **ĐÃ pin** (§5, protocol 0x11a).
- ~~exact byte-offset trong 16B DB item (flag@2 vs padding)~~ — **ĐÃ GIẢI (PORT-1)**: `{WORD id@0; BYTE flag@2; DWORD expire@3(unaligned); pad→16}`, xác nhận từ LoadExteriorBox decompile. Vẫn verify thêm bằng load DB v246 thật ở bước 6.

## 10. Còn phải RE khi code (nhỏ, không chặn thiết kế)
- Layout đầy đủ KEXTERIOR_SET_DB_DATA / SaveExteriorSet (format set — làm ở bước 6).
- Route các Lua binding còn lại (LuaAddExterior, LuaSetExteriorSet, LuaGetAllExterior...) — decompile khi tới bước 8.
- Suit/achievement (LuaCanAchieve, GetSuitAchievementID) — tính năng phụ, port sau cùng.
