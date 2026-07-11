# INTEGRATION MAP — KHairShop + KHairBox (caller / callee / binding)

Mục đích: lưu quan hệ tích hợp để **lần sau không phải RE lại**. Mọi dòng kèm bằng chứng
(địa chỉ hàm v2.5 hoặc file:line trong source 2010). Điền dần khi RE/port.

## A. Binding points (nơi hệ thống móc vào — PHẢI sửa file khác ngoài KHair*)
| loại | v2.5 (binary) | 2010 (file:line sẽ sửa) | trạng thái |
|---|---|---|---|
| Lua register (KHairShop) | **`Luna<KHairShop>`** — LuaGetObj@0826519c: `lua_newuserdata(L,4)`, *ud=KHairShop*, setmetatable "KHairShop". SELF = singleton (không phải KPlayer). Lua kiểm `*this`=m_bCloseHairShop@0 | **RE-4**: pattern = KMentorCache (global singleton), 3 điểm: (a) `KScriptCenter.cpp:774-778` khối `#ifdef _SERVER` — thêm `Luna<KHairShop>::Register(pLuaState);` cạnh `Luna<KMentorCache>`; (b) `KBaseFuncList.cpp:4170` — viết `LuaGetHairShop`→`g_pSO3World->m_HairShop.LuaGetObj(L)` (khuôn `LuaGetMentorCache`); (c) bảng base-func `KBaseFuncList.cpp:5033` — thêm `{"GetHairShop", LuaGetHairShop}`. KExterior KHÔNG dùng Luna global (là method KPlayer, KLuaPlayer.cpp:406) → KHÔNG dùng làm khuôn | CHỐT (RE-4) |
| KHairBox embed trong KPlayer | KPlayer+0xb8a0 (v246); box+0x1c = m_pPlayer back-ptr | KPlayer.h (compiler tự layout, KHÔNG hardcode 0xb8a0) | CHỐT |
| Save role-block | `SAVE_ROLE_BLOCK(m_HairBox.Save, rbtHairBoxData)`; Save@082077ca / Load@08207466 (sig 2010 `Save(size_t*,BYTE*,size_t)` / `Load(BYTE*,size_t)`) | KPlayer::Save + load-dispatch switch (giống rbtExteriorData) | CHỐT (RE-3) |
| role-block enum | `rbtHairBoxData` (v246 nm) | **RE-4**: enum `ROLE_DATA_BLOCK_TYPE` def tại `include/Include/KRoleDBDataDef.h:14-43` (KHÔNG ở KPlayer.cpp; bản `/Test/…/Robot/` là copy cũ, bỏ qua). Append `rbtHairBoxData` giữa `rbtExteriorData` (:40) và `rbtTotal` (:42) | CHỐT (RE-3/RE-4) — ordinal self-consistent, KHÔNG cần khớp v246 |
| packet sync (đổi tóc) | `FUN_0807c56e(&DAT_084e1180, pPlayer, iType, dwID)` trong ChangeHair@08206b76; add-sync `FUN_08059668(&DAT_084e1180,...)` trong Add@082070a6 | **RE-4**: broadcaster represent 2010 = `KPlayerServer::DoSyncEquipRepresent(KPlayer*, int nIndex, DWORD id)` (`KPlayerServer.cpp:3400`, decl `KPlayerServer.h:346`), gọi qua `g_PlayerServer.` — được SetRepresentID gọi tự động khi giá trị đổi (`KItemList.cpp:3373`) | CHỐT (RE-4) |
| apply-hook → player field | ChangeHair@08206b76 ghi `*(short*)(pPlayer+8+(iType+0x4ad0)*2)` = short[2]@KPlayer+0x95a8 {face@0x95a8, hair@0x95aa} (v246) | **RE-4 GIẢI**: field CÓ SẴN 2010 = `KPlayer::m_wRepresentId[perRepresentCount]` (WORD[], `KPlayer.h:338`); KPlayer+0x95a8==`&m_wRepresentId[0]`. face=`m_wRepresentId[perFaceStyle]`, hair=`m_wRepresentId[perHairStyle]` (enum `PLAYER_EQUIP_REPRESENT`, `SO3GlobalDef.h:116`, perFaceStyle=0/perHairStyle=1). Setter `KItemList::SetRepresentID` (`KItemList.cpp:3361`)/copy `KExteriorBox::SetRepresentID` (`KExteriorBox.cpp:171`); broadcast `g_PlayerServer.DoSyncEquipRepresent` (`KPlayerServer.cpp:3400`). Port ChangeHair = SetRepresentID(perFaceStyle,face)+SetRepresentID(perHairStyle,hair) | GIẢI (RE-4) — KHÔNG hardcode 0x95a8, dùng enum+setter |

