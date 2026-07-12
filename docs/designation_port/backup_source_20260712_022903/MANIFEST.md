# Backup MANIFEST (20260712_022903)
Files a KDesignation drift-port will touch (copied read-only, pre-port):
- src/SO3World/Src/KDesignation.h
- src/SO3World/Src/KDesignation.cpp
- src/SO3World/Src/KDesignationList.cpp
- include/Include/SO3World/KDesignationList.h
- include/Include/KRoleDBDataDef.h (rbtDesignationData — NOT touched, already present)

NOTE: KLuaPlayer.cpp, KPlayer.cpp, KPlayerServer.{h,cpp} also edited on real port; back up at port time (large files).

## Added at port-time (slice 5+):
- include/Include/SO3Result.h (ectDesignationNotifyCode=31)
- src/SO3World/Src/KPlayer.cpp (m_bDisplayFlag rename + Activate wire)
- src/SO3World/Src/KLuaPlayer.cpp (Lua bindings)
- src/SO3World/Src/KPlayerServer.cpp (m_bDisplayFlag rename)
- src/SO3World/Src/KPlayerClient.cpp (m_bDisplayFlag rename)
