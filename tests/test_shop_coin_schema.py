from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(__file__).resolve().parents[1]
SOURCE = BUILD / "src/SO3World/Src/KShopCenter.cpp"
SHOP_FILE = (
    ROOT / "镜像端/extracted/root/settings/shop/Usual/Weapon_Sword.tab"
)


def test_shop_loader_accepts_target_coin_amount1_schema():
    """Regression: v2.5 shop data has CoinType1/CoinAmount1, not Coin."""
    source = SOURCE.read_text()
    header = SHOP_FILE.read_text(encoding="gbk").splitlines()[0].split("\t")

    assert "CoinAmount1" in header
    assert "Coin" not in header
    assert 'FindColumn("Coin")' in source
    assert 'FindColumn("CoinAmount1")' in source


def test_shop_loader_does_not_treat_coin_type_as_coin_amount():
    source = SOURCE.read_text()

    assert 'GetInteger(nLine, "CoinType1"' not in source
