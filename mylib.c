#include "mylib.h"

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

#define DEBUG 0
/* length of derived key */
#define KEYLEN 64

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

const char * foo(const char *salt, const char *passphrase, const char *site)
{
    char derived_key[KEYLEN] = { 0 };
    char hexResult[2 * KEYLEN + 1] = { 0 };
    unsigned int md_len = 0;
    /* unsigned char *md = NULL; */
    /* char *data = "OneShallPass v2.0saltgoogle.com10"; */
    unsigned char data[] = {149, 177, 79, 110, 101, 83, 104, 97, 108, 108, 80, 97, 115, 115, 32, 118, 50, 46, 48, 164, 115, 97, 108, 116, 170, 103, 111, 111, 103, 108, 101, 46, 99, 111, 109, 1, 0};
    int i;

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /* password based derivation routines with salt and iteration count */
    PKCS5_PBKDF2_HMAC("password", 8, "salt", 4, 256, EVP_sha512(), KEYLEN, derived_key);

    if (DEBUG) {
        for (i = 0; i < KEYLEN; i++)
        {
            sprintf(hexResult + (i * 2), "%02x", 255 & derived_key[i]);
            /* binResult[i] = digest[i]; */
        };

        printf("dk:\n");
        for (i = 0; i < KEYLEN/4; i++) {
            printf("[%d] %d\n", i, ntohl(*(int *)(derived_key+4*i)));
        }
    }

    
    /* unsigned char *digest = HMAC(EVP_sha512(), derived_key, KEYLEN, data, sizeof(data), NULL, &md_len); */
    unsigned char *digest = HMAC(EVP_sha512(), derived_key, KEYLEN, data, sizeof(data), NULL, &md_len);
    printf("md_len: %d\n", md_len);

    if (DEBUG) {
        for (i = 0; i < md_len; i++)
        {
            sprintf(hexResult + (i * 2), "%02x", 255 & digest[i]);
            /* binResult[i] = digest[i]; */
        };

        digest[md_len] = 0;
        printf("digest raw: %s\n", digest);

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
    int num_args = 0;

    num_args = lua_gettop(L);
    printf("%d\n", num_args);

    salt = lua_tostring(L, 1);
    passphrase = lua_tostring(L, 2);
    site = lua_tostring(L, 3);

    if (DEBUG) {
        printf("%s %s %s\n", salt, passphrase, site);
    }

    /* push result to stack to lua */
    /* lua_pushnumber(L, 1000); */
    lua_pushstring(L, foo(salt, passphrase, site));

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
