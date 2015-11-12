#include <stdio.h>
#include <string.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <arpa/inet.h>

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

int *pack_data(unsigned char *data, int len)
{
    int i, j;

    int *tmp = malloc(sizeof(int) * ((len-1)/4 + 1));
    for (i = 0; i < len - 4; i+=4) {
        tmp[i/4] = (data[i]<<24) | (data[i+1] << 16) | (data[i+2]<<8) | (data[i+3]);
    }
    for (i = 0; i < (len-1)/4 + 1; i++) {
        printf("%d ", tmp[i]);
    }
    return tmp;
}
        

int main(int argc, char **argv)
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
    for (i = 0; i < KEYLEN; i++)
    {
        sprintf(hexResult + (i * 2), "%02x", 255 & derived_key[i]);
        /* binResult[i] = digest[i]; */
    };
    printf("dk:\n");
    for (i = 0; i < KEYLEN/4; i++) {
        printf("[%d] %d\n", i, ntohl(*(int *)(derived_key+4*i)));
    }

    
    /* unsigned char *digest = HMAC(EVP_sha512(), derived_key, KEYLEN, data, sizeof(data), NULL, &md_len); */
    unsigned char *digest = HMAC(EVP_sha512(), derived_key, KEYLEN, data, sizeof(data), NULL, &md_len);
    printf("md_len: %d\n", md_len);
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

    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    return 0;
}
