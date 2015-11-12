#include "passgen_pack.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>

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
