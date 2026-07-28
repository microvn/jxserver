#include "kg_socket.h"

/*
 * The 2010 KG_Socket archive recognises modes -1..3 only.  The v2.5 target
 * accepts KSG_ENCODE_DECODE_FAST (4); its _MakeSecurityKey jump table maps
 * Fast to the same 42-byte key shape as Dynamic.  Preserve the archive for
 * every legacy mode and bridge only this missing Fast entry.
 */
extern int RealMakeSecurityKey(
    ENCODE_DECODE_MODE EncodeDecodeMode,
    unsigned *puEncodeKey,
    unsigned *puDecodeKey,
    IKG_Buffer **ppiSecurityKey
) asm("_Z22_LegacyMakeSecurityKey18ENCODE_DECODE_MODEPjS0_PP10IKG_Buffer");

int _MakeSecurityKey(
    ENCODE_DECODE_MODE EncodeDecodeMode,
    unsigned *puEncodeKey,
    unsigned *puDecodeKey,
    IKG_Buffer **ppiSecurityKey
) 
{
    if (EncodeDecodeMode == KSG_ENCODE_DECODE_FAST)
        return RealMakeSecurityKey(
            KSG_ENCODE_DECODE_DYNAMIC,
            puEncodeKey,
            puDecodeKey,
            ppiSecurityKey
        );

    return RealMakeSecurityKey(
        EncodeDecodeMode,
        puEncodeKey,
        puDecodeKey,
        ppiSecurityKey
    );
}
