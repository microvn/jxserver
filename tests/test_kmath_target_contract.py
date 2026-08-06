"""Target-derived static contract for the KMath free-function root.

This deliberately uses target values/instruction facts, not a source round trip.
It validates the candidate's observable branch and table contract and includes
mutation checks against the validator itself.
"""
from pathlib import Path
from bisect import bisect_left
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "src/SO3World/Src/KMath.h"
SOURCE = ROOT / "src/SO3World/Src/KMath.cpp"
TARGET_SINE = [0, 101, 201, 301, 401, 501, 601, 700, 799, 897, 995, 1092, 1189, 1285, 1380, 1474, 1567, 1660, 1751, 1842, 1931, 2019, 2106, 2191, 2276, 2359, 2440, 2520, 2598, 2675, 2751, 2824, 2896, 2967, 3035, 3102, 3166, 3229, 3290, 3349, 3406, 3461, 3513, 3564, 3612, 3659, 3703, 3745, 3784, 3822, 3857, 3889, 3920, 3948, 3973, 3996, 4017, 4036, 4052, 4065, 4076, 4085, 4091, 4095, 4096]
TARGET_TAN = [50, 151, 252, 353, 454, 556, 659, 763, 867, 973, 1080, 1188, 1298, 1409, 1523, 1638, 1756, 1876, 1999, 2125, 2254, 2387, 2524, 2665, 2810, 2961, 3116, 3278, 3446, 3622, 3805, 3997, 4198, 4409, 4632, 4868, 5118, 5383, 5667, 5970, 6296, 6647, 7028, 7442, 7895, 8392, 8943, 9555, 10242, 11019, 11906, 12929, 14124, 15540, 17247, 19348, 22000, 25457, 30158, 36935, 47564, 66670, 111207, 333752]


def numbers_for(text, array_name):
    match = re.search(r"static\s+int\s+%s\[\]\s*=\s*\{(.*?)\};" % array_name, text, re.S)
    if not match:
        raise AssertionError("missing target-backed array %s" % array_name)
    return [int(value) for value in re.findall(r"\b\d+\b", match.group(1))]


def require_in_order(text, fragments):
    cursor = 0
    for fragment in fragments:
        found = text.find(fragment, cursor)
        if found < 0:
            raise AssertionError("missing or reordered target fragment: %r" % fragment)
        cursor = found + len(fragment)


def target_distance3(ax, ay, az, bx, by, bz):
    return (ax - bx) ** 2 + (ay - by) ** 2 + (az - bz) ** 2


def target_in_range(ax, ay, az, bx, by, bz, radius):
    return target_distance3(ax, ay, az, bx, by, bz) <= radius * radius


def target_direction(nx, ny):
    if nx == 0:
        return 64 if ny > 0 else 192 if ny < 0 else 0
    ax, ay = abs(nx), abs(ny)
    slope = (ay * 4096) // ax
    # Target re-evaluates this multiplication in 64-bit at the stated guard.
    if ay >= 0x7FFFE:
        slope = (ay * 4096) // ax
    direction1 = bisect_left(TARGET_TAN, slope)
    if nx < 0:
        return 128 - direction1 if ny >= 0 else 128 + direction1
    if ny >= 0:
        return direction1
    return 256 - direction1 if direction1 > 0 else 0


def assert_target_contract(header, source):
    assert numbers_for(source, "g_nSinValues") == TARGET_SINE
    assert numbers_for(source, "g_nTanValues") == TARGET_TAN
    require_in_order(source, [
        "nDirection = nDirection % DIRECTION_COUNT;",
        "if (nDirection < 0)",
        "nDirection += DIRECTION_COUNT;",
        "if (nDirection <= DIRECTION_COUNT / 4)",
        "else if (nDirection <= DIRECTION_COUNT / 2)",
        "else if (nDirection <= DIRECTION_COUNT * 3 / 4)",
        "nResult = -g_nSinValues[DIRECTION_COUNT - nDirection];",
    ])
    require_in_order(source, [
        "if (nX == 0)",
        "nValue = nY1 * TAN_PRECISION / nX1;",
        "if (nY1 >= INT_MAX / TAN_PRECISION)",
        "(int)(((long long)nY1) * TAN_PRECISION / nX1)",
        "lower_bound(g_nTanValues, g_nTanValues + TAN_VALUE_COUNT, nValue)",
        "assert(nDirection >= 0);",
        "assert(nDirection < DIRECTION_COUNT);",
    ])
    require_in_order(header, [
        "T nDistance3 = \n        (nSourceX - nDestX) * (nSourceX - nDestX) +",
        "(nSourceY - nDestY) * (nSourceY - nDestY) +",
        "(nSourceZ - nDestZ) * (nSourceZ - nDestZ);",
        "return nDistance3;",
        "return (T)g_GetDirection((int)(nDestX - nSourceX), (int)(nDestY - nSourceY));",
        "return g_Sin(DIRECTION_COUNT / 4 - nDirection);",
        "long long   llRange     = (long long)nRange * nRange;",
        "(long long)nXa, (long long)nYa, (long long)nZa",
        "KG_PROCESS_ERROR(llDistance <= llRange);",
    ])
    enum_block = re.search(r"enum\s+IN_RANGE_RESULT\s*\{(.*?)\};", header, re.S)
    assert enum_block
    enum_values = re.findall(r"\b(irr(?:Invalid|InRange|TooClose|TooFar|OutOfAngle|Total))\b", enum_block.group(1))
    assert enum_values == ["irrInvalid", "irrInRange", "irrTooClose", "irrTooFar", "irrOutOfAngle", "irrTotal"]

    # Target-derived arithmetic and quadrant oracle values (DWARF/decompile
    # functions plus data at 0x084d98a0 and 0x084d99c0).
    assert target_distance3(1, -2, 3, -4, 5, -6) == 155
    assert target_in_range(0, 0, 0, 3, 4, 0, 5)
    assert not target_in_range(0, 0, 0, 3, 4, 0, 4)
    assert {pair: target_direction(*pair) for pair in [(0, 1), (0, -1), (1, 0), (1, -1), (-1, 0), (-1, 1), (-1, -1), (1, 1), (2, 1), (1, 2)]} == {
        (0, 1): 64, (0, -1): 192, (1, 0): 0, (1, -1): 224,
        (-1, 0): 128, (-1, 1): 96, (-1, -1): 160, (1, 1): 32,
        (2, 1): 19, (1, 2): 45,
    }


class KMathTargetContractTest(unittest.TestCase):
    def test_target_derived_contract(self):
        assert_target_contract(HEADER.read_text(encoding="latin-1"), SOURCE.read_text(encoding="latin-1"))

    def test_mutations_are_rejected(self):
        header = HEADER.read_text(encoding="latin-1")
        source = SOURCE.read_text(encoding="latin-1")
        with self.assertRaises(AssertionError):
            assert_target_contract(header, source.replace("nDirection += DIRECTION_COUNT;", "nDirection -= DIRECTION_COUNT;", 1))
        with self.assertRaises(AssertionError):
            assert_target_contract(header, source.replace("nValue = nY1 * TAN_PRECISION / nX1;", "nValue = nX1 * TAN_PRECISION / nY1;", 1))
        with self.assertRaises(AssertionError):
            assert_target_contract(header, source.replace("50, 151", "51, 151", 1))
        with self.assertRaises(AssertionError):
            assert_target_contract(header.replace("KG_PROCESS_ERROR(llDistance <= llRange);", "KG_PROCESS_ERROR(llDistance < llRange);", 1), source)


if __name__ == "__main__":
    unittest.main()