## B. Callee (KHair* GỌI RA — dependency, "cần port trước / tái dùng 2010")
| hàm KHair* | gọi | thuộc hệ | port-first? tái dùng 2010? |
|---|---|---|---|
| Init@081ee78c | LoadHairIndexTable@081ee3ca, LoadHairPriceTable@081edede; đọc g_pWorld+0x4f4 (close flag) | self / config | port cùng KHairShop |
| Load*Table | g_OpenTabFile + vtbl KTabFile (+0x18 GetHeight, +0x28 GetInteger) | KTabFile (đã có 2010) | tái dùng KTabFile 2010 |
| GetHairIndexInfo/GetPriceInfo | std::map find (this+4 / this+0x1c) | self | port thẳng |
| AddHair@081ed3fa | KHairBox::Find@082069b4, KHairBox::Add@082070a6, KHairBox::ChangeHair@08206b76, KLogClient::LogPlayerHairChangeRespond@081babb2 | KHairBox (NEW) + KLogClient (có 2010) | **port KHairBox trước** (RE-3); log tái dùng/stub |
| ChangePlayerHair@081ed5cc | Find, AddHair, GetTotalCost@081ecf88, CallBuyHairResultScript@081ed21d | self + KHairBox + Lua script | port cùng; script call tái dùng cơ chế Lua 2010 |
| KHairBox::ChangeHair@08206b76 | represent broadcaster FUN_0807c56e(&DAT_084e1180); ghi short[2]@player+0x95a8 | represent/appearance (có 2010, giống KExterior) | tái dùng broadcaster 2010 + field 2010 |
| KHairBox::Add@082070a6 | GetPriceInfo(g_HairShop@DAT+0x68890, player.roleType@+0xcfc), add-set FUN_08206e56, sync FUN_08059668(&DAT_084e1180) | self + represent | port cùng KHairBox |
| Buy@081edbd6 | CanBuy@081ec930, GetTotalCost, IsMoneyEnough@081ec866, Pay@081ecafc (money@player+0x6814: Get@0808d0cc/Add@082e3a9a), KLogClient@081b7554 | money in-game (có 2010) | HOÃN (bảng E) |
| OnHairShopBuyHairRespond@081ed7f4 | refund@0838b2ec, AddMoney@082e3a9a(player+0x6814), log BUY_HAIR_FAILED, KHairBox coin-op@08206d08 | COIN/元宝 + paysys (NEW) | HOÃN (bảng E) |

## C. Caller (AI GỌI VÀO KHair* — ai kích hoạt tính năng)
| entry | gọi hàm KHair* | qua đường |
|---|---|---|
| Lua content (NPC 发型师/hair NPC) | LuaOpenHairShop@081f1940 (top==0, set close=0) / LuaCloseHairShop | Luna<KHairShop> global object trong script |
| Lua content | LuaChangePlayerHair@081f19f0 (5 args: playerID, hair1[type1], hair2[type0], bFree1, bFree2) → ChangePlayerHair | Luna, GetPlayer(g_world+0x609dc)@08056a94 |
| Lua content | LuaBuyHair@081f1cb4 (5 args: playerID, (id,bool)×2 → KHAIRSHOP_BUY_PARAM) → owned?ChangeHair:Buy | Luna + GetPlayer |
| Lua content (getter UI) | LuaGetHairIndex@081f1f56 → GetHairIndexInfo; LuaGetHairPrice@081f20b4 → GetPriceInfo | Luna |
| paysys callback (元宝) | OnHairShopBuyHairRespond@081ed7f4 | net/center async (HOÃN) |

