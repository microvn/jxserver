# WORKLOG — KHairShop + KHairBox port (发型/hair)

Pilot dùng **quy trình DWARF-based** (binary `jx3_dwarf/SO3GameServerD`, không strip, có debug_info).
Khác case study KExterior (dùng stripped `__PRETTY`): layout/prototype lấy THẲNG từ DWARF
(`llvm-dwarfdump --name=<T> -c`), không suy từ pseudo-C.

Công cụ: `/opt/homebrew/opt/llvm/bin/llvm-dwarfdump`, pyghidra `SO3GameServerD-2d2a24`.
Ground-truth header gốc: `Source/Common/SO3World/Src/KHairShop.h` (theo DW_AT_decl_file).

---

## [RE-1] Symbol map + struct layout (từ DWARF, 100% chắc)

Xác nhận NEW: `LC_ALL=C grep -rIna 'KHairShop::|KHairBox::' src` = 0. Không có file *hair* nào.

### Method list (nm -C, .text defined)
KHairShop (singleton, khuôn = KExterior):
  Init() / UnInit() / LoadHairPriceTable() / LoadHairIndexTable()
  GetHairIndexInfo(ulong) / GetPriceInfo(int,int,ulong)
  AddHair(KPlayer*,uint,ulong,int)          <- GRANT path (giống LuaAddExterior)
  ChangePlayerHair(KPlayer*,ulong,ulong,int,int)  <- apply hook
  -- buy chain (HOÃN, depend currency NEW):
  IsMoneyEnough(KPlayer*,KHAIR_PRICE_INFO&) / CanBuy(KPlayer*,uint,ulong,int)
  Pay(KPlayer*,KHAIR_PRICE_INFO&,KHAIRSHOP_BUY_PARAM&) / GetCost(...) / GetTotalCost(...)
  Buy(KPlayer*,KHAIRSHOP_BUY_PARAM&) / OnHairShopBuyHairRespond(KPlayer*,KHAIRSHOP_COIN_BUY_PARAM*,int,int)
  -- Lua: LuaOpenHairShop/LuaCloseHairShop/LuaChangePlayerHair/LuaBuyHair/LuaGetHairIndex/LuaGetHairPrice/LuaGetObj

KHairBox (per-player, khuôn = KExteriorBox):
  Init(KPlayer*) / UnInit()
  _Add(uint,ulong) / Add(uint,ulong) / Find(uint,ulong) / ChangeHair(uint,ulong)
  GetHairList(uint) / GetHairFreeCount() / AddHairFreeCount(int) / SetHairFreeCount(int)
  Load(uchar*,uint) / Save(uint*,uchar*,uint) / LoadHairList(...) / SaveHairList(...)

### Struct layout (llvm-dwarfdump --name -c) — field/offset CHÍNH XÁC
KHAIR_INDEX_INFO  size 0x0c  (KHairShop.h:8):
  +0x0  dwHeadformID  DWORD
  +0x4  dwBangID      DWORD
  +0x8  dwPlaitID     DWORD
KHAIR_PRICE_INFO  size 0x0c  (KHairShop.h:33):
  +0x0  nPrice          int
  +0x4  nCoin           int
  +0x8  nNeedFreeCount  int
KHAIRSHOP_BUY_PARAM  size 0x10  (KHairShop.h:40):
  +0x0  dwID[2]         DWORD[2]
  +0x8  bUseFreeCount[2] BOOL[2]
KHAIRSHOP_COIN_BUY_PARAM  size 0x18  (KHairShop.h:47):
  +0x0  nPrice          int
  +0x4  nFreeCount      int
  +0x8  dwID[2]         DWORD[2]
  +0x10 bUseFreeCount[2] BOOL[2]

Ghi chú: hair = {Headform(kiểu đầu), Bang(mái/tóc trước), Plait(tết/búi)} = 3 thành phần DWORD.

---

## [RE-2] KHairShop — logic + apply-hook + Lua (từ DWARF layout + pyghidra logic)

Binary logic decompile: pyghidra `/SO3GameServer-3c8199` (release, stripped nhưng địa chỉ nm
khớp 1:1 với `SO3GameServerD`; decompile theo ĐỊA CHỈ). Layout lấy từ DWARF `SO3GameServerD`.
LƯU Ý: bản `-D` chưa import vào pyghidra; dùng release + address, KHÔNG dùng tên hàm.

