-- End-to-end exterior (wai-guan) test — run from a GM script on a logged-in player.
-- Verifies the full ported chain: grant -> set-slot -> apply -> represent changes.
-- Requires: a real exterior ID from settings/Exterior/ExteriorInfo.tab whose
-- `subtype` matches slot 0 (Chest = subtype 2). Replace TEST_ID accordingly.
--
-- Environment note: needs a client that can log a player in. The stock 2010
-- (pap2) client has no exterior UI (version-gap); rendering needs a v246 client.
-- The server-side state changes below are observable via GetRepresentID().

local TEST_ID = 1        -- <-- set to a real Chest(subtype 2) exterior ID

local function dump_represent(player)
    local t = player:GetRepresentID()
    -- index 5 = perChestStyle, 6 = perChestColor (slot 0)
    print(string.format("  represent[5]=%s represent[6]=%s", tostring(t[5]), tostring(t[6])))
end

local function test(player)
    print("[exterior] grant TEST_ID=" .. TEST_ID)
    print("  AddExterior ->", player:AddExterior(TEST_ID, 0, 1, 1))  -- permanent, payMoney, direct

    print("  AddExteriorSetCount(1) (create set 0)")
    player:AddExteriorSetCount(1)

    print("  SetExteriorSet(0, {chest=TEST_ID})")
    player:SetExteriorSet(0, { [0] = TEST_ID, [1] = 0, [2] = 0, [3] = 0, [4] = 0 })

    print("  set 0 contents:")
    local s = player:GetExteriorSet(0)
    for i = 0, 4 do print(string.format("    slot %d = %s", i, tostring(s[i]))) end

    print("  before apply:"); dump_represent(player)
    print("  ApplyExterior ->", player:ApplyExterior())
    print("  IsApplyExterior ->", player:IsApplyExterior())
    print("  after apply (represent[5]/[6] should now match the exterior):")
    dump_represent(player)

    print("  UnApplyExterior ->", player:UnApplyExterior())
    print("  after un-apply (represent restored to real gear / 0):")
    dump_represent(player)
end

return test
