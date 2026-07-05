# Drift measurement 2010 source vs 2.5.2 binary (function-set diff, 2026-07-06)

## GameServer (SO3GameServer, NOT packed)
binary: /Volumes/ExData/game/jx3/镜像端/extracted/root/SO3GameServer
  distinct Class::method = 3413  classes = 216
source: 210 files; distinct Class::method = 4190

=== DRIFT ===
common               : 2005
binary-only (2.5.2 +) : 1408
source-only (removed) : 2185
coverage: 58.7% of binary funcs exist in 2010 source

=== classes with most NEW methods in 2.5.2 ===
  KPlayer                          +282
  KScriptFuncList                  +190
  KPlayerServer                    +147
  KRelayClient                     +117
  KItemList                        +43
  KLogClient                       +40
  KScene                           +37
  KCharacter                       +34
  KExterior                        +29
  KExteriorBox                     +28
  KDomesticate                     +27
  KDiamondManager                  +23
  KNpc                             +20
  KHairShop                        +18
  KDesignation                     +14
  KSkillManager                    +13
  KSkill                           +13
  KTongServer                      +12
  KFellowPetBox                    +12
  KDropCenter                      +11