### Class KHairShop layout (DWARF, chắc chắn) — size 0x34
  +0x00  m_bCloseHairShop     BOOL         (singleton, offset 0 = cờ đóng shop)
  +0x04  m_HairIndexInfoMap   std::map<DWORD, KHAIR_INDEX_INFO>          size 0x18
  +0x1c  m_HairPriceInfoMap   std::map<KHAIR_PRICE_KEY, KHAIR_PRICE_INFO> size 0x18
  static szClassName, static LuaInterface = Luna<KHairShop>::KLuaData[]
KHAIR_PRICE_KEY size 0x0c (KHairShop.h:15): {nRoleType@0 int, nType@4 int, dwID@8 DWORD}
  → key giá = (nRoleType, nType, dwID); key index = HeadID (DWORD đơn).

### Config tables (Init @081ee78c gọi LoadHairIndexTable rồi LoadHairPriceTable)
Init: m_bCloseHairShop = *(g_pWorld[DAT_084f67f8]+0x4f4); ctor 2 map; load 2 bảng.

LoadHairIndexTable @081ee3ca → file `settings/HairShop/HeadIndex.tab` (fmt "%s/HairShop/%s")
  Cột đọc (bắt đầu dòng 2): HeadID, HeadformID, BangID, PlaitID.
  KEY = HeadID (DWORD). VALUE = KHAIR_INDEX_INFO {dwHeadformID, dwBangID, dwPlaitID}.
  Ghi map (FUN_081f03d4=insert tại this+4): *v=HeadformID; v[1]=BangID; v[2]=PlaitID.
  → column-order (Headform,Bang,Plait) == struct memory-order. Dò trùng HeadID → "already exist".

LoadHairPriceTable @081edede → file `settings/HairShop/HairPrice.tab`
  Cột: RoleType, Type("Type"@0x8409159), ID("ID"@0x840915e), Price, Coin("Coin"@0x8409181),
       NeedFreeCount.
  KEY = KHAIR_PRICE_KEY {RoleType, Type, ID}. VALUE = KHAIR_PRICE_INFO {nPrice=Price, nCoin=Coin,
       nNeedFreeCount=NeedFreeCount}. Ghi tại this+0x1c (FUN_081efcac=insert).
  Assert lúc load: nPrice == 0 (Price phải =0, line 0x89), nCoin >= 0. → giá "điểm" bỏ, dùng Coin.

### Lookup
GetHairIndexInfo(DWORD HeadID) @081ec734: tra m_HairIndexInfoMap (this+4), key=HeadID.
  return &node.value + 4 (bỏ key). → forward lookup HeadID → {Headform,Bang,Plait}.
GetPriceInfo(int nRoleType,int nType,DWORD dwID) @081ec7c4: build key {p2,p3,p4}, tra this+0x1c,
  return &node.value + 0xc. → lookup (RoleType,Type,ID) → KHAIR_PRICE_INFO.

### KHairBox (per-player, EMBED tại KPlayer+0xb8a0) — layout suy từ hàm chạm memory
  box+0x04  list[type=0] (hsFace)  ~0xc byte (std::set)
  box+0x10  list[type=1] (hsHair)  ~0xc byte
  box+0x1c  m_pPlayer (KPlayer*)
  hsTotal = 2 (chỉ type 0/1). Find/_Add/ChangeHair đều assert uType<2.
Find @082069b4(box,uType,dwID): search set tại box+4+uType*0xc.
Add  @082070a6(box,uType,dwID): verify GetPriceInfo(g_HairShop[DAT+0x68890], player.roleType,uType,id)
  → FUN_08206e56 add-to-set → FUN_08059668(&DAT_084e1180, player+0xa50, uType,1,&id) = sync-add client.

