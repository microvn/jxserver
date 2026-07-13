# TODO — JX3 port 2010 → v2.5

Ghi chú công việc còn treo, phát sinh trong lúc port/verify. Mỗi mục kèm lý do + cách làm.
Ground-truth = binary v2.5/v246: DWARF `jx3_dwarf/SO3GameServerD` (pyghidra `SO3GameServerD-2d2a24`)
và stripped release `SO3GameServer-3c8199`. Bản đồ port đầy đủ: `docs/port_map/PORT_MAP.md`.

---

## 1. [BLOCKER — chờ port KLuaPlayer] 6 Lua query binding của KExterior đang "chết"

**Triệu chứng**: 6 hàm truy vấn tủ ngoại trang sẽ trả `nil` tại runtime (đúng kiểu lỗi `GetXxx nil`):
`LuaGetExteriorInfo`, `LuaGetExteriorSuitInfo`, `LuaGetExteriorShopPrice`,
`LuaGetExteriorIndex`, `LuaGetSuitAchievementID`, `LuaCanAchieve`.

**Hiện trạng trong source**:
- Đã được định nghĩa trong `src/.../KLuaPlayer.cpp` (khoảng dòng 528/564/590/618/633/649)
  nhưng dưới dạng **method `KPlayer::`**.
- **Không hàm nào được `REGISTER_LUA_FUNC`** → Lua không thấy → nil.

**Vì sao KHÔNG fix vội (chỉ thêm REGISTER_LUA_FUNC bây giờ)**:
- Binary v2.5 KHÔNG expose các hàm này dạng `KPlayer` method. Nó dùng **static binding
  `Luna<KExterior>`** (`KExterior::LuaGetExteriorInfo/...`). Tức là API surface đúng của v2.5
  khác chỗ đặt hiện tại trong source.
- `KLuaPlayer.cpp` là **hub sẽ được port toàn bộ sau** (thuộc cụm player-hub / KScriptFuncList).
  Nếu giờ đăng ký 6 hàm này theo hình dạng `KPlayer::` hiện tại, rất có thể **đưa vào SAI hình dạng**
  và phải gỡ/làm lại khi port KLuaPlayer cho khớp `Luna<KExterior>` của v2.5.

**Cách làm đúng (khi tới lượt port KLuaPlayer)**:
- Quyết định giữ mô hình nào cho khớp v2.5: **`Luna<KExterior>` static binding** (đúng binary)
  hay adapter `KPlayer::` (tiện nhưng lệch API gốc).
- RE cách v2.5 register (`decompile` hàm đăng ký Lua trong binary, tìm bảng `Luna<KExterior>::Register`)
  để lấy đúng: tên hàm Lua, số tham số, kiểu trả về, self-object (player vs global KExterior).
- Chỉ khi đó mới nối `REGISTER_LUA_FUNC` / `Luna<>` cho khớp — tránh làm hai lần.

**Tạm thời**: chấp nhận 6 query này nil cho tới khi port KLuaPlayer. Không chặn phần cosmetic
(load/getter/apply) vốn đã đúng.

---

## 2. [Chờ quyết định] KExterior::AddExterior là bản gộp + rút gọn

`src/.../KExterior.cpp:311-336` — gộp `AddExterior` + `Delivery`, KHÔNG dịch trung thực từ binary.
Validation (range time/pay/buySource, `nPrice[t][p]!=-1`) và quy tắc expire khớp đường Info.

Thiếu 2 hành vi so với v2.5 (`FUN_081f9498` + `FUN_081f7df0`):
1. **Bỏ `CallBuyExteriorResultScript`** (`FUN_081f9060`): không bắn event `"BuyExteriorResult"`
   cho client sau khi grant. Hệ quả: `AddExterior` KHÔNG tự sync buy-result cho client.
2. **Nhánh `nBuySource==3` (ebsBuyShop) đọc sai bảng**: v2.5 `Delivery` đọc `GetExteriorShopPrice`
   (`+0x24` nLimitType), source luôn đọc `GetExteriorInfo`. Chỉ ảnh hưởng nếu gọi với buySource=3;
   đường grant hiện dùng source 1/2 nên chưa kích hoạt.

**Quyết định cần**: giữ bản grant rút gọn, hay bổ sung 2 hành vi trên khi port nhánh mua.

---

## 3. [Verify nhanh] Thứ tự enum role-block: rbtExteriorData phải TRƯỚC rbtTotal

KExteriorBox Save/Load dùng role-block `rbtExteriorData` (wiring đã có:
`KPlayer.cpp:2356` SAVE_ROLE_BLOCK + `:2032` `case rbtExteriorData`).
Memory nói đã đặt đúng thứ tự, nhưng **verify lại**: `rbtExteriorData` phải nằm trước `rbtTotal`
trong enum, nếu không sẽ lệch/tràn khi save.

---

## 4. Nhánh mua KExterior — CHƯA port (19 method truly-missing)

Xác nhận vắng hẳn trong source (grep = 0):
`Buy, BuyFromItem, BuyFromShop, CanBuy, CanBuyOne, CanRenew, Delivery, GetAllCost,
GetAllCostFromShop, GetExteriorBuyParams, GetExteriorID, IsMoneyEnough, OnBuyExteriorRespond,
Pay, PayCoin, RenewExterior, LuaBuyExteriorFromShop, LuaRenewExterior, LuaCanAchieve`.

Đây là công việc port chính tiếp theo của cụm exterior-cosmetic. Phụ thuộc: currency/tiền
(cụm cash-shop-currency) cho Pay/PayCoin/IsMoneyEnough.