## D. Cross-version drift cần khoá (enum ordinal / offset / protocol id)
| giả định | v2.5 | 2010 | guard |
|---|---|---|---|
| player hair represent field (apply target) | short[2] @KPlayer+0x95a8 {face@0x95a8, hair@0x95aa} | **GIẢI (RE-4)**: `m_wRepresentId[perFaceStyle]` / `[perHairStyle]` (WORD[], `KPlayer.h:338`; enum `SO3GlobalDef.h:116`). KPlayer+0x95a8 == `&m_wRepresentId[0]` (khớp offset {0,+2}) | KHÔNG hardcode 0x95a8; dùng `SetRepresentID(perFaceStyle/perHairStyle, id)` — tự broadcast qua DoSyncEquipRepresent |
| KHairBox embed offset | KPlayer+0xb8a0 | compiler layout | truy m_HairBox qua tên member, không offset |
| player nRoleType | KPlayer+0xcfc (int, hợp lệ 1..6 = rtTotal) | dò getter roleType 2010 | dùng accessor, không offset |
| player money (in-game) | KPlayer+0x6814 (Get@0808d0cc, Add@082e3a9a) | KMoney 2010 | dùng API KMoney 2010 |
| ChangeHair guard flags | *(player+0xbc)==0 && *(player+0xc0)==0 | dò cờ trạng thái 2010 (biến thân/chiến đấu?) | xác minh ý nghĩa 2 cờ trước khi port guard |
| hsTotal / type enum | 2 (hsFace=0, hsHair=1) | định nghĩa lại trong KHairBox.h | enum literal, khớp Type cột HairPrice.tab |
| HAIR_SHOP_RESPOND_CODE | ok=1, closed=0xb, busy=0xd, fail=2 (từ Buy) | định nghĩa enum khớp Lua trả number | giữ ordinal khớp script content |
| Save role-block enum (rbt) | `rbtHairBoxData` | append trước rbtTotal | CHỐT (nm v246) |
| **DB serialize per-item** (RỦI RO CAO) | `KHAIR_DB_DATA::KHAIR_INFO` = 8 B: `WORD wID` @+0 + `BYTE byReserved[6]` @+2 (KRoleDBDataDef.h:591, byte_size 0x08); wID = LOW WORD của DWORD id in-mem | định nghĩa KHAIR_DB_DATA y hệt trong KRoleDBDataDef.h 2010 (chưa có) | KHÓA size=8, wID=WORD. Hair id PHẢI ≤ 0xFFFF (không thì mất bit cao) |
| **DB block header** | mỗi list: `WORD blockSize`(prefix) + `WORD wCount` + wCount×KHAIR_INFO. Tổng blob = block0 + block1 + `WORD freeCount` (trailing) | y hệt | KHÓA: freeCount = WORD (max 60000 < 0xFFFF, an toàn); 2 list (hsTotal=2) LUÔN ghi cả 2 block dù rỗng |
| freecount width | in-mem `int m_nFreeCount`@+0; serialize = WORD; clamp [0,60000] trong AddHairFreeCount | int in-mem, WORD on-disk | KHÓA WORD on-disk; nếu >65535 sẽ hỏng (nhưng cap 60000 chặn) |

## E. Defer (hoãn có lý do — không port lần này)
| hàm | lý do | phụ thuộc |
|---|---|---|
| Buy/Pay/CanBuy/IsMoneyEnough/OnHairShopBuyHairRespond/LuaBuyHair | buy chain depend currency (NEW) | KCurrency |
