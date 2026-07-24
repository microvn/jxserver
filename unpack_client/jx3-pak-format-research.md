# Nghiên cứu định dạng PAK của JX3 (剑网3) — Unpack & Pack

> Tổng hợp từ các bài phân tích ngược và mã nguồn công khai. Chỉ dùng cho mục đích học tập và nghiên cứu kỹ thuật.
> Ngày tổng hợp: 2026-07-04.

## Câu hỏi xuất phát

"Đọc được bài phân tích format PAK và viết được code unpack, thì có viết được code pack ngược lại không?"

**Trả lời dứt khoát: Được — và đã có người làm, có mã nguồn đầy đủ** cho định dạng PACK cổ điển (V1). Các đời mới (V3/V4/V5) hiện chỉ có tool unpack công khai.

---

## Bốn nguồn và bốn triết lý tiếp cận

| Nguồn | Ngôn ngữ | Triết lý | Đóng góp chính |
|-------|----------|----------|----------------|
| Bài feixuwu (cppblog, 2010) + bản kanxue | — | Hiểu format rồi tự viết parser (bỏ cuộc ở khâu nén) | Layout header + index V1, phát hiện `g_FileNameHash` export, offset hàm giải nén `+0x18020` |
| `jx3pak/PakV4-Extract`, `PakV5-Extract` | C++ | Nạp DLL game, gọi hàm export — để engine tự làm | Interface `IFile`, chuỗi tiến hóa V1→V5, cách init từng đời |
| `whc2001/PakV3` | C# | Parse thuần, giải nén bằng lib công khai | Nén V3 = **LZO1X**, format V3 (multi-DAT + Trunk index) |
| `jx3fans/unpackv3` | Go | Parse thuần, phân tích bằng WinHex | **Hash V3 (đã verify bằng test vector)**, struct V3 chi tiết, stride DAT `0x32000000` |
| `SmallBig/KG_XPack` (zhupf) | C++ | **Packer + unpacker hoàn chỉnh** | Hash tên file bản C, nén = **UCL/NRV2B**, manifest tên file, cơ chế fragment |
| QuickBMS `jx3online*.bms` (aluigi) | BMS | Script khai báo format | **Xác nhận độc lập V1 = NRV2B**, biến thể header mã hóa `BMRX`, cờ mới `0x40/0x80`, bản mobile |

---

## Định dạng PACK cổ điển (V1)

### package.ini
Quản lý các file `.pak`, tối đa 32 file (key `0`–`0x20`). **Index càng nhỏ, ưu tiên càng cao** — file trùng thì lấy bản ở pak có index nhỏ hơn.

```ini
[SO3Client]
0=update_1.pak
1=ui.pak
2=settings.pak
...
Path=.\pak
```

### Header (32 byte) — từ `XPack.h` của KG_XPack
```c
struct XPackFileHeader {
    unsigned char cSignature[4];     // 'PACK'  (0x4b434150)
    unsigned long uCount;            // số lượng file
    unsigned long uIndexTableOffset; // offset bảng index (ở cuối file)
    unsigned long uDataOffset;       // offset vùng dữ liệu
    unsigned long uCrc32;            // checksum
    unsigned int  uPakTime;          // thời điểm đóng gói, time()
    unsigned char cReserved[8];      // dành riêng
};
```
> Bài kanxue mô tả header 16 byte đầu: `[PACK] + [số file] + [offset index] + 0x20000000`.
> `0x20000000` thực ra là byte cao của trường tiếp theo (method flag của entry đầu / dữ liệu align), không phải hằng số ma thuật riêng.

### Index entry (16 byte)
```c
struct XPackIndexInfo {
    unsigned long uId;              // hash tên file
    unsigned long uOffset;          // offset dữ liệu trong pak
    long          lSize;            // kích thước gốc (sau giải nén)
    long          lCompressSizeFlag;// 4 bit cao = method; 28 bit thấp = kích thước nén
};
```
**Bảng index phải sort tăng dần theo `uId`** — vì tra cứu dùng binary search. Sai thứ tự → engine không tìm ra file.

