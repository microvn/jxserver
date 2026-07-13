# KShop currency-buy (mua đồ trả currency) — port WORKLOG

DRIFT (KShop/KShopCenter exist in 2010). Un-blocks the buy side of KCurrency. RE 2026-07-13.

## [RE-1] Shop-item drift (KNPC_SHOP_TEMPLATE_ITEM: 2010 ~14 fields -> v2.5 0x64=25 fields)
2010 already had nPrice/nContribution/nPrestige prices. v2.5 INSERTED (DWARF + loader 08139cc0):
nCoin@0x1c (yuanbao), nJustice@0x28, nExamPrint@0x2c, nArenaAward@0x30, nActivityAward@0x34,
nTongReputation@0x40, + RequireCorpsValue/MaskCorpsNeedToCheck/CanReturn/MentorValue/RequireTitle.
Loader columns (in order): TabType,ItemIndex,RandSeed,CountLimit,Durability,ReputeLevel,
RequireCorpsValue,MaskCorpsNeedToCheck,CanReturn, Price, Coin, Prestige, Contribution, Justice,
ExamPrint, ArenaAward, ActivityAward, RequireAchievementRecord, AchievementPoint, TongReputation,
ItemType1, ItemIndex1, ItemCount1, MentorValue, RequireTitle. Assert relaxed: nPrice>0 || nCoin>0.
Respond codes added in v2.5 (SHOP_SYSTEM_RESPOND_CODE): ssrcNotEnough{Coin,Justice,ExamPrint,
ArenaAward,ActivityAward,TongReputation} + ssrcHaveTooMuch{Justice,ExamPrint,ArenaAward,Activity} + ssrcTongPayRepair.

## [RE-2] Buy flow (2010 pattern, extend for new currencies)
CanBuyItem: per-currency `if (price>0) require player-has >= price*count else ssrcNotEnoughXxx`.
BuyItem: per-currency `if (price>0) deduct`. 2010 does this for Prestige (AddPrestige) + Contribution
(AddContribution). New currencies use m_CurrencyList.GetCurrency(type)->GetValue()/AddCurrency(-cost).
Coin uses KPlayer::AddCoin(-cost).

## [PORT-1] Shop currency-buy (BOUNDED: Coin + 4 new KCurrency types) — build ok=202, boot [OK]
- KShopCenter.h KNPC_SHOP_TEMPLATE_ITEM: +nCoin,nJustice,nExamPrint,nArenaAward,nActivityAward.
- KShopCenter.cpp loader: read Coin/Justice/ExamPrint/ArenaAward/ActivityAward columns; relaxed
  price assert to (nPrice>0 || nCoin>0).
- KShop.cpp CanBuyItem: afford-check Coin (m_nCoin) + Justice/ExamPrint/ArenaAward/ActivityAward
  (GetCurrency(2..5)->GetValue() >= cost*count) -> ssrcNotEnough{...} (respond code index math). C++ = anti-hack.
- KShop.cpp BuyItem: deduct Coin (AddCoin) + the 4 currencies (AddCurrency(-cost)).
- SO3Result.h: +ssrcNotEnough{Coin,Justice,ExamPrint,ArenaAward,ActivityAward} before ssrcTotal.
Contribution(0)/Prestige(1) keep their existing 2010 shop-payment (legacy fields) — consistent with
the Lua slice decision (types 0/1 legacy, 2-5 KCurrency).

## DEFER (the rest of the KShop v2.5 upgrade — separate, non-currency)
- Non-currency requirement drift: RequireCorpsValue/MaskCorps (corps/团 shops), TongReputation,
  MentorValue, RequireTitle, CanReturn, HaveTooMuch cap-checks. Each its own feature.
- DoSyncCurrency/DoSyncCurrencyList client packet (balance realtime) — NEW protocol tier.
- Client-side respond-code display for the new ssrc codes (PAP2 version-gap).

---

## [S3 BLOCKER — RE-proven] DoSyncCurrency needs a full GS_CLIENT_PROTOCOL realignment
Client at /Volumes/Data/909160_剑侠3/客户端/JX3 (JX3Client.exe, Sep-Nov 2012) IS a real v2.5.2 client
(resolves the "no client" concern). Server-side DoSyncCurrency is fully RE'd:
- KPlayerServer::DoSyncCurrency (0805b406): send to owner conn — packet 11B = [WORD proto=0xcf][BYTE type]
  [DWORD value][DWORD remainSpace]. DoSyncCurrencyList (0807e52a): loop type 0..5 -> DoSyncCurrency.
- v246 GS_CLIENT_PROTOCOL: s2c_sync_currency = 207 (0xcf), between s2c_sync_camp_info(206) / s2c_sync_tong_info(208).

**BUT the client-facing protocol enum has PERVASIVE 2010->v246 drift (verified DWARF vs source):**
  packet        v246   2010
  message_notify   1     1   (aligned at start)
  begin_roll_item 124   ~   
  battle_field_end 187  ~168
  sync_camp_info  206   169   (+37)
v246 inserted ~37 packets scattered through the enum (auction, tong, battlefield expansions, currency...).
=> The 2010 server sends 2010-numbered protocol bytes; the v246 client expects v246 numbers. They agree
only for the earliest packets. Sending currency as 207 alone doesn't help — the client also misreads every
other late 2010 packet. Using THIS v246 client for anything past the early region requires realigning the
ENTIRE GS_CLIENT_PROTOCOL enum to v246 (insert ~37 packets at correct ordinals + their structs) — a large,
delicate protocol-tier subproject (cf. the KR2S/KS2R realignment, network §R10). Currency-sync is 1 line of it.
Verify = live v246-client login (env-heavy). DEFERRED as its own slice: "GS_CLIENT_PROTOCOL 2010->v246 realign".

The currency LOOP works server-side regardless (grant/hold/spend/persist/reset, all verified). Only the
client's realtime balance DISPLAY needs this realignment; balance is correct on relog (DB Load).
