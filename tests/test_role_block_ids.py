import pathlib
import unittest


ROLE_DEF = pathlib.Path(__file__).parents[1] / "include/Include/KRoleDBDataDef.h"


class RoleBlockIdTest(unittest.TestCase):
    def test_target_v246_role_block_ids_are_explicit(self):
        source = ROLE_DEF.read_bytes().decode("gbk", errors="replace")

        # Regression: source-era implicit enum values routed Currency(25) as Arena(27).
        expected = {
            "rbtPendentData": 23,
            "rbtActivityVariables": 24,
            "rbtCurrencyData": 25,
            "rbtBankPasswordData": 26,
            "rbtArenaData": 27,
            "rbtExteriorBoxData": 30,
            "rbtExteriorOtherData": 31,
            "rbtHairBoxData": 32,
            "rbtRegressionData": 33,
            "rbtMiniAvatarData": 45,
            "rbtTotal": 49,
        }

        for name, value in expected.items():
            self.assertIn(f"{name} = {value}", source)


if __name__ == "__main__":
    unittest.main()
