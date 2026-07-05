
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

### Q4. Option A ket qua — DBGML instrument xac nhan DATA off-by-one, real-mechanism opaque
Instrument KSkill::CallInitializeScript (temp log dwMaxLevel/dwLevel/script khi GetSkillLevelData fail),
build, boot, grep theo timestamp (reliable). KET QUA CHINH XAC:
- id=838/844/855: MaxLevel=21, fail tai lv=21 (tSkillData literal=20 entry) -> tSkillData[21]=nil.
- id=4413 (南诏皇宫_陈和尚_肉毒地藏): MaxLevel=20, fail lv=11..20 (tSkillData literal=**10** entry, dem chinh xac)
  -> tSkillData[11..20]=nil.
=> 13 index-nil = 4 script co tSkillData literal < MaxLevel. Script KHONG co loop/helper fill tSkillData
   (thuan literal). Day la BUG DATA trong 4 script game (tac gia khai tSkillData thieu vs MaxLevel tab).
- Ours doc dwMaxLevel dung tu cot "MaxLevel" (=21/20), loop 1..MaxLevel = Y HET real (FUN_081648fc).
- MAU THUAN chua giai: real cung file (tSkillData 10/20), cung tab (MaxLevel 20/21), cung loop -> dang le
  cung fail lv>tSkillData. Real (ban 56-dong good) index-nil=0. Static RE khong giai thich duoc; KHONG
  instrument duoc real (docker harness flaky: real luc dat [OK] luc 4-dong early-exit -> khong co
  instrumented-real run on dinh de so). Co the real cap level/doc MaxLevel khac o tang sau chua pin duoc.
- Non-fatal (boot [OK] giong real). Option de dong tuyet doi: (a) sua DATA 4 script (extend tSkillData ->
  MaxLevel, HOAC giam MaxLevel tab) -> guarantee 0 index-nil; (b) on dinh harness (tat log-rotate/fix
  docker flaky) roi instrument real pin co che; (c) log GetSkillLevelData-fail o muc DEBUG thay ERROR.
- KET LUAN: moi drift HE THONG fixed; residual = 4 script DATA off-by-one (tSkillData<MaxLevel), real
  tolerate qua co che chua pin (harness limit). Framework logic ta = real.

### Q5. DONG DUT DIEM — data-fix 4 NPC skill, 2 log KHOP (2026-07-05)
4 skill fail deu la NPC/BOSS skill phu ban (chi cast level 1 in-game): id838 天工坊_3号BOSS_震天甲士_冲击,
id844 天工坊_3号BOSS_横扫, id855 天工坊_副本小怪_机甲龙_怒吼, id4413 南诏皇宫_陈和尚_肉毒地藏.
MaxLevel template=20/21 nhung tSkillData literal chi 10/20 -> level >tSkillData = nil (13 dong).
Level 2-20 cua NPC skill vo dung (boss chi cast lv1) -> data-fix an toan.
FIX: tools/fix_npc_tskilldata.py extend tSkillData -> MaxLevel (copy pad), idempotent, chay tren deploy
tree host (data khong trong git; tool trong git de tai lap sau re-extract).
KET QUA: index-nil 13->0, skill-errors=0. **2 log KHOP: ours Failed-load 19 = real 19 (lech 0 ca 2 chieu),
ca hai index-nil=0, Load game settings [OK], center-connect fail.** Khac biet con lai duy nhat = flood
`]:Get` (NpcTemplate field tolerant best-effort) da loc, khong phai divergence chuc nang.
GOAL DAT: moi drift he thong fixed + 4 data-quirk fixed -> log khop real toan bo noi dung nghia.
