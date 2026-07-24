// CRC32 shim (endgame link) — DECISION.md §A7,§E.
// Engine .so (libEngine_Lua5D.so) export "Misc_CRC32" (extern "C"); source đời này gọi
// "CRC32" (cùng signature, chỉ khác tên — engine ship sau đổi tên). Định nghĩa CRC32 = forward
// tới Misc_CRC32 -> dùng ĐÚNG impl trong .so (không tự viết lại, không lệch thuật toán/kết quả).
extern "C" {
    unsigned Misc_CRC32(unsigned nCRC, const void* pvBuf, unsigned uLen);  // định nghĩa trong .so
    unsigned CRC32(unsigned nCRC, const void* pvBuf, unsigned uLen);       // cái source gọi
}

unsigned CRC32(unsigned nCRC, const void* pvBuf, unsigned uLen)
{
    return Misc_CRC32(nCRC, pvBuf, uLen);
}