### Cờ nén (method) — giải mã "ba giá trị" của kanxue
```c
enum XPACK_METHOD {
    TYPE_NONE  = 0x00000000,  // không nén
    TYPE_FRAME = 0x10000000,  // nén PHÂN MẢNH (fragment): spr nhiều frame, hoặc file > 2MB
    TYPE_UCL   = 0x20000000,  // nén UCL / NRV2B
    TYPE_FILTER= 0xf0000000,  // mask lấy 4 bit cao
};
```
- kanxue đoán `0x10` là "định dạng lạ không cần giải nén" → **sai**. Thực ra là **nén phân mảnh**: file bị cắt khối 2MB, mỗi khối nén NRV2B riêng.
- Đây chính là thứ `IsPackedByFragment()` / `GetFragmentCount()` trong interface `IFile` ám chỉ.

### Cấu trúc fragment
```c
struct XPackFileFragmentElemHeader {
    int nNumFragment;         // số khối
    int nFragmentInfoOffest;  // offset bảng thông tin khối
};
struct XPackFileFragmentInfo {
    unsigned int uOffset;          // offset khối
    unsigned int uSize;            // kích thước gốc khối
    unsigned int uCompressSizeFlag;// kích thước nén + cờ method (như index)
};
```
Ngưỡng: file ≥ 2MB (`COMMON_FILE_SPLIT_SIZE`) chia khối 2MB; SPR ≥ 128KB nhiều frame thì mỗi frame là một khối.

---

## Thuật toán hash tên file (`g_FileNameHash`)

**Quan trọng: V1 và V3 dùng hai thuật toán hash KHÁC NHAU.**

### V3 hash — polynomial base `0x83`, 64-bit (từ `jx3fans/unpackv3`, đã verify)
```go
func FileNameHash(fileName string) uint64 {
    hash := uint64(0)
    if fileName == "" { return hash }
    fileName = strings.ToLower(fileName)              // về chữ thường
    fileName = strings.Replace(fileName, `/`, `\`, -1) // chuẩn hóa dấu gạch
    fileName = strings.Replace(fileName, `\\`, `\`, -1)// gộp gạch đôi
    if strings.HasPrefix(fileName, `\`) {
        fileName = fileName[1:]                        // bỏ gạch đầu
    }
    for _, v := range fileName {                       // duyệt theo RUNE (unicode), không phải byte
        hash = uint64(v) + uint64(0x83)*hash           // hash = c + 0x83*hash
    }
    return hash
}
```
Đây là **rolling hash đa thức cơ số 0x83 (131), 64-bit** — đơn giản hơn nhiều so với V1. Đã kiểm chứng khớp 100% với test vector của tác giả:

| Đường dẫn | Hash (verify) |
|-----------|---------------|
| `scripts\achievement\ItemAcquire_Achievement.lua` | `3A3064375F4C9628` ✅ |
| `scripts\Activity\6月16日运营活动\item\加成道具.lua` | `37853E8D9C4E4A3E` ✅ |

Điểm tinh tế: hash **theo rune (unicode code point)**, nên đường dẫn tiếng Trung/tiếng Việt phải xử lý đúng encoding. Bản Python verify (đã chạy khớp):
```python
def h(s):
    s = s.lower().replace('/', '\\').replace('\\\\', '\\')
    if s.startswith('\\'): s = s[1:]
    x = 0
    for ch in s:
        x = (ord(ch) + 0x83 * x) & 0xFFFFFFFFFFFFFFFF
    return x
```