### APPLY HOOK — điểm ghi ngoại hình (QUAN TRỌNG, cross-check 2010)
KHairBox::ChangeHair @08206b76(box,uType,dwID)  ← nơi THỰC SỰ ghi field ngoại hình player:
  pPlayer = *(box+0x1c); iVar3 = (uType==0)?0:1;
  guard: *(pPlayer+0xbc)==0 && *(pPlayer+0xc0)==0 && cur != new
  Find(box,uType,id) phải owned →
    FUN_0807c56e(&DAT_084e1180, pPlayer, iVar3, dwID)         // broadcast/sync đổi tóc cho client
    *(short*)(pPlayer + 8 + (iVar3+0x4ad0)*2) = (short)dwID   // GHI FIELD NGOẠI HÌNH
  → player field = short[2] tại KPlayer+0x95a8: [face]=+0x95a8, [hair]=+0x95aa (v246).
  *** CẢNH BÁO PORT: 0x95a8/0xb8a0/0xcfc/0x6814 = offset v246. KPlayer 2010 layout KHÁC.
      PHẢI dò lại field represent tương đương trong 2010 (khối human-feature/representID,
      giống cách KExterior ghi representID). KHÔNG port thẳng offset. ***

ChangePlayerHair @081ed5cc(this,pPlayer,dwHair1,dwHair2,bFree1,bFree2)  ← orchestrator "grant+wear":
  nếu dwHair1(type1)!=0: Find owned? chưa→AddHair(this,p,1,dwHair1,bFree1); rồi→dwHair1=0 (đã có, ko charge)
  nếu dwHair2(type0)!=0: tương tự AddHair type 0.
  build KHAIRSHOP_BUY_PARAM{dwID[2]={dwHair2,dwHair1},bUseFree[2]={bFree2,bFree1}}
  GetTotalCost(this, *(p+0xcfc)=nRoleType, &param, &priceOut)   // FUN_081ecf88, validate 1..6
  CallBuyHairResultScript(pPlayer, ?, &param, code=1)           // FUN_081ed21d
  *** LƯU Ý: nhánh "đã sở hữu" chỉ zero id, KHÔNG gọi lại ChangeHair — apply chỉ chạy qua AddHair
      (khi CHƯA sở hữu). Verify chủ ý này với 2010 (có thể cần wear-owned riêng). ***

### GRANT path
AddHair @081ed3fa(this,pPlayer,uType,dwID,bResetFree): private, uType<2.
  nếu dwID!=0 && Find(box,uType,id)==0 (chưa có):
    KHairBox::Add(box,uType,id)                                    // FUN_082070a6
    KLogClient::LogPlayerHairChangeRespond(&DAT_084f68a0,p,id,uType,bReset) // FUN_081babb2 (log "face"/"hair")
    KHairBox::ChangeHair(box,uType,id)                             // FUN_08206b76 = apply
  → AddHair = cấp vào box + log + apply. Đây là path port đầy đủ (apply nằm trong đây).

### Buy chain (HOÃN — depend currency NEW)
Buy @081edbd6(this,pPlayer,&BUY_PARAM): 
  if m_bCloseHairShop → 0xb (shop đóng); 
  2× CanBuy(FUN_081ec930) mỗi item phải ==1; GetTotalCost; IsMoneyEnough(FUN_081ec866);
  guard *(p+0xb670); Pay(FUN_081ecafc) trừ tiền; AddHair type0 & type1; CallScript.
  Tiền TRONG GAME: KPlayer+0x6814 = money obj (GetMoney=FUN_0808d0cc, AddMoney=FUN_082e3a9a).
OnHairShopBuyHairRespond @081ed7f4(this,p,KHAIRSHOP_COIN_BUY_PARAM*,nCoin,bSuccess): callback mua = COIN
  (点券/元宝 ngoài, KHAIRSHOP_COIN_BUY_PARAM đến từ paysys async):
  bSuccess=0 (fail/refund): refund FUN_0838b2ec + AddMoney(p+0x6814) + log BUY_HAIR_FAILED;
  bSuccess!=0: AddHair type0/type1 (giao tóc). → DEPEND: subsystem COIN/diamond + paysys = NEW, HOÃN.

### Lua routing (self = GLOBAL KHairShop singleton, KHÔNG phải KPlayer)
LuaGetObj @0826519c: lua_newuserdata(L,4); *ud = param_1 (KHairShop*); setmetatable "KHairShop".
  → binding chuẩn Luna<KHairShop>: đẩy con trỏ singleton làm userdata 4-byte. self trong mọi Lua*
  = KHairShop* (kiểm *param_1 == m_bCloseHairShop@0). KHÁC KExterior (đừng lặp lỗi shape TODO §1).
