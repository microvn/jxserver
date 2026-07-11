# JX3 (剑网3) — Tài liệu thiết kế gốc của dev: Kiến trúc & Luồng server

Tổng hợp từ tài liệu thiết kế nội bộ Kingsoft trong `source/JX3-AIO/Document/`.
Mỗi kết luận ghi rõ file nguồn. Trích dẫn nguyên văn (中文) + dịch nơi quan trọng.

Nguồn chính đã đọc:
- `游戏世界/SO3World的分类说明.doc` (phân loại class world)
- `游戏服务器/剑三同步策略.doc` (đồng bộ — dài, nhiều công thức + struct)
- `游戏服务器/移动同步策略.txt`, `游戏服务器/如何判断游戏卡的原因.txt`
- 4 flow chart `.jpg`: 客户端连接的状态流转图 / 玩家登陆流程图 / 玩家跨服流程图 / GameCenter上玩家状态流转图
- `游戏世界/`: NpcAI.txt, Quest.txt, 攻击过程描述.txt, 魔法状态.txt, 道具相关配置文件.txt, 剑网三-魔法属性-说明文档.doc
- `附魔实现.txt`
- `猪的文档/中转服务器/Relay设计文档.doc` + 帮派系统设计.txt, `任务系统设计/任务概念.txt`

---

## 1. Kiến trúc process server

Từ 4 flow chart trong `游戏服务器/*.jpg` + `Relay设计文档.doc`, cụm server gồm các tiến trình:

| Process | Vai trò |
|---|---|
| **Client (客户端)** | Game client, giữ state-machine kết nối (gsInvalid → … → gsPlaying) |
| **Bishop / Gateway** | Cổng đăng nhập + xác thực account/mật khẩu; trung chuyển client ↔ GameServer. Trong luồng login gọi là `INGATEWAY` → `INGAMESERVER` |
| **GameServer (GS)** | Chứa **KSO3World** — thế giới game chạy logic. Nhiều GS = nhiều "mảnh" bản đồ; player nằm đúng trên **một** GS tại một thời điểm |
| **Relay / GameCenter** | "Trung chuyển" (中转) + **tổng control** toàn cục. Cầu nối giữa các GS, giữ dữ liệu global và logic global. State player quản lý ở đây (psOffline/psLogin/psOnline/psTransfer) |
| **Database** | Kết nối từ Relay để lưu/đọc |
| **RelayServer** | Relay kết nối với RelayServer khác (liên-Relay) |

Trích `Relay设计文档.doc`:
> "Relay的主要功能主要是充当GS之间的桥梁… 随着Relay上的功能越来越复杂… 负责维护全局数据和处理全局性逻辑的功能日渐增多"
> (Relay ban đầu là cầu nối giữa các GS, sau gánh thêm data global + logic global — ranh giới "trung chuyển" và "tổng control" mờ dần.)

Relay kết nối 4 phía (mục 协议处理): **Bishop, GS, Database, RelayServer**. Dữ liệu global Relay giữ: player online, **map副本 (副本/instance)**, friend, **bang phái (帮派)**, biến task thế giới (数据中心 → 数据的分类).

**Luồng packet client↔server** (từ `玩家登陆流程图.jpg`): Client → Bishop(Gateway) để auth → Bishop báo Relay "通知玩家登陆" → Relay `psLogin` → GS "批准玩家登陆" → GS cho phép → Client **kết nối trực tiếp GameServer** → sync data → chơi. Tức auth qua Gateway, còn gameplay đi thẳng client↔GS; Relay điều phối ở tầng control chứ không chuyển tiếp từng gói gameplay.

---

## 2. SO3World taxonomy (`SO3World的分类说明.doc`)

Dev chia class theo 3 mức ưu tiên (完成度): **高/中/低** (cao = xong/quan trọng, trung = một phần, thấp = code thử nghiệm).

**Core / khung thế giới:**
- `KSO3World` (高) — "封装了整个游戏世界" (đóng gói toàn bộ world). Đây là root.
- `KScene` (高) — lớp bản đồ (地图), đóng gói data + thao tác bản đồ.
- `KRegion` (高) — vùng (đơn vị đồng bộ 9-Region, xem mục 4).
- `KCell` (高) — ô lưới trên bản đồ (1m×1m).
- `KMapFile` / `KMapListFile` (高) — đọc/ghi file bản đồ + danh sách bản đồ ("注意宏定义，很复杂").