### V1 hash — prime `0x8000000b` + XOR (từ `KG_XPack/main.cpp`)
```c
unsigned long hash(const char *file_name) {
    unsigned long id = 0;
    const char *ptr = file_name;
    int index = 0;
    while (*ptr) {
        if (*ptr >= 'A' && *ptr <= 'Z')
            id = (id + (++index) * (*ptr + 'a' - 'A')) % 0x8000000b * 0xffffffef;
        else
            id = (id + (++index) * (*ptr)) % 0x8000000b * 0xffffffef;
        ptr++;
    }
    return (id ^ 0x12345678);
}
```
- **Không phân biệt hoa/thường** (fold chữ hoa về thường).
- Cộng dồn có trọng số theo vị trí (`++index`), modulo số nguyên tố `0x8000000b`, nhân `0xffffffef`, XOR `0x12345678`.
- Lưu ý (theo kanxue): đường dẫn truyền vào có thể cần bắt đầu bằng `\`. KG_XPack chuẩn hóa: cwd + `\` + đường dẫn tương đối, chuyển toàn bộ về chữ thường, rồi hash phần sau `root_length`.

---

## Nén

| Đời | Thuật toán | Hàm nén | Hàm giải nén |
|-----|-----------|---------|--------------|
| V1 (PACK cổ điển) | UCL / NRV2B | `ucl_nrv2b_99_compress(..., level=5, ...)` | `ucl_nrv2b_decompress_8` |
| V3 | LZO1X | `lzo1x_1_compress` (chưa dùng trong repo, nhưng có sẵn) | `lzo1x_decompress` |

Cả UCL lẫn LZO đều của **Markus Oberhumer**, mã nguồn mở, **đối xứng** (có sẵn cả nén và giải nén). Đây là lý do khâu nén — tưởng là rào cản lớn nhất — thực ra giải quyết được mà không cần DLL của game.

Trong game, hàm giải nén nằm trong `Engine_Lua5.dll` tại **base + 0x18020**:
```c
typedef int (*PUNPACK_FUN)(void* pSrc, int nSrcLen, void* pDst, int* pDstLen);
```

---

## Khôi phục tên file thật

Bài toán mà feixuwu và whc2001 đều bó tay. Lời giải của KG_XPack: **danh sách tên file được nhúng ngay trong pak**, là một file nội dung bắt đầu bằng chuỗi `TotalFile:`.

- **Unpack**: giải file `TotalFile:` ra, đọc cột tên (tab thứ 3 mỗi dòng), tính hash, đổi tên các file `_-ID-_%08x.txt` về tên thật.
- **Pack**: nếu tên file dạng `_-id-_%08x.txt` thì dùng thẳng id; ngược lại tính hash từ đường dẫn.

Với đời mới không có manifest nhúng: dùng **FileMon** bắt tên file lúc game chạy, hoặc hook `g_OpenFileInPak` / `luaL_loadBuffer` (gợi ý của feixuwu và bạn đọc yafare).

---

## Chuỗi tiến hóa format (từ jx3pak/PakV4-Extract)

| Đời | Nhận diện | Hàm init (export trong DLL) |
|-----|-----------|------------------------------|
| Pak (V1) | `package.ini` `[SO3Client]` | `g_LoadPackageFiles` |
| PakV2 / NewPak (server VN từng dùng) | `NewPackageList.txt` | `KGPack_LoadPackageFiles` |
| PakV3 | `Trunk.Dir` | `KGInitPackFileV3` |
| PakV4 | `PakV4/Trunk.Dir` | `KG_InitPakV4FileSystem` |
| PakV5 (2024, 无界/all-platform) | `configHttpFile.ini` | `g_InitHttpFile` (trong `KGPK5_FileSystemX64.dll`) |

PakV5 là filesystem qua **HTTP/CDN** (`xoyocdn.com`) — "giải nén" thực chất là engine tải về rồi đọc.

### Interface IFile (vtable, từ jx3pak)
```cpp
class IFile {
    virtual unsigned long Read(void*, unsigned long) = 0;
    virtual unsigned long Write(const void*, unsigned long) = 0;
    virtual void*         GetBuffer() = 0;
    virtual long          Seek(long, int) = 0;
    virtual long          Tell() = 0;
    virtual unsigned long Size() = 0;
    virtual int           IsFileInPak() = 0;
    virtual int           IsPackedByFragment() = 0;
    virtual int           GetFragmentCount() = 0;
    virtual unsigned int  GetFragmentSize(int) = 0;
    virtual unsigned long ReadFragment(int, void*&) = 0;
    virtual void          Close() = 0;
    virtual void          Release() = 0;
};
```

---

## Định dạng PakV3 (cross-validate từ whc2001/PakV3 C# + jx3fans/unpackv3 Go)

Khác hẳn V1 — địa chỉ 64-bit, tách index/data/config, ghép nhiều file `.DAT`. Hai implement độc lập khớp nhau về layout, bản Go cho tên trường và hằng số chính xác hơn.

### Ba loại file trong thư mục `PakV3\`
- `Trunk.DIR` — bảng index (header + entries)
- `Package.CFG` — cấu hình: số lượng pak, kích thước
- `Package{N}.DAT` — dữ liệu (N = 0,1,2,...)

### Header của Trunk.DIR (đúng 0x200 byte)
```go
type Head struct {
    Sign    uint64      // signature (offset 0, 8 byte)
    Tmp     uint32      // version   (offset 8, 4 byte)
    Num     int32       // SỐ FILE   (offset 0x0C, 4 byte)  ← whc2001 gọi nhầm là "TotalLength"
    Trunk   [0x80]byte  // 128 byte
    CfgName [0x170]byte // 368 byte, tên file config
}   // tổng = 8+4+4+128+368 = 512 = 0x200
```
> whc2001 đọc `Num` ở `0x0C` và gọi là TotalLength — thực ra là **số lượng file**. Entries bắt đầu ở `0x200`.

### Index entry (20 byte, bắt đầu ở 0x200)
```go
type Item struct {
    Hash    uint64 // hash tên file (little-endian)
    Encpos  int64  // offset trong không gian ảo
    Encsize int32  // độ dài khối trong pak
}
```
> whc2001 đọc `Hash` dạng big-endian (`.Reverse()`) — nhưng repo đó không tính hash để so khớp nên endian không ảnh hưởng kết quả của họ. Bản Go so khớp trực tiếp với `FileNameHash` nên **little-endian là đúng**. Vẫn phải sort tăng dần theo Hash để binary search.

### Định vị dữ liệu qua nhiều DAT — stride cố định `0x32000000` (~800MB)
```go
i := item.Encpos / 0x32000000   // Package{i}.DAT
p := item.Encpos % 0x32000000   // offset trong file đó
```
> Chính xác hơn cách "cộng dồn size" của whc2001. Thiết kế thật: mỗi `.DAT` là một slot kích thước tối đa cố định 0x32000000 byte.

### Header block mỗi file (data bắt đầu ở 0x38 = 56 byte)
```go
type FileInfo struct {
    Tmp1    int64  // offset 0
    Tmp2    int32  // offset 8
    Srcsize int32  // offset 0x0C — kích thước gốc     (whc2001: DecompressedLength)
    Tmp4    int32  // offset 0x10
    Pos     int64  // offset 0x14
    Encsize int32  // offset 0x1C — kích thước nén      (whc2001: RawLength)
    Tmp7    int32  // offset 0x20
    Enc     int32  // offset 0x24 — cờ nén: 1 = LZO1X   (whc2001: CompressType)
}   // data bắt đầu ở 0x38
```
Cross-validated: cả hai impl độc lập đều chốt `Srcsize@0x0C`, `Encsize@0x1C`, `Enc@0x24`, data@`0x38`.

### Nén V3 = LZO1X (xác nhận bởi cả hai impl)
- whc2001: `lzo1x_decompress` (C, `lzo.dll`)
- unpackv3: `lzo.Decompress1X` (Go, `github.com/rasky/go-lzo`)
- `Enc == 1` → LZO1X; `Enc == 0` → không nén (đọc thẳng từ `0x38`).

> ⚠️ **Cảnh báo quan trọng (từ readme unpackv3):** *"Pakv3 后期还进行过升级内置了 N 种压缩算法，所以即使是 pakv3 也不一定能全解"* — PakV3 đời sau **nhúng thêm nhiều thuật toán nén khác**, nên ngay cả file PakV3 cũng không chắc giải hết được. Các giá trị `Enc` khác 0/1 = thuật toán khác, chưa xử lý.

### Ghi chú vận hành (unpackv3)
- Chỉ hỗ trợ **test server** (`_exp`), 64-bit. Tự đọc registry `SOFTWARE\Wow6432Node\kingsoft\JX3\zhcn_exp` → `installPath`.
- Cần file danh sách `ls.txt` (UTF-8) chứa đường dẫn nội bộ — vì V3 không nhúng manifest như V1. Có hash rồi thì hash bất kỳ đường dẫn đoán được để tra.

---

## Binary server đã build (镜像端, JX3 2012-12-14) — recover primitive + xác nhận đầu-cuối

Image server Linux đã giải nén tại `镜像端/extracted/root/` (version `2-5-2-4503`, build 14/12/2012). `libEngine_Lua5.so` là bản Linux của `Engine_Lua5.dll` — **ELF 32-bit, KHÔNG strip**. Đây là nơi chứa hai primitive mà source leak thiếu. `nm` xác nhận export:

```
g_FileNameHash, KG_FileNameHash        # hàm hash (feixuwu 2010 nói "được export" -> đúng)
CD_LCU_C / CD_LCU_D / CD_LCU_I         # wrapper nén/giải/init
ucl_nrv2b_99_compress, __ucl_init2,    # thư viện UCL tĩnh -> xác nhận nén = UCL/NRV2B
  _ucl_crc32_table, ucl_copyright