---

## 5. [DONE + audited] Loot currency — NPC rớt currency khi chết (KNpc/KCharacter drift)

**PORTED (commit sau)**: 3 method mới `KNpc::Loot{Justice,ExamPrint,ActivityAward}` + helper
`LootCurrencyToEligible` + 3 field template (nJustice/nExamPrint/nActivityAward) + loader cột +
wire vào `KCharacter::CheckDie` (cạnh LootContribution). build ok=202, boot [OK].
**AUDIT vs DWARF**: eligibility `FUN_082fd90c` = chính `g_IsInLootRange` (KLootList.cpp) same-scene(+0x44)
+ distance — KHỚP cách 2010 dùng. Đã **sửa bug**: bỏ tong-gate (v246 Loot mới KHÔNG gate m_dwTongID;
chỉ LootContribution gate vì là 帮贡). Scaling: ExamPrint/ActivityAward flat = v246; Justice v246
nhân `(player+0x9568 gain-rate +1024)/1024` — attribute justice-gain là v2.5 version-gapped (2010
không có) → flat = v246-bonus-0, baseline đúng; ghi refinement nếu port attribute justice-gain sau.
Contribution/Prestige giữ 2010-legacy (đã có). → khép vòng currency: earn(loot) ↔ spend(shop).

### 5-RE. Chi tiết RE loot (tham khảo — đã port ở mục 5)

**Verified vs binary (2026-07-13)**: `KCharacter::CheckDie` (FUN_08294d56) khi có killer gọi ~12 hàm
Loot* của KNpc; 5 hàm currency: `KNpc::Loot{Contribution,Prestige,Justice,ExamPrint,ActivityAward}(KPlayer*)`
(0835fa9e/083616ec/083606b4/083602ee/0835ff28). Mỗi hàm:
- Đọc **drop-amount từ NPC template**: nContribution@template+0xe74, nJustice@+0xe7c, nActivityAward@+0xe84
  (các offset khác: RE thêm khi port). Template gấp: 5 field int drop-amount + cột loader NPC-template.tab.
- Duyệt **danh sách player đủ điều kiện** (loot-share group: FUN_0837d3b8 get-list, FUN_0823728a count,
  FUN_0833d2ce get[i], FUN_082fd90c eligible + cùng camp `*(player+0x44)==*(npc+0x44)`) — infra NÀY có
  sẵn 2010 (LootMoney/LootExp dùng), TÁI DÙNG.
- **Scaling per-type**: Justice = base*(1 + player.bonus(+0x9568)/1024); Contribution = level-diff
  (diff=player.lv-npc.lv, clamp[0,15], nếu >5 thì base*(0x73-3*diff)/100); ExamPrint/ArenaAward/Activity: RE khi port.
- Gọi `GetCurrency(player+0x9878, type)->AddCurrency(scaled)` (4 type mới) / `KPlayer::AddContribution` (legacy 0/1).

**Cách port (bé→to)**: (a) 5 field template + loader; (b) 5 method KNpc mirror 1 Loot 2010 (iteration)
+ scaling từ binary; (c) wire 5 call vào KCharacter::CheckDie 2010. Verify: oracle scaling + build + boot.
Đây là mảng **nối currency với content** (earn source) — khép vòng earn↔spend với shop currency-buy (đã xong).

## 6. [Defer] KHairShop buy-chain (cosmetic, KHÔNG dùng KCurrency)

`KHairShop::Buy/Pay/CanBuy` (081edbd6/081ecafc/081ec930) + c2s handler + respond. Trả **money(铜钱,
player+0x6814) / coin(元宝, player+0xb8a0) / item** — KHÔNG phải KCurrency. Deferred lúc port hair
("needs coin subsystem" — giờ money/coin có: AddMoney/AddCoin). Drift riêng: KHAIR_PRICE_INFO + Pay
deduct 3 loại + CanBuy gate + c2s buy-request handler + respond code. Ước ~2-3h. Feature mua cosmetic,
orthogonal currency.

## 7. [Defer] KExterior buy-chain (cosmetic, KHÔNG dùng KCurrency) — xem thêm mục 4

`EXTERIOR_PAY_CURRENCY_TYPE = {Free=0, Money=1, Coin=2}` (KHÔNG có KCurrency). Methods (mục 4):
Buy/BuyFromShop/BuyFromItem/CanBuy/CanBuyOne/AddExterior/Pay/PayCoin/... + time-type + buy-source +
c2s handler + OnBuyExteriorRespond. Trả free/money/coin. Ước ~3-4h. Feature mua cosmetic, orthogonal currency.
(Trùng phần lớn mục 4 "19 method truly-missing" — gộp khi port.)

## Ghi chú phương pháp (đọc trước khi RE tiếp)

- **Cross-build name drift**: DWARF debug build và stripped release `-3c8199` (cùng thời v2.5)
  đặt TÊN HÀM KHÁC NHAU. Vd set-API: debug = `AddExteriorIntoSet/Find/ChangeExteriorSet`;
  release = `SetExterior/GetExteriorItem/SetCurrentSetID`. Source port khớp release build.
  → Khi đối chiếu tên hàm, kiểm cả hai build trước khi kết luận "thiếu"/"sai tên".
- KExterior/KExteriorBox verify PASS: struct/enum/serialize/DB/sync byte-exact vs DWARF,
  logic loader/getter/apply/_Add khớp binary. Không rủi ro data-integrity. (Chi tiết: đã verify
  bằng 2 sub-agent 2026-07-12.)