**Object model / quản lý bộ nhớ:**
- `KBaseObject` — base cho mọi object có ID cần quản lý memory (chỉ struct data).
- `KBaseObjectSet<T>` / `KBaseObjLiteSet<T>` (高) — template quản lý bộ nhớ object (bản Lite bỏ tìm-theo-ID).
- `KSceneObject` (高) — "场景物体类，出现在地图上的所有东西的基类" (base cho mọi thứ hiện trên bản đồ).
- `KObjectManager` (低) — gom hết class quản lý memory, "仅仅是个大杂烩" (mớ tạp).
- `KIDCache<T>` (高) — cache theo ID, chủ yếu cache gói protocol.

**Nhân vật (角色) — state machine:**
- `KCharacter` (中) — base của Player + Npc, "实现了角色状态机的绝大部分内容" (chứa phần lớn state machine nhân vật).
- `KPlayer` (中), `KNpc` (中), `KMissile` (高, đạn/子弹), `KDoodad` (低, vật trang trí).
- `KNpcAI` (低) — "M2的另时代码，结构有待重新设计" (code tạm, cần thiết kế lại — xem mục 5).
- `KNpcTemplateList` / `KDoodadTemplateList` — bảng template.

**Item / kỹ năng / thuộc tính:**
- `KItem`, `KItemList`, `KPlayerItemList` (低) — item + túi đồ.
- `KSkill` (中), `KSkillList` (中, danh sách skill khả dụng của nhân vật), `KSkillEvent`, `KNpcFightSkill`.
- `KAttrModifier` (中) — hàm sửa thuộc tính: "现在只有若干示例代码… 后期会扩大到10000行以上，300－500个函数" (rất quan trọng, sẽ phình to).
- `KRecipe` / `KProfession` (nghề/sinh hoạt — "胡子写的" / do người khác viết).

**Quest / script:**
- `KQuest` / `KQuestLink` (低) — "试验代码，没写几行" (mới thử nghiệm).
- `KScriptCenter` (高) — trung tâm script (đọc + chạy script), `KScriptFuncList` (中).

**Khác:** `KDropList` (bảng rơi đồ), `KWorldSettings` (低, gom setting), `KCircleList` (高, queue vòng thread-safe), `KRegion/KCell/KObstacle` (dữ liệu ô/chướng ngại).

Ghi chú cuối doc: các class song song ở process khác — `KPlayerServer` (trong SO3GameServer), `KPlayerClient` (SO3Client), `KRelayClient` (client nối Relay, chờ code cross-server xong mới test).

---

## 3. Login flow + Cross-server flow + Connection state

### 3a. Connection state machine của client (`客户端连接的状态流转图.jpg`)
Chuỗi state client-side:
`gsInvalid` → (client nối GS) → `gsWaitForGUID` → (client gửi yêu cầu sync) → `gsSyncData` → (client xác nhận sync xong) → `gsPlaying`.

Hai state đổi bản đồ: `gsSearchMap` (khi chuyển sang map không tồn tại trên GS hiện tại) và `gsTransferData` (khi cross-GS, tìm được GS đích + GS đích đồng ý nhận). Mọi state đều rơi về `gsInvalid`/断开连接 khi: timeout chờ sync, timeout chờ ACK, timeout PING, hoặc nhận gói bất thường.

### 3b. Login flow (`玩家登陆流程图.jpg`)
Actor: Client / Bishop / GameServer / Relay. State song song: client `gsXxx`, Relay `psXxx`.
1. Client(`gsInvalid`) → nối **Bishop**, gửi account+mật khẩu → Bishop `INGATEWAY`.
2. Bishop gửi lại **角色资料** (dữ liệu nhân vật) → client `gsWaitForGUID`.
3. Client chọn nhân vật, "请求登陆" → Bishop `INGAMESERVER` → báo Relay "通知玩家登陆" → Relay `psLogin`.
4. Relay "通知玩家登陆" xuống GS (GS `gsWaitForGUID`) → GS "批准玩家登陆" ngược lên Relay (`psLogin`) → "批准玩家登陆" về Bishop.
5. Bishop "批准登陆" → client `gsSyncData`, và client **连接 GameServer** trực tiếp (GS `gsWaitForGUID`).
6. Client "请求同步玩家数据" → GS `gsSyncData` → "同步玩家数据" về client `gsPlaying` → "确认同步完成" → GS `gsPlaying` → "玩家登陆完成，开始游戏" → Relay `psOnline`.

