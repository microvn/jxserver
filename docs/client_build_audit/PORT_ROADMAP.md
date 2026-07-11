# ROADMAP — Port client 1834 → 2.5.2 (có exterior, chơi được với server 4503)

Ngày: 2026-07-08. Quyết định: KHÔNG dừng ở build 1834; đẩy tiếp thành client 2.5.2-source.
Đối xứng với port server 1834→2.5.2 đã hoàn tất (memory [[jx3-linux-build-recovery]]).

## Bức tranh: 3 phase, blocker khác nhau

### Phase 0 — Build 1834 Debug (audit này) = LẤY BASELINE COMPILE
- Trạng thái: audit xong (REPORT/AUDIT.md). Chưa link được.
- Việc: host Windows+VS2008(ATL)+DXSDK June2010 → build RainbowD.lib từ source SwordOnline → gom lib → link Debug → linker liệt kê symbol (giống server 1332→32).
- Blocker: **toolchain Windows** (mac arm64 không chạy được).
- Ra: `JX3Client.exe` (Debug, 1834) — CHƯA có exterior, CHƯA nói được với server 4503.

### Phase 1 — Port protocol client↔server 1834 → 2.5.2
**Tin tốt (đã đo, diff header vs server-port tree `linux-build/include/Include/`):**
- **c2g** (`GatewayClientProtocol.h`, client→gateway) = **IDENTICAL** 1834↔2.5.2 → 0 việc.
- **g2c** (`GS_Client_Protocol.h`, GS→client) = DIFFER **đúng 1 dòng**: `GAME_WORLD_CURRENT/LOWEST_VERSION` 138→**246**. Struct layout header KHÔNG đổi.
- `Relay_GS_Protocol.h` (internal r2s/s2r) đã port ở server (+31 dòng) — client KHÔNG parse trực tiếp, bỏ qua.

**Cảnh báo (không được tin header):** server port phát hiện drift thật (S2R_HANDSHAKE 14→22B, enum r2s 110→178, INTERNAL_HEADER 3→2B) bằng **RE binary**, không phải diff header. g2c có thể vẫn drift trên dây mà **chưa ai phát hiện vì chưa client nào đi qua path đó**. Con `138→246` tìm ra vì stock GS gửi `0xf6`.

**→ Chiến lược: client 1834 build được = INSTRUMENT khám phá drift g2c**, đúng như stock GS binary từng là instrument cho r2s/s2r. Trỏ client vào server 4503, bắt tay, đọc lỗi parse → vá g2c struct theo binary. Việc này CHỈ làm được sau Phase 0 (cần client chạy). Đối xứng hoàn toàn với cách đã vá server.

### Phase 2 — Graft exterior vào client source (việc LỚN NHẤT)
- Source 1834 = **0 file KExterior**; `JX3Client.exe` 4550 = **101 hit "Exterior"** (memory). Engine render exterior chỉ có trong binary.
- Đường: RE KExterior từ server 4503 + client 4550 render ([[kexterior-port-re]], skill jx3-re-port) → tái tạo class C++ (KExterior/KExteriorBox/KDomesticate/KHairShop/KDiamond/KFellowPet) + hook render trong KG3DEngine/SO3Represent → graft vào source client.
- Đây là phần nặng và chưa có source ở đâu — phải RE. Không né được.

## Engine_lua5 chứa gì — và Phase 2 port ĐÚNG cái gì (chi tiết)

**Engine_lua5 = CƠ CHẾ scripting, KHÔNG chứa game-content.** 4 file glue (source ở Sword3-Full `.../base/base/SourceCode/Engine/Lua/`):
- `KLuaScript.cpp` — vòng đời VM: `Init/Load/LoadBuffer/ExecuteCode/ExecuteMain/CallFunction/CreateTable/ModifyTable/Exit` (chính là symbol `KLunaBase::_Register/GetLuaData` lib đóng export).
- `KLuaScriptEx.cpp` — môi trường sandbox script chạy trong: `_MakeEnv`, const-list, `AddGlobalInteger/String`.
- `KLuaWrap.cpp` + `lua_tinker.cpp` — marshalling C++↔Lua: `meta_get/meta_set/invoke/dobuffer` + allocator `_chunk_alloc`.
- `Luna.h` (576 dòng) — DSL macro binding: `DECLARE_LUA_CLASS / REGISTER_LUA_FUNC / REGISTER_LUA_INTEGER / REGISTER_LUA_ENUM…`.

→ Nó chỉ trả lời "class C++ phơi field/method nào cho script Lua". **0 KExterior, 0 logic game.** Vì thế lib đóng không chặn: bạn thêm `REGISTER_LUA_*` phía SOURCE, VM (cơ chế) xử lý generic.

**Port KHÔNG phải port Engine_lua5.** Port = 3 rổ, độ khó tăng dần:

1. **Engine_lua5 — port 0.** Build 1 lần từ glue-source (Sword3-Full) + Lua5-VM upstream, rồi LINK. Không sửa.
2. **KExterior data/logic — ĐÃ port, TÁI DÙNG.** `linux-build/src/SO3World/Src/{KExterior,KExteriorBox}.{cpp,h}, KExteriorDef.h` (RE cho server). Thuần DATA: đọc `.tab` (`g_OpenTabFile`), ôm ownership/suit/price/expiry, ánh xạ `nRepresentID`. **0 render.** SO3World build cả config Client → client được data này miễn phí.
3. **VIỆC THẬT còn lại = RENDER binding exterior trong SO3Represent** (+ đăng ký Lua). SO3Represent giữ máy `RepresentID→model` (tập trung `Src/case/gameworld/ktabledefine.h` 52 ref, `ktabletransform.h`, `kgameworldhandler.h`) NHƯNG **0 ref "exterior"** — tầng represent 1834 biết RepresentID gốc, chưa biết lớp phủ ngoại trang. Phải: (a) RE từ client 4550 cách exterior chồng lên represent (swap model/màu/texture trên nền RepresentID), thêm hook đó; (b) `REGISTER_LUA` class KExterior để script UI (ui.pak: tiệm ngoại trang/tủ đồ) điều khiển được.

## Thứ tự đề xuất
1. **Dựng host Windows build** (chặn cả Phase 0). Không có host thì cả roadmap đứng.
2. Phase 0: link Debug 1834 sạch (RainbowD + lib tên-lệch + symbol loop).
3. Phase 1: áp `GAME_WORLD 138→246`; dùng client chạy làm instrument đo drift g2c thật vs server 4503; vá theo binary.
4. Phase 2: RE-graft exterior (song song được với Phase 1 vì độc lập tầng protocol).

## Đòn bẩy tái dùng từ server port
- Server-port tree `linux-build/include/Include/` = nguồn diff header authoritative (đã làm sẵn 138→246, Relay drift).
- Kỹ thuật RE binary phân-tầng-drift (strace/header-as-ground-truth/#pragma pack) áp thẳng cho g2c client.
- Rainbow `Protocol/protocol.h` = đầu kia của kg_socket framing server (loop-closer 2 chiều).