LuaOpenHairShop  @081f1940: top==0; set *this(m_bCloseHairShop)=0 (mở). return 0.
  (LuaCloseHairShop = set =1, đối xứng.)
LuaChangePlayerHair @081f19f0: top==5; a1=dwPlayerID→GetPlayer(g_world+0x609dc via FUN_08056a94),
  a2=hair1(type1), a3=hair2(type0), a4/a5 = bool-hoặc-number (bFree). Gọi ChangePlayerHair. push bool.
LuaBuyHair @081f1cb4: top==5; a1=playerID; đọc 2 cặp (id,bool)→KHAIRSHOP_BUY_PARAM{id0,id1,free0,free1};
  GetPlayer; mỗi type: nếu owned→ChangeHair(box)+zero id; nếu còn id→Buy(). push respond-code (number).
LuaGetHairIndex @081f1f56 / LuaGetHairPrice @081f20b4: getter tra 2 map (chưa decompile chi tiết,
  wrap GetHairIndexInfo/GetPriceInfo, trả các field số).

### Tổng kết port
- Layout class + 4 struct + KHAIR_PRICE_KEY: 100% từ DWARF, port thẳng an toàn.
- 2 bảng tab + thứ tự cột: port thẳng (settings/HairShop/HeadIndex.tab + HairPrice.tab).
- Apply = KHairBox::ChangeHair ghi short[2] represent + broadcast FUN_0807c56e. Offset player
  (0x95a8, 0xb8a0, 0xcfc) v246 → PHẢI re-derive cho 2010.
- Grant (AddHair) port đầy đủ. Buy/OnCoinBuy HOÃN (depend money in-game OK nhưng coin/paysys NEW).
- Lua self = KHairShop singleton (Luna userdata 4B). ChangePlayerHair/BuyHair unpack playerID→GetPlayer.

## [RE-3] KHairBox — per-player hair inventory (DWARF + pyghidra, READ-ONLY)
Nguồn: DWARF `jx3_dwarf/SO3GameServerD` (--name -c), logic pyghidra binary `/SO3GameServer-3c8199`
(addr == SO3GameServerD, verified nm). Src/KHairBox.cpp / KHairBox.h. Class byte_size 0x20.

### Class layout KHairBox (0x20) — DWARF @0x059ef098
| off | field | type | size | ghi chú |
|---|---|---|---|---|
| +0x00 | m_nFreeCount | int | 4 | số lần đổi tóc MIỄN PHÍ còn lại; clamp [0,60000] |
| +0x04 | m_HairList[2] | KHAIR_VECTOR[2] | 0x18 | 2 × std::vector<unsigned long> (0xc mỗi cái); mỗi phần tử = DWORD hair id |
| +0x1c | m_pPlayer | KPlayer* | 4 | back-ref, set ở Init |
- KHAIR_VECTOR = typedef `std::vector<long unsigned int>` (byte_size 0xc, 3 con trỏ). In-mem hair item = **DWORD id thuần** (không struct), giữ SORTED + UNIQUE (lower_bound + insert).
- 2 list = 2 loại tóc: hsTotal=2 (hsFace=0 / hsHair=1 — khớp cột Type HairPrice.tab & KHairShop). uType param LUÔN < 2.
- MAX_HAIR_LIST_SIZE = 0x400 (1024): _Add lỗi khi size() > 0x3ff.
- KPlayer embed: m_HairBox @KPlayer+0xb8a0 (DWARF KPlayer.h:1350) — CROSS-CHECK ONLY, port để compiler tự layout.

### DB serialize (RỦI RO CAO — byte-exact) — KRoleDBDataDef.h:588
KHAIR_DB_DATA (byte_size 0x02, flexible):
  +0x0 wCount   WORD
  +0x2 HairInfo KHAIR_INFO[] (flexible array)
KHAIR_DB_DATA::KHAIR_INFO (byte_size 0x08):
  +0x0 wID       WORD   (= LOW 16 bit của DWORD id in-mem)
  +0x2 byReserved BYTE[6] (ghi 0 khi Save — memset 6)

