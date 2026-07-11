# Backup MANIFEST — exterior port

## [PORT-1] 2026-07-08 10:43:41 +0700 — enums + leaf structs
- Lat #1 CHI THEM file moi: src/SO3World/Src/KExteriorDef.h (enums + KEXTERIOR_INFO/ITEM + *_DB_DATA).
- KHONG sua file goc nao trong lat nay -> khong co ban backup file goc o buoc nay.
- Cac lat sau (khi cham KPlayer.h/.cpp, KLuaCharacter.cpp...) se cp -Rp vao day truoc khi sua.

## [PORT-2] slice #2 — KExterior singleton + LoadTable
- NEW: src/SO3World/Src/KExterior.h, KExterior.cpp.
- MODIFIED (backed up above): include/Include/SO3World/KWorldSettings.h (add member + include), src/SO3World/Src/KWorldSettings.cpp (Init/UnInit wiring).

## [PORT-5] slice #5 — embed KExteriorBox in KPlayer + apply
- MODIFIED (backed up): src/SO3World/Src/KPlayer.h, KPlayer.cpp (embed m_ExteriorBox + m_dwApplyExteriorFlag + Init + ApplyExteriorRepresent hook).
- [PORT-6] MODIFIED include/Include/KRoleDBDataDef.h (+rbtExteriorData)
- [PORT-7] MODIFIED GS_Client_Protocol.h (+2 s2c proto +2 structs), KPlayerServer.h/.cpp (+2 DoSync methods)
- [PORT-8] MODIFIED KLuaPlayer.cpp (+9 exterior Lua bindings + register), KExterior.h/.cpp (+AddExterior grant), KExteriorBox.h/.cpp (+AddExteriorSet), KExteriorDef.h (+buy enums)
