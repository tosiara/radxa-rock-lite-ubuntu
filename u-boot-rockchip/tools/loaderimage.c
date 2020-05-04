#include "compiler.h"
#include <version.h>
#include <sha.h>

#define MODE_PACK             0
#define MODE_UNPACK           1
#define UBOOT_NUM             4
#define UBOOT_MAX_SIZE        1024*1024
#define U_BOOT_VERSION_STRING U_BOOT_VERSION " (" U_BOOT_DATE " - " \
    U_BOOT_TIME ")" CONFIG_IDENT_STRING

#define LOADER_MAGIC_SIZE     16
#define LOADER_HASH_SIZE      32
#define CMD_LINE_SIZE         512

#define RK_UBOOT_MAGIC        "LOADER  "
#define RK_UBOOT_SIGN_TAG     0x4E474953
#define RK_UBOOT_SIGN_LEN     128
typedef struct tag_second_loader_hdr
{
    uint8_t magic[LOADER_MAGIC_SIZE];  // "LOADER  "

    uint32_t loader_load_addr;           /* physical load addr ,default is 0x60000000*/
    uint32_t loader_load_size;           /* size in bytes */
    uint32_t crc32;                      /* crc32 */
    uint32_t hash_len;                   /* 20 or 32 , 0 is no hash*/
    uint8_t hash[LOADER_HASH_SIZE];     /* sha */

    uint8_t reserved[1024-32-32];
    uint32_t signTag; //0x4E474953
    uint32_t signlen; //128
    uint8_t rsaHash[256];
    uint8_t reserved2[2048-1024-256-8];
}second_loader_hdr;


