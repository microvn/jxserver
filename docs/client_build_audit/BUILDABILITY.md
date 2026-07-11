# BUILDABILITY — cái gì build được, cái gì KHÔNG, và port có khả thi không

Ngày: 2026-07-08. Câu hỏi cốt lõi: **nếu phần không-build-được nằm ở LÕI thì port bất khả.**
Kết luận ngắn: **KHÔNG có phần exterior-critical nào bị đóng.** Port khả thi — nhưng CHỈ nếu build từ cây **Sword3-FullSource** (không phải jx3dev).

## Cách phân loại (dứt khoát, không đoán)
Một lib "build được" ⇔ có `.vcxproj/.dsp` sinh ra nó HOẶC có `.cpp` nguồn. "Đóng" ⇔ chỉ tồn tại dạng `.lib` binary, 0 source, 0 project.

## TIER A — SOURCE-AVAILABLE (sửa + build tự do) ← exterior đổ vào đây

| Component | Sinh ra | .cpp | Exterior chạm? | Ghi chú |
|---|---|---|---|---|
| KG3DEngine (DLL) | KG3DEngine.dll | 339 | **CÓ — render** | full source cả 2 cây |
| SO3World / **SO3WorldClient** | SO3WorldClientD.lib | 249 | **CÓ — data-model** | SO3WorldClient = project SO3World, config `Client\|Win32` (`$(ProjectName)Client.lib`). KHÔNG phải binary riêng — **full source** |
| SO3Represent | JX3Represent.dll | 68 | **CÓ — ngoại hình nhân vật** | đây là tầng "represent" — nơi exterior appearance sống |
| SO3UI / SO3Interaction / KGUI / SO3Client | .dll/.exe | 27+16+79+4 | glue/UI | full source |
| **Base/Engine** (CHỈ Sword3-Full) | common/engine.lib | 76 | infra | gồm `Cryptography/`(MD5,EDOneTimePad=framing crypto!), `File/PackFile`, **`Lua/KLuaScript.cpp`** |
| ~~Lua glue (KLunaBase)~~ | — | — | — | **[SỬA] SAI: `KLuaScript.cpp` Sword3-Full là wrapper sản phẩm KHÁC.** Engine_lua5 thật = binary, xem §D |
| Binding ABI | headers | — | **CÓ** | `Luna.h`, `lua_tinker.h` — server đã regen `LUA_ATTRIBUTE_TYPE` phía source nhờ đây |
| Rainbow | RainbowD.lib | ~30 | net | source SwordOnline (đã verify interface byte-match) |

## TIER B — BINARY-ONLY nhưng KHÔNG chặn (chỉ link, không bao giờ sửa)

| Lib | Loại | Vì sao không chặn |
|---|---|---|
| **Lua5.lib** | Lua 5.x VM core (lapi/lgc/ldo…) | **Lua chuẩn OPEN-SOURCE** — 0 file trong leak nhưng dựng lại từ upstream Lua 5.x được. Không phải bí mật KingSoft |
| AHClientInterface / apex / PhysXLoader | NVIDIA PhysX/APEX cloth | third-party ổn định; exterior KHÔNG chạm physics |
| lzo2 | nén | open-source |
| **Engine_lua5D.lib** | IMPORT-LIB → `EngineD.dll` | **byte-identical với `engined.lib`** (md5 `0c4a902b`, 118882B). KHÔNG static-lib, KHÔNG glue-source. Bind `EngineD.dll` (**không** phải file `Engine_Lua5D.dll`). DLL thật = `EngineD.dll` 569344B md5 `aefb1317` (jx3dev SO3Gateway). Export class `KJxScript/KScriptGroup/KJxScriptSet/KLunaBase` + KPackFilePartner/KWin32App/KTimer/KIme… VM Lua nhúng tĩnh. VS2005 CRT. **CÓ SẴN trong leak → link+ship, không rebuild** |

## §D-bis — Engine_lua5 = DLL prebuilt CÓ SẴN, không phải blocker

Đính chính lượt trước (đã tưởng glue = `KLuaScript.cpp` Sword3-Full — SAI, đó là wrapper sản phẩm khác). Sự thật mở từ archive: `Engine_lua5D.lib` là **import-lib của `EngineD.dll`** (runtime engine nền KingSoft: script Lua `KJxScript` + binding `KLunaBase` + pak/app/thread/IME). DLL **tồn tại trong leak**. → link import-lib + ship DLL, KHÔNG cần source, KHÔNG rebuild. Server build đang làm đúng vậy nên boot được. "Đóng" chỉ đau khi cần khớp bảng-binding nội bộ DLL (vd enum `LUA_ATTRIBUTE_TYPE` — extract từ binary bằng `extract_lua_attr.py`, không sửa DLL được).

## TIER C — VERDICT: exterior có đụng phần đóng không?

**KHÔNG (theo nghĩa: không phải MODIFY byte đóng nào).** Chuỗi phụ thuộc của exterior:
- dữ liệu ngoại trang (KExterior/Box…) → **SO3World** (source ✓, đã port `linux-build/src/SO3World/Src/`)
- render ngoại trang → **KG3DEngine** (source ✓)
- ngoại hình nhân vật/represent → **SO3Represent** (source ✓)
- bind Lua cho hệ ngoại trang → macro `REGISTER_LUA_*` (Luna.h, source ✓) chạy qua `KLunaBase` trong **`EngineD.dll` prebuilt** (link, không sửa)

Engine_lua5 (`EngineD.dll`) là DLL prebuilt — LINK+SHIP chứ không sửa. → exterior không cần recompile bất kỳ byte đóng nào; chỉ cần **DLL đúng bản khớp ABI 1834** mà source mong đợi.

## Rủi ro THẬT DUY NHẤT (không tô hồng)

Các prebuilt `Engine_lua5D.lib / commonD.lib / SO3WorldClientD.lib` là **binary đời 1834**. Nếu build source game-object ở mức 2.5.2 (thêm class exterior) rồi phải gọi vào lib đóng 1834 có ABI đã drift → mismatch link/runtime **không sửa được vì không rebuild được lib đóng**. Đây đúng lớp lỗi server đã gặp (attribute-type enum drift) nhưng server vá được vì enum nằm phía SOURCE.

**Cách khử rủi ro:** build **TẤT CẢ từ source cây Sword3-Full** (có Base+Engine-glue source; SO3World/Represent/Engine đều source), thu bề mặt đóng còn đúng: Lua5-VM (open, dựng lại) + PhysX (NVIDIA ổn định, exterior không đụng). → với Sword3-Full, bề mặt đóng exterior có thể chạm phải ≈ **0**.

→ **Đó là lý do kỹ thuật (không chỉ "tự-chứa hơn") để chọn Sword3-FullSource làm cây build, không phải jx3dev.**

## Việc kiểm-chứng còn phải làm trước khi cam kết port

1. Xác nhận `base/base.sln` (Sword3-Full) build ra được `common/engine/Engine_lua5` từ 76 .cpp — tức thay được toàn bộ prebuilt lib đóng jx3dev. (cần host Windows).
2. Dựng lại `Lua5.lib` từ upstream Lua 5.x đúng minor version (đọc version từ `lua_tinker`/header).
3. Dump ABI `Engine_lua5D.lib` (đã thấy `KLunaBase::_Register`/`GetLuaData`) đối chiếu với `KLuaScript.cpp` — xác nhận source == binary (không phải bản glue khác).
