from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
PLAYER_CPP = ROOT / "src/SO3World/Src/KPlayer.cpp"
REGRESSION_H = ROOT / "src/SO3World/Src/KRegressionPlayerData.h"


class RegressionRoleBlockTest(unittest.TestCase):
    def test_role_regression_block_uses_target_player_chunk(self):
        """Regression: role block 33 is target LoadPlayerData (46B), not 68B."""
        player_cpp = PLAYER_CPP.read_text(encoding="utf-8")
        regression_h = REGRESSION_H.read_text(encoding="utf-8")

        self.assertIn("LoadPlayerData(BYTE* pbyData, size_t uDataLen)", regression_h)
        self.assertIn("m_RegressionData.LoadPlayerData(pbyOffset, pBlock->dwLen)", player_cpp)


if __name__ == "__main__":
    unittest.main()
