#include "passgen_pack.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>

enum {
    PT_NULL = 0,
    PT_FALSE,
    PT_TRUE,
    PT_BIN8,
    PT_BIN16,
    PT_BIN32,
    PT_EXT8,
    PT_EXT16,
    PT_EXT32,
    PT_FLOAT,
    PT_DOUBLE,
    PT_UINT8,
    PT_UINT16,
    PT_UINT32,
    PT_UINT64,
    PT_INT8,
    PT_INT16,
    PT_INT32,
    PT_INT64,
    PT_FIX_EXT1,
    PT_FIX_EXT2,
    PT_FIX_EXT4,
    PT_FIX_EXT8,
    PT_FIX_EXT16,
    PT_STR8,
    PT_STR16,
    PT_STR32,
    PT_ARRAY16,
    PT_ARRAY32,
    PT_MAP16,
    PT_MAP32,
    PT_FIX_STR_MIN,
    PT_FIX_STR_MAX,
    PT_FIX_ARRAY_MIN,
    PT_FIX_ARRAY_MAX,
    PT_FIX_MAP_MIN,
    PT_FIX_MAP_MAX,
    PT_FIX_ARRAY_COUNT_MASK,
    PT_FIX_MAP_COUNT_MASK,
    PT_FIX_STR_COUNT_MASK,
    PT_NEGATIVE_FIX_MIN,
    PT_NEGATIVE_FIX_MAX,
    PT_NEGATIVE_FIX_MASK,
    PT_NEGATIVE_FIX_OFFSET,
    PT_POSITIVE_FIX_MAX,
    PT_RPC_REQUEST,
    PT_RPC_RESPONSE,
    PT_RPC_NOTIFY,
    PT_MAX,
};


static uint8_t meta[] = {
    /* null                 : 0xc0 */
    /* false                : 0xc2 */
    /* true                 : 0xc3 */
    /* bin8                 : 0xc4 */
    /* bin16                : 0xc5 */
    /* bin32                : 0xc6 */
    /* ext8                 : 0xc7 */
    /* ext16                : 0xc8 */
    /* ext32                : 0xc9 */
    /* float                : 0xca */
    /* double               : 0xcb */
    /* uint8                : 0xcc */
    /* uint16               : 0xcd */
    /* uint32               : 0xce */
    /* uint64               : 0xcf */
    /* int8                 : 0xd0 */
    /* int16                : 0xd1 */
    /* int32                : 0xd2 */
    /* int64                : 0xd3 */
    /* fix_ext1             : 0xd4 */
    /* fix_ext2             : 0xd5 */
    /* fix_ext4             : 0xd6 */
    /* fix_ext8             : 0xd7 */
    /* fix_ext16            : 0xd8 */
    [PT_STR8]                 = 0xd9,
    [PT_STR16]                = 0xda,
    [PT_STR32]                = 0xdb,
    [PT_ARRAY16]              = 0xdc,
    [PT_ARRAY32]              = 0xdd,
    /* map16                : 0xde */
    /* map32                : 0xdf */
    [PT_FIX_STR_MIN]          = 0xa0,
    [PT_FIX_STR_MAX]          = 0xbf,
    [PT_FIX_ARRAY_MIN]        = 0x90,
    [PT_FIX_ARRAY_MAX]        = 0x9f,
    /* fix_map_min          : 0x80 */
    /* fix_map_max          : 0x8f */
    /* fix_array_count_mask : 0xf */
    /* fix_map_count_mask   : 0xf */
    /* fix_str_count_mask   : 0x1f */
    /* negative_fix_min     : 0xe0 */
    /* negative_fix_max     : 0xff */
    /* negative_fix_mask    : 0x1f */
    /* negative_fix_offset  : 0x20 */
    /* positive_fix_max     : 0x7f */
    /* rpc_request          : 0x00 */
    /* rpc_response         : 0x01 */
    /* rpc_notify           : 0x02 */
};

uint8_t *pack_length(uint32_t len, uint8_t fixmin, uint8_t fixmax,
        uint8_t m, uint8_t b, uint8_t s, size_t *n)
{
    uint8_t *buf = NULL;

    if (len <= ((uint32_t)fixmax - (uint32_t)fixmin)) {
        *n = 1;
        buf = malloc(*n * sizeof(uint8_t));
        buf[0] = (len|fixmin);
    } else if (s != 0 && len <= UINT8_MAX) {
        *n = 2;
        buf = malloc(*n * sizeof(uint8_t));
        buf[0] = meta[PT_UINT8];
        buf[1] = len;
    } else if (len <= UINT16_MAX) {
        *n = 3;
        buf = malloc(*n * sizeof(uint8_t));
        buf[0] = meta[PT_UINT16];
        *(uint16_t *)(buf+1) = htons(len);
    } else {
        *n = 5;
        buf = malloc(*n * sizeof(uint8_t));
        buf[0] = meta[PT_UINT32];
        *(uint32_t *)(buf+1) = htonl(len);
    }

    return buf;
}

uint8_t *pack_string(const char *s, size_t *n)
{
    size_t  slen = strlen(s);
    size_t  nbytes = 0;
    uint8_t *plen = NULL;
    uint8_t *pbuf = NULL;

    /* prepare_utf8() */

    plen = pack_length(slen, meta[PT_FIX_STR_MIN], meta[PT_FIX_STR_MAX],
            meta[PT_STR16], meta[PT_STR32], meta[PT_STR8], &nbytes);

    /* pbuf <- plen . s */
    pbuf = malloc(sizeof(uint8_t) * (slen + nbytes + 1));
    memset(pbuf, 0, slen + nbytes + 1);
    memcpy(pbuf, plen, nbytes);
    memcpy(pbuf + nbytes, s, slen);

    free(plen);

    *n = nbytes + slen;

    return pbuf;
}

uint8_t *pack_array(uint32_t len, size_t *n)
{
    return pack_length(len, meta[PT_FIX_ARRAY_MIN], meta[PT_FIX_ARRAY_MAX],
            meta[PT_ARRAY16], meta[PT_ARRAY32], 0, n);
}
