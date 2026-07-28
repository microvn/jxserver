#include "kg_socket.h"
#include "ksg_encodedecode.h"

typedef int (*KSG_ENCODE_DECODE_FUNCTION)(size_t, unsigned char *, unsigned *);

extern "C" int Encrypt(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);
extern "C" int Decrypt(size_t uSize, unsigned char *pbyBuf, unsigned *puKey);

static const unsigned g_KsgFastPublicKeys[] = {
#include "ksg_fast_public_keys.inc"
};

enum {
    KSG_FAST_PUBLIC_KEY_COUNT = 5679,
    KSG_FAST_WORD_DELTA = 0x2e6d23c1u,
    KSG_FAST_KEY_DELTA = 0x08088405u
};

typedef char KSG_FAST_TABLE_SIZE_CHECK[
    (sizeof(g_KsgFastPublicKeys) / sizeof(g_KsgFastPublicKeys[0]) == KSG_FAST_PUBLIC_KEY_COUNT) ? 1 : -1
];

int KSG_DecodeEncodeFast(size_t uSize, unsigned char *pbyBuf, unsigned *puKey)
{
    unsigned uWordCount = (unsigned)(uSize >> 2);
    unsigned uTailCount = (unsigned)(uSize & 3);
    unsigned uCode = g_KsgFastPublicKeys[(uWordCount + *puKey) % KSG_FAST_PUBLIC_KEY_COUNT];

    while (uWordCount > 0)
    {
        --uWordCount;
        uCode += uWordCount + KSG_FAST_WORD_DELTA;
        *(unsigned *)pbyBuf ^= uCode;
        pbyBuf += sizeof(unsigned);
    }

    uCode += KSG_FAST_WORD_DELTA;
    while (uTailCount > 0)
    {
        *pbyBuf ^= (unsigned char)uCode;
        ++pbyBuf;
        uCode >>= 8;
        --uTailCount;
    }

    *puKey = *puKey * 31 + KSG_FAST_KEY_DELTA;
    return 1;
}

int _SetEncodeDecodeFunction(
    ENCODE_DECODE_MODE EncodeDecodeMode,
    KSG_ENCODE_DECODE_FUNCTION *ppfnEncode,
    KSG_ENCODE_DECODE_FUNCTION *ppfnDecode
)
{
    if (ppfnEncode == NULL || ppfnDecode == NULL)
        return 0;

    switch (EncodeDecodeMode)
    {
    case KSG_ENCODE_DECODE_NONE:
        *ppfnEncode = NULL;
        *ppfnDecode = NULL;
        return 1;
    case KSG_ENCODE_DECODE:
        *ppfnEncode = KSG_EncodeBuf;
        *ppfnDecode = KSG_DecodeBuf;
        return 1;
    case EASYCRYPT_ENCODE_DECODE:
        *ppfnEncode = Encrypt;
        *ppfnDecode = Decrypt;
        return 1;
    case KSG_ENCODE_DECODE_SIMPLE:
        *ppfnEncode = KSG_EncodeBufSimple;
        *ppfnDecode = KSG_DecodeBufSimple;
        return 1;
    case KSG_ENCODE_DECODE_DYNAMIC:
        *ppfnEncode = KSG_EncodeDynamic;
        *ppfnDecode = KSG_DecodeDynamic;
        return 1;
    case KSG_ENCODE_DECODE_FAST:
        *ppfnEncode = KSG_DecodeEncodeFast;
        *ppfnDecode = KSG_DecodeEncodeFast;
        return 1;
    default:
        return 0;
    }
}