uint32_t gTable_Crc32[256] =
{
    0x00000000, 0x04c10db7, 0x09821b6e, 0x0d4316d9,
    0x130436dc, 0x17c53b6b, 0x1a862db2, 0x1e472005,
    0x26086db8, 0x22c9600f, 0x2f8a76d6, 0x2b4b7b61,
    0x350c5b64, 0x31cd56d3, 0x3c8e400a, 0x384f4dbd,
    0x4c10db70, 0x48d1d6c7, 0x4592c01e, 0x4153cda9,
    0x5f14edac, 0x5bd5e01b, 0x5696f6c2, 0x5257fb75,
    0x6a18b6c8, 0x6ed9bb7f, 0x639aada6, 0x675ba011,
    0x791c8014, 0x7ddd8da3, 0x709e9b7a, 0x745f96cd,
    0x9821b6e0, 0x9ce0bb57, 0x91a3ad8e, 0x9562a039,
    0x8b25803c, 0x8fe48d8b, 0x82a79b52, 0x866696e5,
    0xbe29db58, 0xbae8d6ef, 0xb7abc036, 0xb36acd81,
    0xad2ded84, 0xa9ece033, 0xa4aff6ea, 0xa06efb5d,
    0xd4316d90, 0xd0f06027, 0xddb376fe, 0xd9727b49,
    0xc7355b4c, 0xc3f456fb, 0xceb74022, 0xca764d95,
    0xf2390028, 0xf6f80d9f, 0xfbbb1b46, 0xff7a16f1,
    0xe13d36f4, 0xe5fc3b43, 0xe8bf2d9a, 0xec7e202d,
    0x34826077, 0x30436dc0, 0x3d007b19, 0x39c176ae,
    0x278656ab, 0x23475b1c, 0x2e044dc5, 0x2ac54072,
    0x128a0dcf, 0x164b0078, 0x1b0816a1, 0x1fc91b16,
    0x018e3b13, 0x054f36a4, 0x080c207d, 0x0ccd2dca,
    0x7892bb07, 0x7c53b6b0, 0x7110a069, 0x75d1adde,
    0x6b968ddb, 0x6f57806c, 0x621496b5, 0x66d59b02,
    0x5e9ad6bf, 0x5a5bdb08, 0x5718cdd1, 0x53d9c066,
    0x4d9ee063, 0x495fedd4, 0x441cfb0d, 0x40ddf6ba,
    0xaca3d697, 0xa862db20, 0xa521cdf9, 0xa1e0c04e,
    0xbfa7e04b, 0xbb66edfc, 0xb625fb25, 0xb2e4f692,
    0x8aabbb2f, 0x8e6ab698, 0x8329a041, 0x87e8adf6,
    0x99af8df3, 0x9d6e8044, 0x902d969d, 0x94ec9b2a,
    0xe0b30de7, 0xe4720050, 0xe9311689, 0xedf01b3e,
    0xf3b73b3b, 0xf776368c, 0xfa352055, 0xfef42de2,
    0xc6bb605f, 0xc27a6de8, 0xcf397b31, 0xcbf87686,
    0xd5bf5683, 0xd17e5b34, 0xdc3d4ded, 0xd8fc405a,
    0x6904c0ee, 0x6dc5cd59, 0x6086db80, 0x6447d637,
    0x7a00f632, 0x7ec1fb85, 0x7382ed5c, 0x7743e0eb,
    0x4f0cad56, 0x4bcda0e1, 0x468eb638, 0x424fbb8f,
    0x5c089b8a, 0x58c9963d, 0x558a80e4, 0x514b8d53,
    0x25141b9e, 0x21d51629, 0x2c9600f0, 0x28570d47,
    0x36102d42, 0x32d120f5, 0x3f92362c, 0x3b533b9b,
    0x031c7626, 0x07dd7b91, 0x0a9e6d48, 0x0e5f60ff,
    0x101840fa, 0x14d94d4d, 0x199a5b94, 0x1d5b5623,
    0xf125760e, 0xf5e47bb9, 0xf8a76d60, 0xfc6660d7,
    0xe22140d2, 0xe6e04d65, 0xeba35bbc, 0xef62560b,
    0xd72d1bb6, 0xd3ec1601, 0xdeaf00d8, 0xda6e0d6f,
    0xc4292d6a, 0xc0e820dd, 0xcdab3604, 0xc96a3bb3,
    0xbd35ad7e, 0xb9f4a0c9, 0xb4b7b610, 0xb076bba7,
    0xae319ba2, 0xaaf09615, 0xa7b380cc, 0xa3728d7b,
    0x9b3dc0c6, 0x9ffccd71, 0x92bfdba8, 0x967ed61f,
    0x8839f61a, 0x8cf8fbad, 0x81bbed74, 0x857ae0c3,
    0x5d86a099, 0x5947ad2e, 0x5404bbf7, 0x50c5b640,
    0x4e829645, 0x4a439bf2, 0x47008d2b, 0x43c1809c,
    0x7b8ecd21, 0x7f4fc096, 0x720cd64f, 0x76cddbf8,
    0x688afbfd, 0x6c4bf64a, 0x6108e093, 0x65c9ed24,
    0x11967be9, 0x1557765e, 0x18146087, 0x1cd56d30,
    0x02924d35, 0x06534082, 0x0b10565b, 0x0fd15bec,
    0x379e1651, 0x335f1be6, 0x3e1c0d3f, 0x3add0088,
    0x249a208d, 0x205b2d3a, 0x2d183be3, 0x29d93654,
    0xc5a71679, 0xc1661bce, 0xcc250d17, 0xc8e400a0,
    0xd6a320a5, 0xd2622d12, 0xdf213bcb, 0xdbe0367c,
    0xe3af7bc1, 0xe76e7676, 0xea2d60af, 0xeeec6d18,
    0xf0ab4d1d, 0xf46a40aa, 0xf9295673, 0xfde85bc4,
    0x89b7cd09, 0x8d76c0be, 0x8035d667, 0x84f4dbd0,
    0x9ab3fbd5, 0x9e72f662, 0x9331e0bb, 0x97f0ed0c,
    0xafbfa0b1, 0xab7ead06, 0xa63dbbdf, 0xa2fcb668,
    0xbcbb966d, 0xb87a9bda, 0xb5398d03, 0xb1f880b4,
};

