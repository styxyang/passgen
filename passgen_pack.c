#include "meta.h"

#include <stdlib.h>
#include <intttypes.h>
#include <arpa/inet.h>

#define UINT8_MAX  0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffff

static uint8_t pack_length(uint32_t len, uint8_t fixmin, uint8_t fixmax)
{
    uint8_t *buf = NULL;
    int   n = 0;

    if (len <= UINT8_MAX) {
        if (len <= ((uint32_t)fixmax - (uint32_t)fixmin)) {
            n = 1;
            buf = malloc(sizeof(uint8_t) * n);
            buf[0] = (len|fixmin);
        } else {
            n = 2;
            buf = malloc(sizeof(uint8_t) * n);
            buf[0] = PT_UINT8;
            buf[1] = len;
        }
    } else if (len <= UINT16_MAX) {
        n = 3;
        buf = malloc(sizeof(uint8_t) * n);
        buf[0] = PT_UINT16;
        *(uint16_t *)(buf+1) = htons(len);
    } else {
        n = 5;
        buf = malloc(sizeof(uint8_t) * n);
        buf[0] = PT_UINT32;
        *(uint32_t *)(buf+1) = htonl(len);
    }
}
