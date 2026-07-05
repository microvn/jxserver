
### Q2. Kiem real dut diem — real SACH, 13 residual la ours-specific
- Real complete log = 56 dong, 1 FILE (real khong flood ]:Get nen khong rotate; ours flood NpcTemplate
  tolerant warning 18724x2 -> rotate 65536 dong/file = nguon nhieu do truoc). Real: index-nil=0,
  GetSkillLevelData-fail=0, Failed-load=25(=ours), 6161 skill sach. => 13 index-nil CONFIRM ours-specific.
- Root-cause 13: tSkillData[skill.dwLevel] nil du m_dwLevel set TRUOC CallInitializeScript (dwLevel dung)
  + tSkillData >= dwLevel entries => tSkillData GLOBAL collision/nil khi GetSkillLevelData chay. NHUNG
  ours CallInitializeScript CAU TRUC Y HET real (SafeCallBegin/PushValueToStack(this)/CallFunction/
  SafeCallEnd = FUN_08112a72/081567ec/081131fe/08112a1c) => khac o tang Lua script-env isolation sau hon,
  can debug Lua-runtime. 4/6161 skill, non-fatal (boot van [OK]).
- KET LUAN: moi drift HE THONG fixed, log khop real toan bo noi dung he thong + Map-fails y het;
  residual 13 dong (4 script) edge-case Lua-scope chua fix (tach biet, can Lua-runtime debug).

### Q3. Root-cause 13 index-nil chinh xac (2026-07-05) — data off-by-one MaxLevel vs tSkillData
- 13 index-nil DETERMINISTIC (tai dung: 10x line35 nam-chieu-hoang-cung, 2x line66 + 1x line72 thien-cong-fang).
- 4 script fail deu: tab MaxLevel > so entry tSkillData script khai. VD `天工坊_3号BOSS_震天甲士_冲击`
  MaxLevel=**21** nhung tSkillData chi **20** entry -> level 21: tSkillData[21]=nil -> index nil (line 66).
  南诏皇宫_南诏皇宫_* (line35) tuong tu. => bat nhat DATA trong 4/6161 script (tac gia khai tSkillData thieu).
- Da loai tru: dwLevel dung (m_dwLevel set truoc CallInitializeScript), tSkillData khong nil (co entries),
  CallInitializeScript CAU TRUC Y HET real (SafeCallBegin/PushValueToStack/CallFunction/SafeCallEnd).
- CHUA pin: vi sao real (cung MaxLevel=21, cung tSkillData=20, cung loop 1..MaxLevel FUN_081648fc) KHONG
  fail. Nghi real doc/cap dwMaxLevel khac (TranslateTableInfo/LoadSkillTableLine) HOAC engine xu level>data
  khac. Runtime-instrument (error() trong script) KHONG fire duoc do harness multi-log (server rotate
  65536 dong/file) + find-newer flaky; can harness tot hon (tat rotate / lua debug hook) de dao tiep.
- Trang thai: non-fatal (boot [OK]), 4 script edge. Moi drift HE THONG da khop real. Residual nay = data
  off-by-one + 1 an so real-MaxLevel-handling, tach biet.