### 3c. Cross-server / đổi bản đồ (`玩家跨服流程图.jpg`)
Chia 2 ca:
- **不跨GS (đổi map trong cùng GS):** client `gsSearchMap` → GS `gsSearchMap` "请求换地图" → Relay(`psOnline`) "同意换地图" → GS `gsPlaying` → client "换地图成功" `gsPlaying`. Nhẹ.
- **跨GS (đổi map sang GS khác):** client `gsSearchMap` → Src GS `gsSearchMap` "请求换地图" → Relay `psTransferGS`. Relay hỏi **Dest GS** "请求换地图" (Dest `gsWaitForGUID`) → Dest "同意接收" → Src GS `gsTransferData` "发送角色数据" → Relay → Dest GS `gsWaitForGUID` "确认角色数据" → Relay `psTransfer(GS:Dest)` → Src GS `gsInvalid`, báo client "通知新GS的地址和GUID" → client `gsSyncData` → nối Dest GS, "请求角色数据" → Dest `gsSyncData` "发送角色数据" → client `gsPlaying` "确认角色数据" → Dest GS `gsPlaying` → Relay `psOnline(GS:Dest)`.

Điểm cốt lõi: **transfer đầy đủ nhân vật giữa 2 GS**, Relay là điều phối, client re-connect sang GS đích với GUID mới.

### 3d. GameCenter player-state machine (`GameCenter上玩家状态流转图.jpg`)
Đây là state chi tiết ở GameCenter/Relay, tiền tố `rs*` (relay-side). Trục: `rsOffline` ↔ `rsOnline` với các state trung gian cho login (`rsLG_WaitForLogin/Permit/CreateMap`) và cross-server (`rsCG_WaitFor…`: CreateMap, PlayerData, Guid, Login_Logout, SrcTimeout, DestTimeout, SrcLogout, DestLogin). Ghi chú trong ảnh (注释) quan trọng:
> "Src是指角色当前所在的服务器，Dest是跨服的目标服务器" (Src = GS hiện tại, Dest = GS đích cross-server).
> "除了SendPlayerData…之后的状态以外，角色处于其他所有状态时，都明确的存在于一台GS上，如果这台GS宕机，则等同于所有玩家退出，立刻切换至rsOffline状态" (trừ vài state sau SendPlayerData, mọi lúc player nằm rõ trên 1 GS; GS đó chết → coi như logout → `rsOffline`).
> Xử lý lỗi chia 2 loại: lỗi lường trước → có state Failed; lỗi "logic không thể xảy ra" → dump memory rồi tắt server.

---

## 4. Sync strategy (`剑三同步策略.doc` + `移动同步策略.txt`)

**Mục tiêu thiết kế** (2. 设计目标): giải độ trễ khi loạn chiến đông người; giảm băng thông; giảm lệch dữ liệu client↔server (chủ yếu toạ độ, rồi tới máu + trạng thái).

**Đồng bộ theo 9-Region.** Mỗi client chỉ sync 9 Region quanh mình (Region rộng 16m). Qua Region là xử lý đặc biệt: xoá object của các Region rời khỏi, thêm object của Region mới. Công thức thời gian sync: `Interval = RegionWidth / PlayerVelocity`, RegionWidth=16m, tốc độ tối đa 16m/s (=1 cell/frame), mục tiêu ≤2s sync xong 3–5 Region mới. Grid đổi từ 0.5m×0.5m sang **1m×1m** (2005.05.09) để đạt mục tiêu.

**Hai loại đồng bộ** (名词约定):
- **状态同步 (state sync):** định kỳ gửi dữ liệu bất kể có đổi hay không.
- **指令同步 (command sync):** chỉ gửi khi data đổi hoặc client request.
- **逻辑数据 (logic data — luôn phải sync, đổi rất nhanh):** toạ độ hiện tại, toạ độ đích, tốc độ, state máy trạng thái, magic-state hiển thị, %máu/%nội lực/%nộ khí.
- **显示数据 (display data — ít đổi):** tài nguyên hiển thị, tên, phe (阵营).

**JX1 vs JX3.** JX1 dựa hết vào state-sync mỗi 2 frame (tối đa 81 Npc/s/player, ~1240B/s ≈ 73% băng thông xuống). JX3 tách chức năng "phát hiện player mới" khỏi state-sync để bỏ dữ liệu dư, hạ tần suất/kích thước gói, và phân tầng tần suất sync theo trong-thành/ngoài đồng, chiến-đấu/không, target/không-target.

