# Backup 20260712_010655 — hair port (S2-S5 wire)
Files copied BEFORE editing for KHairShop/KHairBox integration:
- SO3World/Src/KPlayer.h, KPlayer.cpp  (embed m_HairBox + Save/Load dispatch rbtHairBoxData)
- SO3World/Src/KSO3World.h, KSO3World.cpp  (add m_HairShop singleton + Init)
- Include/Include/KRoleDBDataDef.h  (append rbtHairBoxData before rbtTotal)
New files (no backup needed): KHairShopDef.h, KHairShop.h/.cpp, KHairBox.h/.cpp