lzo1x_decompress                       # cũng có -> xác nhận V3 = LZO1X
g_OpenFile, g_OpenFileInPak            # điểm hook lấy tên file (gợi ý feixuwu)
KGInitPackFileV3, KGUninitPackFileV3   # khớp export name của jx3pak extractor
```

`objdump` `CD_LCU_C` → gọi thẳng `ucl_nrv2b_99_compress` (đúng hàm KG_XPack dùng). Chốt: nén V1 = **UCL NRV2B**, không còn suy đoán.

### g_FileNameHash disassembly — v1_hash verify BYTE-EXACT
Dịch ngược hàm tại `0x1b910`:
```asm
eax = 0x12345678                       ; XOR cuối
mỗi ký tự c (index bắt đầu 1):
  'A'<=c<='Z':  c += 0x20              ; hoa -> thường
  c == '/' (0x2f): c = '\' (0x5c)      ; chuẩn hóa dấu gạch  ← chi tiết KG_XPack thiếu
  eax = c * (++index)
  edx += eax
  edx = edx % 0x8000000b               ; 1 phép trừ có điều kiện
  edx = edx * 0xffffffef               ; (-0x11)
return edx ^ 0x12345678
```
Khớp `v1_hash` 100% (`0x8000000b`, `0xffffffef`, `^0x12345678`) — chỉ bổ sung chuẩn hóa `/`→`\`. Verify lại trên pak thật: **268/268** với path **giữ nguyên `\` dẫn đầu** (`\script\...`); bỏ `\` đầu thì hỏng → hash tính trên path tương đối *kèm* separator đầu. Đã cập nhật cả hai script + prepend `\` khi hash từ cây thư mục.

**Ý nghĩa**: image binary này nối source leak (không chạy được, thiếu 2 primitive) với một server **chạy thật** — và recover đúng hai primitive đó, xác nhận toàn bộ chuỗi reverse hội tụ vào chính engine gốc. Ngoài ra `extracted/root/` còn có toàn bộ tài nguyên đã giải (không đóng pak): `scripts/` (188M Lua), `maps/` (3.8G), `recorder/` (874M — dữ liệu record protocol thật), `pak_stat` — tức server chạy bằng file rời, không cần pak.

---

## Source gốc Kingsoft (JX3-AIO leak, 2008→2012) — xác nhận 1:1

Tìm thấy trong `JX3-AIO/Source/Tools/ExtractPack/.../KPackFileManager.cpp` — **class quản lý pack gốc của Kingsoft** (Author Wooy, mốc 2005). Đây là implementation đứng sau mọi thứ đã reverse. Nó xác nhận từng quyết định trong `jx3-pack-v1.py`:

- **Hash**: `uHashId = g_FileNameHash(path + m_nElemFileRootPathNotEnderLen)` — hash đường dẫn tương đối so với root (bỏ root, không tính `\` cuối). Có check trùng id giữa các pak.
- **Nén**: `CD_LCU_C(src, srcSize, dst, &destSize, 5)` = UCL nén **level 5** (`CD_LCU` = "UCL" đảo chữ). Chỉ giữ bản nén nếu `nSrcSize > uDestSize`, ngược lại `XPACK_METHOD_NONE` — đúng logic `if len(comp) < len(raw)` của packer ta.
- **Method/fragment**: hằng `XPACK_METHOD_NONE / XPACK_METHOD_UCL / XPACK_FLAG_FRAGMENT`; file lớn/SPR → `AddElemToPakFragment`, mỗi khối `uCompressSizeFlag = size | type`. Struct `XPackFileHeader`/`XPackIndexInfo` cùng tên.
- **API packer đầy đủ**: `CreatePack / AddFolderToPak / AddElemToPak / DeleteElemInPak / AddElemToPakFragment / AddElemToPakCommon / AddElemToPakFragmentSPR / GenerateElemIndexAndHashId`.

**Không có trong leak**: thân hàm `g_FileNameHash` và `CD_LCU_C` — include qua `Precompile.h`, biên dịch sẵn trong `.lib` (`Lib/Win32/`). `0x8000000b` không xuất hiện trong source. Nhưng cả hai primitive này ta đã có đúng và verify độc lập (hash: KG_XPack 268/268; UCL/NRV2B: tự viết 3112/3112). Leak lấp nốt tầng giữa (logic quản lý) và chứng minh kiến trúc packer của ta khớp bản gốc Kingsoft.

---

## QuickBMS scripts (aluigi) — xác nhận độc lập + biến thể

Ba script BMS công khai, là nguồn thứ tư độc lập. `jx3online.bms` xác nhận từng chi tiết format V1 (nguồn thứ 3 chốt cùng NRV2B/PACK/16-byte index/cờ fragment — độ tin cậy gần tuyệt đối).

### jx3online.bms (script 0.1.1) = format V1 PACK
```bms
comtype NRV2b                    # <- xác nhận nén = UCL/NRV2B
getdstring SIGN 4
if SIGN == "BMRX"                # biến thể header MÃ HÓA (một số server)
    encryption xor "678iujhygrehFG$WE%*&..."
    log MEMORY_FILE 0 0x20
    filexor 0x62
    ...