uint32_t crc32(uint8_t* pData, uint32_t ulSize) {
    uint32_t i;
    uint32_t nAccum = 0;
    for ( i=0; i<ulSize; i++) {
        nAccum = (nAccum<<8)^gTable_Crc32[(nAccum>>24)^(*pData++)];
    }
    return nAccum;
}

void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [--pack|--unpack] file\n", prog);
}

int main (int argc, char *argv[])
{
    int	mode, size, i;
    FILE	*fi, *fo;
    second_loader_hdr hdr;
    char *buf = 0;

    if (argc < 4) {
        usage(argv[0]);
        exit (EXIT_FAILURE);
    }

    if (!strcmp(argv[1], "--pack"))
        mode = MODE_PACK;
    else if (!strcmp(argv[1], "--unpack"))
        mode = MODE_UNPACK;
    else {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    fi = fopen(argv[2], "rb");
    if (!fi) {
        perror(argv[2]);
        exit (EXIT_FAILURE);
    }

    fo = fopen(argv[3], "wb");
    if (!fo) {
        perror(argv[3]);
        exit (EXIT_FAILURE);
    }

    buf = calloc(UBOOT_MAX_SIZE, UBOOT_NUM);
    if (!buf) {
        perror(argv[3]);
        exit (EXIT_FAILURE);
    }
    //memset(buf, 0, UBOOT_NUM*UBOOT_MAX_SIZE);
    if(mode == MODE_PACK){
        printf("pack input %s \n", argv[2]);
        fseek( fi, 0, SEEK_END );
        size = ftell(fi);
        fseek( fi, 0, SEEK_SET );
        printf("pack file size:%d \n", size);
        if(size > UBOOT_MAX_SIZE - sizeof(second_loader_hdr)){
            perror(argv[3]);
            exit (EXIT_FAILURE);
        }
        memset(&hdr, 0, sizeof(second_loader_hdr));
        strcpy(hdr.magic, RK_UBOOT_MAGIC);
        hdr.loader_load_addr = CONFIG_SYS_TEXT_BASE;
        hdr.loader_load_size = size;
        fread(buf + sizeof(second_loader_hdr), size, 1, fi);
        hdr.crc32 = crc32(buf + sizeof(second_loader_hdr), size);
        printf("crc = 0x%08x\n", hdr.crc32);

        SHA_CTX ctx;
        uint8_t* sha;
        hdr.hash_len = (SHA_DIGEST_SIZE > LOADER_HASH_SIZE) ? LOADER_HASH_SIZE : SHA_DIGEST_SIZE;
        SHA_init(&ctx);
        SHA_update(&ctx, buf + sizeof(second_loader_hdr), size);
        SHA_update(&ctx, &hdr.loader_load_addr, sizeof(hdr.loader_load_addr));
        SHA_update(&ctx, &hdr.loader_load_size, sizeof(hdr.loader_load_size));
        SHA_update(&ctx, &hdr.hash_len, sizeof(hdr.hash_len));
        sha = SHA_final(&ctx);
        memcpy(hdr.hash, sha, hdr.hash_len);

        printf("uboot version:%s\n",U_BOOT_VERSION_STRING);
        memcpy(buf, &hdr, sizeof(second_loader_hdr));
        for(i=0; i<UBOOT_NUM; i++){
            fwrite(buf, UBOOT_MAX_SIZE, 1, fo);
        }
        printf("pack %s success! \n", argv[3]);
    }else if (mode == MODE_UNPACK){
        printf("unpack input %s \n", argv[2]);
        memset(&hdr, 0, sizeof(second_loader_hdr));
        fread(&hdr, sizeof(second_loader_hdr), 1, fi);
        fread(buf, hdr.loader_load_size, 1, fi);
        fwrite(buf, hdr.loader_load_size, 1, fo);
        printf("unpack %s success! \n", argv[3]);
    }
    free(buf);
    fclose(fi);
    fclose(fo);

}