**Đồng bộ toạ độ di chuyển (5.2.1) — chiến lược JX3:**
> "《剑网3》策略：变种的以客户端为准，服务端校验修正" (biến thể "client làm chủ, server kiểm tra & sửa").

Cơ chế: (1) **lịch sử vị trí** — server ghi vị trí/tốc độ từng frame trong ~10s; (2) gói lệnh di chuyển mang **toạ độ nguồn + đích + frame thời điểm** (mọi gói JX3 đều mang frame). Server nhận gói → **rollback** nhân vật về vị trí ở frame của gói rồi mới đối chiếu toạ độ nguồn: khớp → xử lý di chuyển, lệch → sửa client. Client chậm (bình thường) → rollback; client nhanh (bất thường: máy server yếu / bug / hack) → server chạy đuổi. Nhờ vậy hầu như không bị "kéo về", chống dịch chuyển tức thời (瞬移外挂).

Từ `移动同步策略.txt`: gói lệnh di chuyển = 协议号 + `nSendFrame`(8) + `nDirection`(1) + `nFrame`(1) = 11B; biến `nLastFrame` giới hạn rollback; hành vi ghi đè `nLastFrame`: mọi gói client (TCP có thứ tự) + hành vi cưỡng bức server (gặp chướng ngại đứng lại, trúng đạn). **"所有帧数以服务端为准"** (mọi frame lấy server làm chuẩn).

**Phát hiện object mới khi qua Region (5.2.2):** dựa vào **client phát hiện** — client nhận gói broadcast (di chuyển/nhảy/skill) có ID lạ thì request logic-data của ID đó. Player thụ động (đứng bán hàng) không broadcast → giải bằng **强制同步 (forced sync)**: sau khi vào Region mới ~2s, cưỡng bức gửi tư liệu các nhân vật chưa hoạt động. Cần 2 biến trên mỗi nhân vật: `m_nLastBroadcastFrame`, `m_nEnterRegionFrame[9]`. Struct gói: `G2C_NEW_CHARACTER_INTO_REGION` (17B, bit-packed) và `G2C_ALL_CHARACTER_IN_REGION` (mảng `KSyncCharacter` 15B).

**Chẩn đoán lag (`如何判断游戏卡的原因.txt`):** 4 nguyên nhân — (1) CPU server quá tải, (2) băng thông ra đầy, (3) độ trễ link, (4) client render yếu. Triệu chứng phân biệt: di chuyển thẳng nơi đông người bị **kéo về** → nguyên nhân 1 (CPU).

---

## 5. Logic subsystem tổng quát

**NpcAI (`NpcAI.txt`, class `KNpcAI`/`KAIController`).** 2 state cố định **OnIdle / OnFight**, hàm state chạy định kỳ mỗi ~0.5–1s (Npc chủ động quét môi trường + bản thân). Sự kiện không để lại "dấu vết" (vd Npc bị trọng kích) báo AI ngay qua `TriggerEvent`. `Activate` phân phối hàm xử lý theo loại AI. `KAIController` có 2 nhóm biến: **Params** (interface cho planner chỉnh) + **Variables** (state tạm). Có AI bầy đàn mẫu `WolfKing` + khái niệm đội hình (阵型). 2 toạ độ: `OriginPoint` (vị trí đặt trong editor) và `ReturnPoint` (vị trí lúc Idle→Fight để quay về roam/patrol). Dev tự đánh giá đây là code tạm cần thiết kế lại.

**Quest (`Quest.txt` + `任务概念.txt`).** 3 class: `KQuestInfo` (data từ bảng config), `KQuestList` (quest trên mỗi Player — mỗi Player có 1), `KLuaQuestInfo` (map sang Lua cho UI). Mô hình khái niệm: **任务链 (quest chain) = tập 任务 (quest)** nối bằng một cờ trạng thái; **任务 = tập điểm-kích-hoạt (触发点) + điều kiện + đổi cờ trạng thái + script mở rộng**. Bảng quest-chain có: NPC nhận/giao, điều kiện level + thiện-ác-độ, thưởng exp/danh vọng/item, quest kế. Ở Relay: 1 task ↔ 1 script, task là **hữu hạn trạng thái (state machine)**, mỗi task nhiều "task副本" (副本 = instance) cho attв data như list player, số hiệu map-copy.