**Full blob layout (Save@082077ca / Load@08207466):**
```
list0:  [WORD blockSize0][WORD wCount0][ wCount0 × KHAIR_INFO(8B) ]
list1:  [WORD blockSize1][WORD wCount1][ wCount1 × KHAIR_INFO(8B) ]
trailer:[WORD freeCount]
```
- blockSizeN = 2 + wCountN*8 (bytes SaveHairList ghi cho list đó, KHÔNG gồm prefix WORD).
- LUÔN ghi cả 2 block dù list rỗng (block rỗng = WORD 0x0002 prefix + WORD 0x0000 count).
- freeCount = trailing WORD (m_nFreeCount cast xuống short). Load yêu cầu uLeftSize==2 sau 2 block (đúng = 1 WORD freecount).
- puUsedSize (out) = tổng byte đã dùng = blockSize0+2 + blockSize1+2 + 2.

**Save@082077ca**: loop uType 0..1 → chừa WORD prefix, gọi SaveHairList(&used, buf, left, uType), backfill *prefix=(WORD)used, tiến buf/left. Sau loop: *buf=(WORD)m_nFreeCount; *puUsedSize = bufSize-(left-2).
**SaveHairList@08207636**: *buf=(WORD)vector.size(); mỗi i: buf[i*4+1]=(WORD)vector[i] (wID), memset(+6) byReserved; *used = 2+count*8. Assert uType<2, left>=2, left>=8 mỗi item.
**Load@08207466**: loop 0..1: đọc WORD uDataSize (blockSize), check left-2>=uDataSize, LoadHairList(buf+2,uDataSize,uType), tiến 2+uDataSize. Sau loop: m_nFreeCount=(WORD)*buf; assert left==2.
**LoadHairList@08207202**: đọc WORD wCount; check left-2>=wCount*8; mỗi i: wID=buf[i*4+1]; validate qua HairShop GetPriceInfo(g_HairShop+0x68890, player.roleType@+0xcfc, uType, wID) → nếu hợp lệ gọi _Add(uType,wID). Assert uType<2, left>=sizeof(KHAIR_DB_DATA)=2, left>=wCount*8.
CROSS-CHECK Save↔Load: KHỚP (prefix WORD + count WORD + 8B/item + trailing freecount WORD). ✔

### Logic hàm
| addr | hàm | vai trò |
|---|---|---|
| 08206aea | Init(KPlayer*) | assert pPlayer; m_nFreeCount=0; m_pPlayer=pPlayer; construct 2 vector |
| 08206976 | UnInit() | destruct vectors (khuôn KExteriorBox) |
| 08206e56 | _Add(uType,dwID) | RAW insert: lower_bound → nếu chưa có thì insert giữ SORTED+UNIQUE; cap size<1024; KHÔNG validate shop. Dùng bởi Load |
| 082070a6 | Add(uType,dwID) | dwID==0→noop; uType<2; VALIDATE GetPriceInfo (hair phải có trong shop table) → _Add → sync client FUN_08059668. Đường "cấp tóc" bình thường |
| 082069b4 | Find(uType,dwID) | lower_bound; true nếu *it==dwID (player sở hữu hair này chưa) |
| 08206b76 | ChangeHair(uType,dwID) | dwID==0→noop; uType<2; assert m_pPlayer; guard (player+0xbc==0 && +0xc0==0) && current!=dwID → Find (phải sở hữu) → broadcaster FUN_0807c56e(represent) → ghi applied *(short*)(player+0x95a8+uType*2). ÁP tóc đang sở hữu lên ngoại hình |
| 08206948 | GetHairList(uType) | return &m_HairList[uType] (const) |
| 0820696c | GetHairFreeCount() | return m_nFreeCount |
| 08206d08 | AddHairFreeCount(n) | saturating add, floor 0, ceil INT_MAX, **cap 60000**; assert m_pPlayer; sync |
| 08206de2 | SetHairFreeCount(n) | set thẳng (không clamp); assert m_pPlayer; sync |

