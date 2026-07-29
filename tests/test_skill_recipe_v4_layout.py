import pathlib
import struct
import unittest


SOURCE = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KSkillRecipeList.cpp"
HEADER = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KSkillRecipeList.h"


class SkillRecipeV4LayoutTest(unittest.TestCase):
    def test_live_v4_block_has_target_three_byte_recipe_nodes(self):
        payload = bytes.fromhex("04 00 01 00 f1 27 00")
        version, count = struct.unpack_from("<HH", payload)
        recipe_key, active = struct.unpack_from("<HB", payload, 4)

        self.assertEqual((version, count), (4, 1))
        self.assertEqual((recipe_key, active), (0x27F1, 0))
        self.assertEqual(4 + count * 3, len(payload))

    def test_loader_dispatches_version_four(self):
        source = SOURCE.read_text(encoding="ascii")
        header = HEADER.read_text(encoding="ascii")

        # Regression: type 10 version 4 (len=7) was rejected before parsing.
        self.assertIn("case 4:", source)
        self.assertIn("_LoadDataV4", source)
        self.assertIn("KG_SKILL_RECIPE_LIST_DB_DATA_V4*", source)
        self.assertIn("KG_SKILL_RECIPE_DB_CURRENT_VER      4", header)

    def test_save_uses_v4_wire_layout(self):
        source = SOURCE.read_text(encoding="ascii")

        self.assertIn("KG_SKILL_RECIPE_LIST_DB_DATA_V4*    pSkillRecipeData", source)
        self.assertIn("pSkillRecipeData->RecipeArray[i].wRecipeKey", source)


if __name__ == "__main__":
    unittest.main()
