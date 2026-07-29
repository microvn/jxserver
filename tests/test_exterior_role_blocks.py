import pathlib
import struct
import unittest


PLAYER_CPP = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp"
ROLE_DEF = pathlib.Path(__file__).parents[1] / "include/Include/KRoleDBDataDef.h"
EXTERIOR_CPP = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KExteriorBox.cpp"
EXTERIOR_H = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KExteriorBox.h"


def read_source(path):
    return path.read_bytes().decode("gbk", errors="replace")


class ExteriorRoleBlockTest(unittest.TestCase):
    def test_target_exterior_block_framing(self):
        latest_buy = struct.pack("<H", 0)
        exterior_set = struct.pack("<H", 1) + bytes(18) + bytes(3)
        payload = struct.pack("<H", len(latest_buy)) + latest_buy
        payload += struct.pack("<H", len(exterior_set)) + exterior_set

        self.assertEqual(len(payload), 29)
        self.assertEqual(struct.unpack_from("<H", payload, 0)[0], 2)
        self.assertEqual(struct.unpack_from("<H", payload, 4)[0], 23)

    def test_target_block_ids_route_to_target_loaders(self):
        player = read_source(PLAYER_CPP)
        role_def = read_source(ROLE_DEF)
        exterior = read_source(EXTERIOR_CPP)
        header = read_source(EXTERIOR_H)

        # Regression: legacy rbtExteriorData=23 was applied to target PendentData.
        self.assertIn("rbtExteriorBoxData = 30", role_def)
        self.assertIn("rbtExteriorSetData = 31", role_def)
        self.assertIn("case rbtExteriorBoxData:", player)
        self.assertIn("case rbtExteriorSetData:", player)
        self.assertIn("LoadExteriorBox", exterior)
        self.assertIn("LoadExteriorBox", header)


if __name__ == "__main__":
    unittest.main()
