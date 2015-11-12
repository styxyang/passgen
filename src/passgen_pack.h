#ifndef META_H
#define META_H

#include <inttypes.h>
#include <sys/types.h>

uint8_t *pack_string(const char *s, size_t *n);
uint8_t *pack_array(uint32_t len, size_t *n);

#endif /* META_H */
