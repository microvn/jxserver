/* -------------------------------------------------------------------------
//	文件名		：	protocol/kg_spreader_protocol.h
//	创建者		：	xiewen
//	创建时间	：	2009-12-25 10:38:16
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __PROTOCOL_KG_SPREADER_PROTOCOL_H__
#define __PROTOCOL_KG_SPREADER_PROTOCOL_H__

#include "protocolbasic.h"

#define	KSPREADER_PROTOCOL_VER		1
#define _KD_MAX_NAME_LEN					33

#pragma pack(push, 1)
// -------------------------------------------------------------------------
enum KE_S2GC_PROTOCOL // SpreaderSys -> GameCenter
{
	emKS2GC_START,

	emKS2GC_PING_REPLY,
	emKS2GC_ADD_CONSUME_RECORD_REPLY,

	emKS2GC_END
};

enum KE_GC2S_PROTOCOL // GameCenter -> SpreaderSys
{
	emKGC2S_START,

	emKGC2S_ADD_CONSUME_RECORD, // 增加消费记录
	emKGC2S_PING,

	emKGC2S_END
};

// gc -> ss
typedef struct tagKCONSUME_RECORD : ProtocolHeader
{
	GUID Guid; // 用于追踪数据包
	CHAR szZoneGroup[_KD_MAX_NAME_LEN];
	CHAR szAccount[_KD_MAX_NAME_LEN];
	CHAR szRoleName[_KD_MAX_NAME_LEN];
	INT nConsumeFee;
}KCONSUME_RECORD;

// ss -> gc
typedef struct tagKCONSUME_RECORD_REPLY : ProtocolHeader
{
	INT nResult; // AddConsumeRecord操作结果：1成功，0失败，-1重复的GUID键, -2稍后重试
	GUID Guid; // 用于追踪数据包
}KCONSUME_RECORD_REPLY;

// -------------------------------------------------------------------------
#pragma pack(pop)

#endif /* __PROTOCOL_KG_SPREADER_PROTOCOL_H__ */