- _Add vs Add: **_Add** = insert thuần (dedupe+sort+cap), không check shop, cho Load reconstruct. **Add** = _Add + validate hair tồn tại trong HairShop table + client-sync. Cả hai cap 1024.
- Find key = (uType, dwID) — dwID là hair id (khớp wID DB, ≤0xFFFF).
- freecount rule: credit "đổi tóc miễn phí"; nNeedFreeCount/bUseFreeCount ở KHAIRSHOP_BUY_PARAM tiêu credit này. Cap 60000 (⇒ WORD serialize an toàn).
- applied-hair (ChangeHair ghi) = short[2] @KPlayer+0x95a8 {face@0x95a8, hair@0x95aa} — DRIFT-LOCK, re-derive field represent 2010, KHÔNG hardcode.

### Rủi ro port (khóa lại)
1. wID = **WORD** trong DB ⇒ hair id PHẢI ≤ 65535, nếu không mất 16 bit cao. In-mem là DWORD.
2. KHAIR_INFO stride = 8B (WORD + 6B reserved) — pad cứng, #pragma pack không đổi (đã align tự nhiên). Save memset reserved=0.
3. freeCount = WORD on-disk nhưng int in-mem; cap 60000 chặn tràn.
4. LUÔN 2 block (kể cả rỗng) + trailing WORD; Load assert chặt (left==2 cuối) ⇒ sai layout = fail load role-block.
5. rbtHairBoxData append TRƯỚC rbtTotal; ordinal self-consistent trong DB 2010 (không cần khớp v246).

## [PORT-1] S1 — enums/structs + config load (KHairShop)

Tạo 3 file mới (build.sh globs SO3World/Src/*.cpp → tự join):
- `KHairShopDef.h` — HAIR_SLOT_TYPE{hsFace,hsHair,hsTotal}, MAX_HAIR_LIST_SIZE 1024,
  MAX_HAIR_FREE_COUNT 60000, 5 struct config/param + KHAIR_DB_DATA (pack1) — mọi size từ DWARF.
- `KHairShop.h` — class singleton (m_bCloseHairShop + 2 map), S1 methods (Init/UnInit/Load*/Get*).
- `KHairShop.cpp` — Init→LoadHairIndexTable(HeadIndex.tab)→LoadHairPriceTable(HairPrice.tab);
  columns pinned RE-2; giữ nguyên assert v246: Price==0 && Coin>=0. Get lookups.

VERIFY: oracle-mirror `scratchpad/oracle_hair_s1.cpp` — static_assert sizeof+offset của
KHAIR_INDEX_INFO(0xc)/KHAIR_PRICE_INFO(0xc)/KHAIR_PRICE_KEY(0xc)/KHAIRSHOP_BUY_PARAM(0x10)/
KHAIRSHOP_COIN_BUY_PARAM(0x18)/KHAIR_DB_DATA::KHAIR_INFO(8) == DWARF. clang++ gnu++11: PASS.
Còn: compile thật trên host (gnu++98 + game headers), wire Init vào boot (cần bảng data).

---

## [RE-4] Cross-check SOURCE 2010 (READ-ONLY — định vị chỗ móc, tránh transcribe offset v246 mù)

Mục tiêu: 3 điểm móc trong `linux-build/src` để port an toàn. Tất cả grep `LC_ALL=C grep -rIna`.

