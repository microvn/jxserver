# Cấu trúc thư mục port (convention — dùng cho MỌI subsystem)

Mỗi lần port một subsystem tạo 1 thư mục: `linux-build/docs/<subsystem>_port/`
(vd `exterior_port/`, `designation_port/`, `domesticate_port/`).

## File chuẩn trong mỗi thư mục
- **REPORT.md** — báo cáo tiến độ: thời gian, chỉ số, khuyến nghị (đọc đầu tiên).
- **PORT_DESIGN.md** — thiết kế port. BẮT BUỘC có mục **§1 LINKED/RELEVANT** (ma trận phụ thuộc + cross-ref) và **thứ tự port BÉ->TO** (leaf trước, tránh TODO treo).
- **WORKLOG.md** — nhật ký RE: địa chỉ hàm (FUN_xxxx), offset, struct, decompile findings theo block [RE-1], [RE-2]...
- **backup_source_YYYYMMDD_HHMMSS/** — backup file gốc TRƯỚC khi chạm, kèm `MANIFEST.md` (gì/khi nào/vì sao). CHỈ copy, KHÔNG xoá gốc.

## Quy ước đặt tên backup
`backup_source_<YYYYMMDD>_<HHMMSS>/` — có ngày+giờ để tra cứu không rối; nhiều lần backup = nhiều thư mục theo thời gian.
Bên trong giữ NGUYÊN cây thư mục tương đối (src/SO3World/Src/...) để biết file gốc ở đâu.

## Ràng buộc (goal)
- Cách li + backup TRƯỚC khi chạm source. Bản gốc bất khả xâm phạm cho tới khi chủ đích chỉnh.
- **KHÔNG XOÁ BẤT KỲ GÌ** — nếu cần xoá thì DỪNG, hỏi user.
- Fix source + build; KHÔNG patch exe.
- Values-from-binary: mọi offset/struct/enum từ Ghidra/objdump, không đoán.
