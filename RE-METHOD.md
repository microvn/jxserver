# RE Method: đọc binary JX3 bị pack + đối chiếu với source

Phương pháp reverse-engineer các binary server JX3 (2.5.2) — kể cả khi bị **pack/obfuscate** —
và cách so sánh định lượng với source 2010. Đúc kết 2026-07-06 (DECISION §R4, milestone #15).

Tools kèm theo: `tools/dump_packed_process.py`, `tools/wrap_dump_elf.py`, `tools/re_center_dump.py`,
`tools/re_center_handshake.py`.

---

## 0. Binary nào packed, binary nào không

| Binary | Trạng thái | RE thế nào |
|---|---|---|
| `SO3GameServer` | **KHÔNG pack** (plaintext, ~423 symbol sạch) | RE thẳng (pyghidra import + analyze) |
| `SO3GameCenter` | **PACKED** (custom packer) | Phải runtime-dump (mục 2) |
| `SO3Gateway` | **PACKED** | Runtime-dump |
| `libEngine_Lua5*.so`, `libSO3*.so` | KHÔNG pack | RE thẳng |

**Nhận biết packed (không cần chạy):**
```
readelf -l SO3GameCenter
```
Dấu hiệu: entry point nằm trong LOAD segment **RWE** (Read-Write-**Execute**); LOAD đầu `FileSiz`
rất nhỏ (0x187) nhưng `MemSiz` lớn (3.7MB) = giải mã lúc chạy; các LOAD chồng lấn offset.
Cross-check: `strings -a bin | grep -cE '^_Z|at line'` — packed ra vài trăm rác entropy-cao thay vì
hàng nghìn symbol/format-string sạch. (Center: 36197 "strings" nhưng chỉ ~186 symbol thật.)

Không có magic UPX/Themida → **custom packer** (kiểu server TQ dùng để chặn sửa center/DB).

---

## 1. RE binary KHÔNG pack (GameServer, .so)

pyghidra-mcp (config `~/.claude.json`, project `linux-build/ghidra-project`):
- `import_binary` **KHÔNG tự analyze** — kiểm `list_project_binary_metadata` thấy `Analyzed=false`,
  chỉ ~564 hàm. Phải chạy pyghidra **standalone** `analyze=True` (như `tools/extract_lua_attr.py`).
- Rồi `search_strings` / `decompile_function` / `list_xrefs` hoạt động.

---

## 2. RE binary PACKED: dump memory lúc runtime (defeat packer)

Nguyên lý: packer **bắt buộc tự giải mã vào RAM để chạy**. Bật process → dump `/proc/PID/mem` →
code+string đã giải mã.

### 2a. Bật process (center cần MySQL — xem `tools/cluster.sh`)
```
cd /root/jx3 && ./cluster.sh up          # mysql + center + gateway + gameserver
# hoặc chỉ mysql + center để dump center
```

### 2b. Dump vùng code đã giải mã (chạy TRÊN HOST, process đang chạy)
```
HPID=$(pgrep -x SO3GameCenter | head -1)
python3 tools/dump_packed_process.py $HPID 0x08048000 0x088ca000 /tmp/center_dump.bin
```
Range lấy từ `readelf -l` (từ đầu LOAD tới hết segment RWE). Host root đọc được `/proc/PID/mem`
của process trong container.

### 2c. Bọc thành ELF để Ghidra nạp đúng base
```
python3 tools/wrap_dump_elf.py            # -> center_decrypted.elf, base 0x08048000
```
Absolute code-refs (x86 position-dependent) chỉ resolve khi load đúng base 0x08048000.
`file` báo "corrupted program header" (ELF tối giản) — kệ, Ghidra vẫn nạp (16195 hàm).

### 2d. Decompile
```
python3 tools/re_center_dump.py           # tìm hàm qua __PRETTY_FUNCTION__ string, decompile
```
Verify: strings đã giải mã hiện plaintext (`KGameServer::ProcessNetwork`, `at line %d in`...).

---

## 3. Khôi phục symbol từ `__PRETTY_FUNCTION__`

Macro `KGLOG_PROCESS_ERROR(...)` nhét full signature `RetType Class::Method(args)` vào mỗi hàm.
→ trích tất cả → có tên hàm thật cho phần lớn `FUN_xxxx`:
```
strings -a center_decrypted.elf | grep -aE '\b[A-Za-z_]\w*::[A-Za-z_]\w*\s*\('
```
Dùng để: (a) rename FUN_xxxx trong Ghidra, (b) so tập hàm với source (mục 4).

---

## 4. Đối chiếu ĐỊNH LƯỢNG binary vs source (đo drift 2010→2.5.2)

**Mức 1 — diff danh sách hàm (nhanh):** trích `Class::method` từ binary (mục 3) và từ source
(`grep 'Class::method(' *.cpp`, nhớ `\r`→`\n` vì source dùng CR line-ending) → so tập:
common / binary-only (thêm ở 2.5.2) / source-only (bỏ). Xem `tools/diff_bin_vs_source.py`.

**Mức 2 — so từng hàm ở mức code:** decompile hàm trong binary + đọc source 2010 cạnh nhau. VD
`KGameServer::OnHandshakeRequest`: phát hiện 2.5.2 thêm field `nResourceVersion@10`, check
`MapGroup@14`. Lưu ý: decompiled-C ≠ source-C → so NGỮ NGHĨA, không phải text.

**Mức 3 — bulk export:** rename toàn bộ theo signature → export decompiled C cả module → cây C
đọc được, diff với source.

---

## 5. Đo runtime không cần sửa binary (khi binary packed/stripped)

- **tcpdump trong netns** (`nsenter -t <pid> -n tcpdump -i lo -s0 -Z root -w cap.pcap 'tcp port N'`)
  — đọc framing/handshake trên dây. QUAN TRỌNG: `-Z root` (nếu không, tcpdump drop-priv → file rỗng).
- **so log 2 binary**: chạy binary thật vs bản build, gom log (rotate 65536 dòng/file), lọc noise.
- Server daemonize → giữ container sống (`... & sleep 900`) mới bắt được log/port.

---

## 6. Bài học

- Binary packed KHÔNG phải hộp đen tuyệt đối — runtime memory dump là chìa khóa.
- `:736` kiểu KGLOG là số DÒNG (macro `__LINE__`, hex như 0x2e0) trong hàm được nêu ở
  `__PRETTY_FUNCTION__` — dùng để định vị chính xác chỗ fail.
- `KGLOG_PROCESS_ERROR(nRetCode) at line 736 in ProcessNetwork` = TRIỆU CHỨNG (socket poll trả 0),
  reject THẬT ở hàm khác — phải grep TẤT CẢ dòng KGLOG quanh thời điểm, không chỉ dòng đầu thấy.
