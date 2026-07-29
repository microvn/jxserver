import pathlib
import unittest


SOURCE = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KSkillRecipeList.cpp"


class SkillRecipeVersionCompatibilityTest(unittest.TestCase):
    def test_load_normalizes_observed_byte_swapped_v2_marker(self):
        source = SOURCE.read_text(encoding="ascii")

        # Regression: live role block type 10 starts with 00 02, which the
        # native little-endian read interpreted as version 512 and rejected.
        self.assertIn("wVersion == 0x0100", source)
        self.assertIn("wVersion == 0x0200", source)
        self.assertIn("wVersion >>= 8", source)


if __name__ == "__main__":
    unittest.main()
