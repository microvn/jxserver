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