### 1. enum role-block — `ROLE_DATA_BLOCK_TYPE`
- **DEF (không phải nơi dùng)**: `include/Include/KRoleDBDataDef.h:14-43` (KHÔNG ở KPlayer.cpp; KPlayer.cpp
  chỉ DÙNG các rbt* tại switch load ~1937-2047 và SAVE_ROLE_BLOCK ~2336-2358).
  Chú ý: `find src -name KRoleDBDataDef.h` chỉ ra bản `/Test/`; bản build thật ở **include/Include/**
  (KPlayer.cpp:20 `#include "KRoleDBDataDef.h"`). Bản `/Test/…/Robot/SO3World/KRoleDBDataDef.h` là copy cũ, KHÔNG sửa.
- Tên enum = `ROLE_DATA_BLOCK_TYPE` (KHÔNG phải ROLE_BLOCK_TYPE). Bắt đầu `rbtInvalid=0`.
- Đuôi enum:
  ```
  38      rbtAntiFarmerData,
  39      rbtMentorData,
  40      rbtExteriorData,     <- KExterior port đã thêm ở đây (cuối, ngay trước rbtTotal)
  41
  42      rbtTotal
  ```
- **APPEND `rbtHairBoxData` giữa dòng 40 (`rbtExteriorData,`) và dòng 42 (`rbtTotal`)** — đúng chỗ KExterior
  đã append. Ordinal self-consistent (KHÔNG cần khớp v246 vì Save/Load dùng cùng enum 1 phía).

### 2. Face/hair field 2010 — CÓ SẴN, không phải subsystem mới
v246 ghi short[2] {face@+0, hair@+2} @KPlayer+0x95a8 = **map 1:1** sang mảng represent 2010:
- **Field**: `KPlayer::m_wRepresentId[perRepresentCount]` — `WORD[]`, khai báo `KPlayer.h:338`
  (kèm `m_dwRepresentIdLock` @:339). KPlayer+0x95a8 (v246) == `&m_wRepresentId[0]`.
- **Enum index**: `PLAYER_EQUIP_REPRESENT` @ `include/Include/SO3GlobalDef.h:116-155`:
  `perFaceStyle=0` (相貌/face), `perHairStyle=1` (发型/hair), … `perRepresentCount` (=count).
  → face = `m_wRepresentId[perFaceStyle]`, hair = `m_wRepresentId[perHairStyle]`. Vì WORD, offset {0,+2} khớp v246.
- **Setter (canonical)**: `KItemList::SetRepresentID(int nRepresentIndex, int nRepresentID)`
  `KItemList.cpp:3361` (decl `KItemList.h:201`). KExterior port copy y hệt tại
  `KExteriorBox::SetRepresentID` `KExteriorBox.cpp:171`. Logic: guard `m_dwRepresentIdLock`
  (+ `m_bHideHat` cho perHelmStyle), nếu đổi → broadcast, rồi ghi `m_wRepresentId[idx]=(WORD)id`.
- **Broadcast**: `KPlayerServer::DoSyncEquipRepresent(KPlayer*, int nIndex, DWORD dwRepresentID)`
  def `KPlayerServer.cpp:3400` (decl `KPlayerServer.h:346`), gọi qua `g_PlayerServer.DoSyncEquipRepresent(...)`
  bên trong SetRepresentID (`KItemList.cpp:3373` / `KExteriorBox.cpp:184`). Precedent Lua: `KLuaPlayer.cpp:368`.
- **KẾT LUẬN DRIFT (bảng D)**: KHÔNG transcribe 0x95a8. Port ChangeHair =
  `SetRepresentID(perFaceStyle, faceID); SetRepresentID(perHairStyle, hairID);` (reuse KItemList setter
  hoặc copy như KExteriorBox). Broadcast tự động qua DoSyncEquipRepresent. Chỉ cần: KHairBox giữ con trỏ
  player + gọi setter. (v246 FUN_0807c56e ~ DoSyncEquipRepresent path.)

### 3. Luna register cho GLOBAL singleton — pattern KMentorCache (3 điểm)
KExterior KHÔNG dùng Luna global — nó là method trên KPlayer (LuaAddExterior… `KLuaPlayer.cpp:406`).
KHairShop KHÁC: self = global singleton, metatable "KHairShop" (v246 LuaGetObj@0826519c:
lua_newuserdata(4)+setmetatable). Analog đúng = **KMentorCache / KGlobalSystemValueCache**
(global singleton của g_pSO3World, expose qua Get* base-func). 3 điểm móc:
- **(a) Register class**: `KScriptCenter.cpp` khối `#ifdef _SERVER` **dòng 774-778** (đang có
  `Luna<KScriptServer>`, `Luna<KGlobalSystemValueCache>`, `Luna<KMentorCache>::Register(pLuaState)`).
  Thêm `Luna<KHairShop>::Register(pLuaState);` vào đây. (Site chính init Lua env; bản table thứ 2 =
  `KBaseFuncList.cpp:4828+` map "LoadXxx"→Register, tùy chọn.)
- **(b) Getter free-func**: `KBaseFuncList.cpp:4170` `int LuaGetMentorCache(Lua_State* L)` →
  `g_pSO3World->m_MentorCache.LuaGetObj(L)`. Viết `LuaGetHairShop` tương tự →
  `g_pSO3World->m_HairShop.LuaGetObj(L)` (hoặc `m_Settings.m_HairShop` tùy nơi đặt singleton — xem note).
- **(c) Đăng ký tên**: bảng base-func `KBaseFuncList.cpp:5033` `{"GetMentorCache", LuaGetMentorCache}`.
  Thêm `{"GetHairShop", LuaGetHairShop}`. Script gọi `GetHairShop()` → object có metatable → gọi method.
- Note vị trí singleton: g_pSO3World có 2 khuôn — member trực tiếp (`m_MentorCache`) HOẶC trong
  `m_Settings` (KExterior: `g_pSO3World->m_Settings.m_Exterior`, `KLuaPlayer.cpp:420`). Chọn 1 khi port
  KHairShop (RE-2 dùng g_HairShop@DAT+0x68890 = global đơn → member g_pSO3World hợp lý nhất).

## [PORT-2..4] S2/S3/S4 — KHairBox skeleton + Save/Load + apply + wire

- `KHairBox.h/.cpp` mới: _Add (sorted+unique vector, cap 1024), Add (validate GetPriceInfo),
  Find (binary_search), ChangeHair (= SetRepresentID(nType,id); nType==perFaceStyle/perHairStyle),
  SetRepresentID (copy KItemList setter: lock guard + DoSyncEquipRepresent + m_wRepresentId),
  AddHairFreeCount (saturating 60000)/SetHairFreeCount, Save/Load role-block.
- Serialize: per-slot [WORD blockSize][WORD count][8B item x count] (blockSize=2+count*8) ×2 +
  [WORD freeCount]; Load re-validate qua HairShop GetPriceInfo(roleType,type,id), assert leftover==0.
- WIRE (5 điểm): KRoleDBDataDef.h append `rbtHairBoxData` trước rbtTotal; KWorldSettings.h/.cpp
  m_HairShop + Init; KPlayer.h embed m_HairBox; KPlayer.cpp m_HairBox.Init(this) + case rbtHairBoxData
  load + SAVE_ROLE_BLOCK save.

VERIFY:
- oracle-mirror `oracle_hair_s2s3.cpp`: _Add sort/unique/cap, Save byte-layout (blockSize/count/free),
  Save→Load roundtrip, empty-box, cap 1024 → ALL PASS.
- build host: **ok=193 fail=0, link 0 undefined**.
- boot no-regression: **"Load game settings ... [OK]"** (m_HairShop.Init đọc HeadIndex/HairPrice.tab OK,
  KPlayer wire không regress). center-connect [Failed] = smoke test không cluster (bình thường).

## [PORT-5] S5 — grant + Lua bindings + register  &  [S6] coverage-diff

- KHairShop::AddHair (box.Add validate + box.ChangeHair apply) + ChangePlayerHair (grant hair+face).
- KLuaHairShop.cpp (mới): LuaOpen/Close/ChangePlayerHair/GetHairIndex/GetHairPrice + DEFINE_LUA_CLASS(KHairShop).
  self = global singleton Luna<KHairShop> (KHÁC KExterior, đúng shape v246 — không lặp lỗi TODO §1).
- Register 3 điểm: KHairShop.h DECLARE_LUA_CLASS; KScriptCenter.cpp Luna<KHairShop>::Register;
  KBaseFuncList.cpp LuaGetHairShop + {"GetHairShop"} → script gọi GetHairShop().
- VERIFY: build host **ok=194 fail=0, link 0 undefined**; boot **"Load game settings [OK]"**,
  Luna<KHairShop>::Register không crash (không lỗi metatable/ASSERT).

## [S6] COVERAGE-DIFF (bắt buộc close-out)
A=implemented 26, B=binary surface 35, GAP=11 — phân rã đúng, KHÔNG hàm nào bị quên:
- **Deferred (8)** = buy chain (depend coin/paysys NEW): Buy, CanBuy, GetCost, GetTotalCost,
  IsMoneyEnough, Pay, OnHairShopBuyHairRespond, LuaBuyHair.
- **Folded (3)**: GetHairFreeCount (inline trong .h); LoadHairList + SaveHairList (gộp vào Load/Save —
  2010 KRoleBlockHeader không chunk key, giống KExterior).
- Ported as-named: 24 (box core + shop config + grant + 5 Lua). +1 helper thêm (SetRepresentID reuse).
TALLY: ported 24 / folded 3 / deferred 8 / N-A 0 / forgotten 0.