else
    idstring MEMORY_FILE "PACK"  # header thường
endif
get FILES long
get INFO_OFF long                # offset bảng index
get BASE_OFF long                # offset dữ liệu
goto INFO_OFF
for i = 0 < FILES                # mỗi entry 16 byte:
    get CRC long                 #   hash tên file
    get OFFSET long
    get SIZE long                #   kích thước gốc
    get ZSIZE long               #   kích thước nén + cờ (nibble cao)
    if ZSIZE & 0x10000000        #   TYPE_FRAME -> phân mảnh (đọc CHUNKS)
    elif ZSIZE & 0x20000000      #   TYPE_UCL   -> clog NRV2B
    elif ZSIZE & 0x40000000      #   "not supported" (loại mới)
    elif ZSIZE & 0x80000000      #   "not supported" (loại mới)
    else                         #   không nén
    endif
next i
```
- Cờ `0x10000000`/`0x20000000` khớp chính xác `TYPE_FRAME`/`TYPE_UCL` của KG_XPack. Hàm `unpack_0x10000000` đọc `CHUNKS` + `CHUNKS_OFFSET` rồi từng chunk `OFFSETX/SIZEX/ZSIZEX` — trùng struct `XPackFileFragmentElemHeader`/`XPackFileFragmentInfo`.
- Header aliases trong script: *Trường Kiếm, KiemThe, wjx2, njx, jxonline3, 麻辣江湖* — engine PACK V1 dùng chung cho cả dòng KingSoft, **gồm server Việt Nam** (Kiếm Thế / Trường Kiếm).

**Điểm mới so với các nguồn khác:**
1. **Biến thể `BMRX`**: header bị XOR bằng key cố định + `filexor 0x62`. Format V1 nhưng có lớp che.
2. **Cờ `0x40000000` / `0x80000000`**: tồn tại nhưng script không hỗ trợ → V1 về sau thêm loại nén/mã hóa mới ngoài NONE/FRAME/UCL.

### jx3online_new.bms (0.1.2) = format đời sau (V3/V4)
- `comtype lzo1x` → xác nhận LZO1X (khớp whc2001 + unpackv3).
- **Heuristic hoàn toàn**: không index sạch, padding byte `0xce` giữa các file, offset 40-bit trải trên nhiều `package.dat` ("1500 package.dat"). Entry: `0xc dummy + SIZE + ENTRY_OFF + ZSIZE(longlong) + ZIP(long) + 0x10 dummy`; `ZIP==0` không nén, `ZIP==1` LZO.

### jx3mobile.bms (0.1.2) = bản mobile (MỚI)
- Tách `.idx` (index) / `.CP` (container nén) / `.dat` / `.patch`; `comtype lzo1x`.
- `filexor` bằng key positional (bytes `0x00..0xff`).
- **Cây thư mục giữ TÊN FILE THẬT** (không chỉ hash) — khác hẳn PC. `ID >= 50000000` đánh dấu file patch overlay.

---

## Thực nghiệm trên dữ liệu thật (China3, 2010) — ĐÃ VERIFY

Test trên bộ pak thật `China3/pak/` (`scripts.pak`, `settings.pak`, `represent.pak`, đề ngày 2010-05-20 = format V1). Kết quả:

- **Format khớp 100%**: cả ba đều `PACK`, header 32 byte (`data_off=0x20`), index ở cuối. Phân bố method: đa số `UCL (0x20000000)`, một ít `NONE (0x00)`, vài `FRAME (0x10)`.
- **Hàm hash V1 verify 268/268**: `v1_hash(filename)` (thuật toán KG_XPack) tái tạo đúng toàn bộ cột ID trong manifest `script.pak.txt`. Xác nhận `g_FileNameHash` bằng thực nghiệm.
- **Giải nén verify 3112/3112 entry**: viết NRV2B thuần Python (không lib), giải đúng độ dài mọi entry. Nội dung ra Lua/text hợp lệ (header script chuẩn `文件名/作者/创建时间`, code `Include(...)`, bảng `tSkillData={...}`).
- Script hoàn chỉnh: `jx3-unpack-v1.py` (cùng thư mục).

### Đính chính kỹ thuật quan trọng về NRV2B
Khi tự implement `ucl_nrv2b_decompress_8`, hai chỗ dễ sai (tôi đã vấp và sửa):
1. **Vòng giải offset phải là dạng NRV2B đơn giản**, KHÔNG phải dạng interleaved của NRV2D:
   ```python
   m_off = 1
   while True:
       m_off = m_off*2 + getbit()
       if getbit(): break          # NRV2B: chỉ data-bit + flag-bit
   # SAI (đây là NRV2D): thêm dòng m_off = (m_off-1)*2 + getbit()
   ```
   Dùng nhầm dạng NRV2D vẫn giải đúng các entry có offset nhỏ (đánh lừa), chỉ desync khi gặp offset lớn đầu tiên.
2. **getbit dùng đúng macro `bb & 0x7f`** (MSB-first, nạp lại mỗi 8 bit):
   ```python
   def getbit():           # bb, ip là biến ngoài (nonlocal)
       if bb & 0x7f:
           bb = bb * 2
       else:
           bb = src[ip] * 2 + 1; ip += 1
       return (bb >> 8) & 1
   ```
   Tương đương macro C: `((bb = bb & 0x7f ? bb*2 : src[i++]*2+1) >> 8) & 1`.

### Packer V1 — ĐÃ dựng và verify
Viết `jx3-pack-v1.py`: **compressor NRV2B thuần Python** (mirror bit-for-bit format của decoder), + build container PACK (header + data + index sort theo hash).
- **Round-trip 3112/3112 byte-identical**: nén mọi entry game thật → giải lại (bằng decompressor đã verify với game) → khớp tuyệt đối. Vì decompressor đã đúng khớp game, đây là bằng chứng mạnh rằng game engine đọc được pak do packer này tạo.
- **Chu trình pack→unpack đầy đủ**: đóng 240 file thành `.pak` mới, parse lại OK, index sort tăng dần đúng, unpack lại khớp 240/240. Kích thước 1.83MB (nhỏ hơn bản gốc 2.05MB).
- Chi tiết encoder NRV2B cần đúng: (a) bit-writer dùng tag-byte MSB-first nạp mỗi 8 bit — phải khớp `getbit`; (b) offset high-part = mã gamma NRV2B (data-bit + flag-bit, flag=1 kết thúc); (c) min match length = 2 (dist≤0xd00) hoặc 3 (dist>0xd00), vì offset xa "mượn" 1 vào m_len; (d) kết thúc bằng EOF marker `(H-3)*256+0xff == 0xffffffff`; (e) index BẮT BUỘC sort tăng theo id.
- **Chưa test được**: nạp pak vào game engine Windows thật (không có môi trường ở đây). Round-trip qua decompressor-đã-verify là mức xác thực cao nhất đạt được offline.

### Về client 2015 của bạn
Bộ China3 này là **V1 PACK + NRV2B** — chính xác loại mà script QuickBMS `jx3online.bms` xử lý. Rất khớp với việc script chạy được cho client bạn dùng ~2015 (Kiếm Thế / engine cũ / private server), khác với JX3 PC đời mới (đã lên PakV3/V4/V5).

---

## Đánh giá khả năng REPACK

| Mảnh | V1 (PACK cổ điển) | V3/V4/V5 hiện đại |
|------|-------------------|-------------------|
| Layout container | ✅ đầy đủ (`XPack.h`) | ✅ V3 map trọn (2 impl khớp); V4/V5 chưa |
| Thuật toán nén | ✅ UCL/NRV2B, open source, đối xứng | ✅ V3 = LZO1X đối xứng; ⚠️ V3 đời sau có N thuật toán; V4/V5 chưa rõ |
| Hash tên file | ✅ bản C thuần (KG_XPack) | ✅ **V3 = poly base 0x83, đã verify** (unpackv3) |
| Khôi phục tên file | ✅ manifest `TotalFile:` nhúng | ⚠️ V3 không nhúng manifest → cần `ls.txt` / FileMon / hook |
| **Packer công khai** | ✅ **KG_XPack** | ❌ chưa thấy — chỉ có unpack |

**Kết luận cập nhật**: Pack format cổ điển (V1) đã giải quyết trọn vẹn — có packer. Với V3, cả ba mảnh kỹ thuật (layout, nén LZO1X đối xứng, **hash đã verify**) đều đã lộ, nên **về nguyên tắc viết packer V3 là khả thi** — chưa ai công bố nhưng không còn rào cản tri thức nào cho các file nén LZO1X thường. Rào cản còn lại chỉ là: (a) các thuật toán nén "lạ" mà PakV3 đời sau nhúng thêm, và (b) V4/V5 chưa được map. Hướng cho V4/V5 vẫn như cũ: tìm hàm pack/compress export trong DLL editor và gọi, hoặc chờ ai đó phân tích format.

**Bài học RE xuyên suốt**: unpack luôn dễ hơn pack; nhưng cả hai chỉ khó tới mức bạn *chưa biết* mảnh còn thiếu. Với JX3, cộng đồng đã dần lộ hết mảnh cho định dạng cũ (hash, nén, manifest đều công khai). Và một triết lý mạnh: **khi phần mềm gốc còn chạy được, đừng tái tạo nó — hãy điều khiển nó** (nạp DLL, gọi hàm export).

---

## Nguồn

- Bài gốc feixuwu: `http://www.cppblog.com/feixuwu/archive/2010/07/16/120581.aspx` (mirror CSDN: `https://blog.csdn.net/wuxufei/article/details/5769779`)
- KG_XPack (packer + unpacker): https://gitee.com/SmallBig/KG_XPack — mirror: https://gitee.com/yangminglong_a/KG_XPack
- jx3pak/PakV4-Extract: https://github.com/jx3pak/PakV4-Extract
- jx3pak/PakV5-Extract: https://github.com/jx3pak/PakV5-Extract
- whc2001/PakV3 (parser C#, LZO1X): https://github.com/whc2001/PakV3
- jx3fans/unpackv3 (parser Go, hash V3 + struct chi tiết): https://github.com/jx3fans/unpackv3
- QuickBMS scripts jx3online.bms / jx3online_new.bms / jx3mobile.bms (aluigi): mirror https://github.com/PMArkive/quickbms (thư mục `__quickbms_scripts/`)
- Thread gốc XeNTaX "Need help for JX3ONLINE PAK file" (backup): https://github.com/XeNTaXBackup/XeNTaXBackup.github.io (file `Need help for JX3ONLINE PAK file !!_4041.md`)
- Trang QuickBMS gốc: https://aluigi.altervista.org/quickbms.htm
- Simplicit.Net.Lzo (bản sửa của whc2001): https://github.com/whc2001/Simplicit.Net.Lzo
- 新版PAKV3解压工具 (多玩论坛): http://bbs.duowan.com/thread-36574576-1-1.html
