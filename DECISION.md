
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
