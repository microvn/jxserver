from pathlib import Path


ROOT = Path(__file__).parents[1]
PROTO = (ROOT / "include/Include/GS_Client_Protocol.h").read_text(encoding="utf-8", errors="ignore")
SERVER = (ROOT / "src/SO3World/Src/KPlayerServer.cpp").read_text(encoding="utf-8", errors="ignore")


def test_exterior_packet_keeps_target_nested_record_layout():
    start = PROTO.index("struct S2C_SYNC_EXTERIOR_ALL_SET_DATA")
    end = PROTO.index("// Exterior apply-flag", start)
    block = PROTO[start:end]
    assert "struct EXTERIOR_SET_DATA" in block
    assert "WORD wExteriorSet[5];" in block
    assert "} ExteriorSet[0];" in block
    assert "WORD  wExteriorSet[0];" not in block


def test_exterior_sender_uses_nested_record_and_target_sizing_chain():
    start = SERVER.index("BOOL KPlayerServer::DoSyncExteriorAllSetData")
    end = SERVER.index("BOOL KPlayerServer::DoSyncClientReportConfig", start)
    block = SERVER[start:end]
    assert "pPacket->ExteriorSet[i].wExteriorSet[j]" in block
    assert "pPacket->wSize = (WORD)(sizeof(*pPacket) + uSetCount * 5 * sizeof(WORD));" in block


def test_client_report_uses_packed_target_schema():
    assert "struct S2C_CLIENT_REPORT_CONFIG : DOWNWARDS_PROTOCOL_HEADER" in PROTO
    assert "BYTE byNetworkDelayReportFlag;" in PROTO
    assert "int  nNetworkDelayReportFrequency;" in PROTO
    assert "int  nNetworkDelayReportThreshold;" in PROTO
    assert "S2C_CLIENT_REPORT_CONFIG_LOCAL" not in SERVER
    assert "Pak.byProtocolID = s2c_client_report_config;" in SERVER
