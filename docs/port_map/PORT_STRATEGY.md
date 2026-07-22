# PORT STRATEGY — thứ tự & nguyên tắc port 2010 → v2.5

> **THAY BẰNG `FEATURE_ATLAS.md` (2026-07-13).** Khung tier-coupling dưới đây (leaf→hub) tối ưu SAI
> thứ: nó theo coupling code, không theo playability, nên chơi-được rất muộn. Kế hoạch port hiện dùng
> = atlas: chọn theo playability (dọc), thực thi coherent theo struct CHỈ trên serialization spine.
> File này giữ làm **tham chiếu dependency/cạm-bẫy**, không còn là thứ tự port.

Bổ trợ cho `PORT_MAP.md` (số liệu) và `PORT_MAP.csv` (danh sách hàm). File này = **chiến lược**:
port cái gì trước, vì sao, cạm bẫy.

## Sự thật về quy mô (đọc trước khi lạc quan)

- Tổng port thật = **1581 method** (54 NEW class / 438 method + 54 DRIFT class / 1256 method — 135 name-elsewhere cần verify riêng).
- **NEW self-contained chỉ 28%.** 72% khối lượng nằm trong **hub DRIFT**, và 10 class gánh 1059/1256:
  KPlayer 344, KPlayerServer 184, KRelayClient 154, KScriptFuncList 123, KItemList 70,
  KCharacter 54, KScene 41, KLogClient 41, KNpc 26, KStatDataServer 22.
- Leaf-first **không** giảm khối lượng hub — nó chỉ (a) build momentum rủi ro thấp, (b) hoàn thiện
  quy trình RE dựa trên **DWARF** (`SO3GameServerD`) trước khi đụng hub nơi 1 offset sai = hỏng cả hệ.

## Nguyên tắc chọn thứ tự: "ít linked/binding/depend" trước

"Ít method" ≠ "ít depend". Xếp theo coupling THẬT (đọc từ signature + bản chất subsystem):

### Tier 1 — leaf thật, self-contained (port trước)
| class | loại | depend | ghi chú |
|---|---|---|---|
| **KHairShop + KHairBox** | NEW ~35 | currency (chỉ cho buy) | **PILOT** — song sinh KExterior, tái dùng case study |
| KMiniAvatar (+Settings) | NEW ~14 | ~không | Acquire/Sync/Lua, không buy — leaf sạch nhất |
| KDesignation (称号) | DRIFT 16 | ~không | title prefix/postfix, đã có 15/31 |
| KRegressionManager (+PlayerData) | NEW ~20 | tab+reward | singleton Init() không param |
| KRankListServer | NEW 3 | network sync | đọc+sync rank |

### Tier 2 — dependency-root (mở khóa Tier khác)
| class | vì sao port ở đây |
|---|---|
| KCurrency + KCurrencyList | nền cho MỌI buy chain (exterior/hair/rewards). Port để mở khóa các nhánh mua đang hoãn. Data đơn giản (Init(KPlayer*)+Save/Load) nhưng nhiều thứ chờ nó. |
| KDiamondManager | đụng IItem (đá quý gắn đồ) — nặng hơn currency, hoãn tới khi item ổn |

### Tier 3 — coupling cao (đừng bị đánh lừa bởi count nhỏ)
- **KTalentSkill** (天赋) — CallScript + talent set → đụng **skill hub**.
- **KTongServer / KAntiFarmerServer** — hook `OnPlayerLogin/EnterScene` → coupling **player/scene event**.
- **KActivityMgrServer / KCampInfo** — activity hub.
- Các traverse-functor trong skill-script (KBroadcastCastSkillFunc, KSkillRectangleTravFunc,
  GetSkillRecipeTraverseFunc) — **không phải leaf độc lập**, port kèm hub tương ứng.

### Tier 4 — HUB (cuối cùng, khối lượng lớn nhất)
skill-script (KScriptFuncList 123) → world-scene (KScene/KNpc/KDoodad) → item (KItemList 70) →
player-hub (KPlayer 344 + KPlayerServer 184 + KCharacter 54). Sai offset ở đây lan toàn hệ →
chỉ làm sau khi quy trình đã chín + có drift-guard.

### Tách riêng — network-infra (đánh giá per-method, không chắc gameplay)
KRelayClient 154, KLogClient 41, KStatDataServer 22, KLocalGSDataStat, KCenterRemote, KG_*.
Phần lớn là protocol handler; tier network đã thông (milestone #22). Không port hàng loạt.

## Cạm bẫy chung (từ verify KExterior/Box)

1. **Cross-build name drift**: DWARF debug build vs stripped release `-3c8199` đặt TÊN HÀM khác nhau
   (debug `AddExteriorIntoSet/Find` ↔ release `SetExterior/GetExteriorItem`). Kiểm cả hai trước khi
   kết luận "thiếu/sai tên".
2. **Buy chain depend currency (NEW)** — mọi shop (hair/exterior/rewards) chỉ port được **grant-path**
   cho tới khi KCurrency xong; hoãn nhánh Buy/Pay có tài liệu, đừng stub gãy.
3. **Lua binding hình dạng**: v2.5 dùng `Luna<KClass>` static; source hay remap thành `KPlayer::` method.
   Quyết định hình dạng khớp binary TRƯỚC khi register — nếu không phải làm lại khi port KLuaPlayer
   (xem TODO.md §1: 6 binding KExterior đang chết).
4. **Serialize/DB byte-layout PHẢI khớp**; member offset trong RAM thì tự do (compiler layout).
   role-block enum: append trước `rbtTotal`, không chèn giữa.

## Pilot đang chạy: KHairShop + KHairBox

Mục tiêu kép: (1) port grant-path hoàn chỉnh, (2) chạy hết quy trình **DWARF-based** để rồi
**viết lại `.claude/skills/jx3-re-port/SKILL.md`** — skill hiện hướng dẫn theo lối cũ (chỉ có
stripped `__PRETTY`, chưa có DWARF `SO3GameServerD`). Docs port sẽ ở `docs/hair_port/`.