**Combat (`攻击过程描述.txt`).** Có attacker (prefix `A_`) + defender (`D_`); điểm `_V`, phần trăm `_P`. Luồng:
1. `命中指数 = A_命中率 − (D_闪避率 − A_减对方闪避率_V) × A_减对方闪避率_P`; random < 命中指数 → trúng, không thì dừng.
2. `攻击力 = A_基础攻击力 × A_攻击力提升_P + A_附加攻击力`; random < 暴击率 → ×200% (bạo kích).
3. `伤害 = (攻击力 − D_防御力_V) × (1 − D_防御力_P)`.
4. Máu giảm = 伤害 × (1 − 魔法吸收_P); nội lực chia phần hấp thụ; có phản đòn (攻击反弹), hút máu/nội (偷取生命/内力). (Sơ đồ chi tiết `攻击计算流程.vsd` — file Visio, không convert được.)

**Magic attribute / state (`魔法状态.txt` + `剑网三-魔法属性-说明文档.doc`).** Mô hình: **角色数值 (character values)** = biến trên nhân vật; **属性 (attribute)** = mô tả cách sửa các value đó (data), "应用/apply" = action (code/hàm) đưa attribute vào nhân vật. Trang bị + skill đều mang **tập attribute**: trang bị apply vĩnh viễn tới khi tháo; skill apply rồi gỡ theo 3 kiểu (không gỡ / gỡ ngay / gỡ trễ). Khái niệm **属性集合 (attribute set)** (2005.09.26) mang tham số điều kiện: khi nào apply, khi nào gỡ, xác suất apply, đối tượng (target/source). **事件 (event)** = điểm phán đoán trên luồng tính value; thoả điều kiện thì tự dùng skill trong event-list. **BUFF** 3 quan hệ cùng tồn tại: **互斥 (loại trừ) / 可叠加 (chồng) / 可替代 (thay thế — cần level ≥ mới thay)**; BUFF âm/dương là một loại phân loại skill. Doc thuộc tính quy định naming (`EnhanceMagicDefence_v`, hậu tố `_v`/`_p`), phân loại (ảnh trực tiếp value vs BUFF có thời hạn), cờ 是否存盘 (BUFF có lưu khi offline không), 4 tham số. (Bảng số liệu ở các `.xls` — không đọc trong task này.)

**Enchant (`附魔实现.txt`).** Ngắn gọn: thêm 1 bảng "tập thuộc tính ma pháp" (tạm chỉ cho phụ ma); trong `KItem::m_GenParam` thêm 1 ID tập附魔 (dùng cho lưu + truyền protocol); lúc sinh trang bị bung ID này thành **danh sách attribute** gắn vào trang bị.

**Bang phái (`帮派系统设计.txt`).** Cấu trúc **cây** (thống nhất memory + DB): mỗi node ~248B (角色流水号, cha, con đầu, anh-em kế + info + 权限×64 mỗi cái 2B + 指令 20B). Ước tính 10^5 player nhập bang ≈ 25MB/GS+Relay. Quyền mô tả 3 phần: có được không / hạ phóng được không / đối tượng thao tác. **指令 (command)** truyền theo cấp, có thể phân rã thành lệnh con, xong biến mất.

---

## 6. File KHÔNG đọc được / bỏ qua trong task này
- `.vsd` (Visio): 攻击计算流程.vsd, các *.vsd flow-chart gốc, 设计类图.vsd, 地图副本创建/删除/申请流程.vsd, 地图加载最新流程图.vsd — nhưng bản `.jpg` tương ứng đã đọc (login/connect/cross-server/GameCenter). Sơ đồ dynamic-map-copy chỉ có `.jpg` chưa đọc chi tiết (创建/删除/申请流程).
- `.xls`: các bảng魔法属性/装备/技能/QuestItem/演示任务链/动态地图测试 — data số, không thuộc phạm vi kiến trúc.
- `.ppt`: 动态地图加载.ppt, 剑网三客户端介绍.ppt — chưa convert.
- `.wps` (Kingsoft): MySQL监控/文件备份需求 — công cụ vận hành, ngoài phạm vi.
- `3DEngine/*`, `UI/*`: thuộc client/engine, không phải kiến trúc server.
- `剑网客户端数据包分析/*`: dữ liệu đo băng thông thô (là nguồn cho các con số trong 同步策略).
