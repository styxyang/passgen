#include "mylib.h"
#include "passgen_pack.h"

#include <stdio.h>
#include <string.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <arpa/inet.h>

#include <lua.h>
#include <lauxlib.h>

#define DEBUG 1
/* length of derived key */
#define KEYLEN 64

const char *ver = "OneShallPass v2.0";

char * base64(unsigned char * input, int length) {

    BIO *b64 = NULL;
    BIO * bmem = NULL;
    BUF_MEM *bptr = NULL;
    char * output = NULL;

    b64 = BIO_new((BIO_METHOD *)BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    output = (char *) calloc (bptr->length + 1, sizeof(char));
    memcpy(output, bptr->data, bptr->length);

    BIO_free_all(b64);

    return output;
}

static void print_bytes(uint8_t *v, size_t n, const char *name)
{
    int i;

    printf("%s:", name);
    for (i = 0; i < n; i++) {
        printf(" %d", v[i]);
    }
    printf("\n");
}

const char * foo(const char *salt, const char *passphrase, const char *site,
        uint32_t generation, uint32_t itercnt)
{
    char derived_key[KEYLEN] = { 0 };
    char hexResult[2 * KEYLEN + 1] = { 0 };
    unsigned int md_len = 0;
    /* unsigned char *md = NULL; */
    uint8_t *ver_buf = NULL, *salt_buf = NULL, *site_buf = NULL, *array_buf = NULL;
    size_t nb_array = 0, nb_ver = 0, nb_salt = 0, nb_site = 0;
    int i;
    uint8_t *data = NULL;
    size_t datalen = 0;

    array_buf = pack_array(5, &nb_array);
    datalen += nb_array;

    ver_buf = pack_string(ver, &nb_ver);
    datalen += nb_ver;

    salt_buf = pack_string(salt, &nb_salt);
    datalen += nb_salt;

    site_buf = pack_string(site, &nb_site);
    datalen += nb_site;

    if (DEBUG) {
        print_bytes(array_buf, nb_array, "array");
        print_bytes(ver_buf, nb_ver, "var");
        print_bytes(salt_buf, nb_salt, "salt");
        print_bytes(site_buf, nb_site, "site");
    }

    datalen += 2;

    data = malloc(datalen * sizeof(uint8_t));
    memcpy(data, array_buf, nb_array);
    memcpy(data + nb_array, ver_buf, nb_ver);
    memcpy(data + nb_array + nb_ver, salt_buf, nb_salt);
    memcpy(data + nb_array + nb_ver + nb_salt, site_buf, nb_site);
    data[nb_array + nb_ver + nb_salt + nb_site] = generation;
    data[nb_array + nb_ver + nb_salt + nb_site + 1] = itercnt;
    free(array_buf);
    free(ver_buf);
    free(salt_buf);
    free(site_buf);

    /* unsigned char data[] = {149, 177, 79, 110, 101, 83, 104, 97, 108, 108, 80, 97, 115, 115, 32, 118, 50, 46, 48, 164, 115, 97, 108, 116, 170, 103, 111, 111, 103, 108, 101, 46, 99, 111, 109, 1, 0}; */
    if (DEBUG) {
        print_bytes(data, datalen, "data");
    }

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /* password based derivation routines with salt and iteration count */
    PKCS5_PBKDF2_HMAC(passphrase, strlen(passphrase), salt, strlen(salt),
            itercnt, EVP_sha512(), KEYLEN, derived_key);

    if (DEBUG) {
        for (i = 0; i < KEYLEN; i++) {
            sprintf(hexResult + (i * 2), "%02x", 255 & derived_key[i]);
            /* binResult[i] = digest[i]; */
        };

        printf("dk:\n");
        for (i = 0; i < KEYLEN/4; i++) {
            printf("[%d] %d\n", i, ntohl(*(int *)(derived_key+4*i)));
        }
    }

    
    /* unsigned char *digest = HMAC(EVP_sha512(), derived_key, KEYLEN, data, sizeof(data), NULL, &md_len); */
    unsigned char *digest = HMAC(EVP_sha512(), derived_key, KEYLEN, data, datalen, NULL, &md_len);
    printf("md_len: %d\n", md_len);

    if (DEBUG) {
        for (i = 0; i < md_len; i++)
        {
            sprintf(hexResult + (i * 2), "%02x", 255 & digest[i]);
            /* binResult[i] = digest[i]; */
        };

        digest[md_len] = 0;

        printf("digest:\n");
        for (i = 0; i< md_len/4; i++) {
            printf("[%d] %d\n", i, ntohl(*(int*)(digest+i*4)));
        }
        printf("digest %s\n", hexResult);
        /* base64 to the raw output you fking idiot! */
        printf("digest base64 %s\n", base64(digest, 64));
    }

    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    return base64(digest, 64);
}

/* Lua interface */
static int passgen(lua_State *L)
{
    const char *salt = NULL;
    const char *passphrase = NULL;
    const char *site = NULL;
    uint32_t gen, iter;
    int num_args = 0;

    num_args = lua_gettop(L);
    printf("%d\n", num_args);

    salt = lua_tostring(L, 1);
    passphrase = lua_tostring(L, 2);
    site = lua_tostring(L, 3);
    gen = lua_tonumber(L, 4);
    iter = lua_tonumber(L, 5);

    if (DEBUG) {
        printf("%s %s %s\n", salt, passphrase, site);
    }

    /* push result to stack to lua */
    /* lua_pushnumber(L, 1000); */
    lua_pushstring(L, foo(salt, passphrase, site, gen, (1 << iter)));

    return 1;
}

static const luaL_Reg mylib[] = {
    { "passgen", passgen },
    { NULL, NULL },
};

int luaopen_mylib(lua_State *L)
{
#if defined(LUA_51)
    luaL_register(L, "mylib", mylib);
    /* luaL_openlib(L, "mylib", mylib, 0); */
#elif defined(LUA_52)
    luaL_newlib(L, mylib);
#endif
    return 1;
}
