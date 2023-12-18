// SPDX-FileCopyrightText: 2023 Mete Balci
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal5.h"
#include "bsp.h"

#if defined(CAVP_TEST_SHA1)

#include "cavp-secure-hash/SHA1ShortMsg.rsp.h"
#include "cavp-secure-hash/SHA1LongMsg.rsp.h"

#elif defined(CAVP_TEST_SHA256)

#include "cavp-secure-hash/SHA256ShortMsg.rsp.h"
#include "cavp-secure-hash/SHA256LongMsg.rsp.h"

#elif defined(CAVP_TEST_SHA512)

#include "cavp-secure-hash/SHA512ShortMsg.rsp.h"
#include "cavp-secure-hash/SHA512LongMsg.rsp.h"

#else

#error "unknown CAVP_TEST"

#endif

static void str2bytes(const char* in, uint8_t* out, uint32_t outlen)
{
    if (*in == '\0') return;

    uint32_t offset = 0;

    while (true)
    {
        assert (offset < outlen);
        out[offset] = *in;
        offset++;
        in++;
        if (*in == '\0') return;
    }
}

static uint8_t hexchar2nibble(const char c)
{
    if (c < 48) assert (false);
    else if (c <= 57)
    {
        // 0-9
        return (c - 48);
    }
    else if (c < 65) assert (false);
    else if (c <= 70) 
    {
        // A-F
        return (c - 65) + 10;
    }
    else if (c < 97) assert (false);
    else if (c <= 102)
    {
        // a-f
        return (c - 97) + 10;
    }
    else assert (false);
}

static uint32_t hexstr2bytes(const char* in, uint8_t* out, uint32_t len)
{
    if (*in == '\0') return 0;

    uint32_t offset = 0;

    while (true)
    {
        assert (offset < len);

        out[offset] = hexchar2nibble(*in);
        in++;
        if (*in == '\0') return offset+1;

        out[offset] = (out[offset] << 4) | hexchar2nibble(*in);
        in++;
        if (*in == '\0') return offset+1;

        offset++;
    }

    return offset;
}

static bool cmpbytes(uint8_t* i1, uint8_t* i2, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        if (i1[i] != i2[i]) return false;
    }
    return true;
}

static void print_bytes(
        const char* label,
        uint8_t* buf, 
        uint32_t len)
{
    printf("%s", label);
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02X", buf[i]);
    }
    printf("\n");
}

void cavp_secure_hash_test(
        hal5_hash_algorithm_t algorithm, 
        const char** rsp)
{
    while (*rsp != NULL)
    {
        const char* input = *rsp;
        uint32_t input_len = strlen(input)/2;
        rsp++;

        const char* expected = *rsp;
        rsp++;

        uint8_t* input_buf = (uint8_t*) malloc(input_len);
        hexstr2bytes(input, input_buf, input_len);

        uint8_t correct_digest[64];
        hexstr2bytes(expected, correct_digest, 64);

        hal5_hash_init_for_hash(algorithm);

        for (uint32_t i = 0; i < input_len; i += 4)
        {
            hal5_hash_update(input_buf, i, input_len);
        }

        hal5_hash_finalize();

        uint8_t* calculated_digest = hal5_hash_get_digest();

        if (cmpbytes(
                    calculated_digest, 
                    correct_digest, 
                    hal5_hash_get_digest_size(algorithm)))
        {
            printf(".");
            fflush(stdout);
        }
        else 
        {
            printf("Len:%lu test failed\n", input_len);
            print_bytes("input: ", input_buf, input_len);
            print_bytes("cor_digest: ", correct_digest, 20);
            print_bytes("cal_digest: ", calculated_digest, 20);

            assert (false);
        }

        free(input_buf);
    }
    printf("\n");
}

void cavp_secure_hash_tests()
{
#if defined(CAVP_TEST_SHA1)

    printf("SHA1 short tests:\n");

    cavp_secure_hash_test(
            hal5_hash_sha1, 
            sha1shortmsg_rsp);

    printf("SHA1 long tests:\n");

    cavp_secure_hash_test(
            hal5_hash_sha1, 
            sha1longmsg_rsp);

#elif defined(CAVP_TEST_SHA256)

    printf("SHA256 short tests:\n");

    cavp_secure_hash_test(
            hal5_hash_sha2_256,
            sha256shortmsg_rsp);

    printf("SHA256 long tests:\n");

    cavp_secure_hash_test(
            hal5_hash_sha2_256, 
            sha256longmsg_rsp);

#elif defined(CAVP_TEST_SHA512)

    printf("SHA512 short tests:\n");

    cavp_secure_hash_test(
            hal5_hash_sha2_512,
            sha512shortmsg_rsp);

    printf("SHA512 long tests:\n");

    cavp_secure_hash_test(
            hal5_hash_sha2_512, 
            sha512longmsg_rsp);
#endif
}

void boot(void) 
{
    hal5_rcc_initialize();
    hal5_console_configure(921600, false);
    hal5_console_clearscreen();
    hal5_console_boot_colors();
    hal5_console_dump_info();

    printf("Booting...\n");

    hal5_icache_enable();
    hal5_flash_enable_prefetch();
    bsp_configure(NULL);
    uint32_t divm, muln, divp;
    hal5_change_sys_ck_to_pll1_p(240000000, &divm, &muln, &divp);

    hal5_hash_enable();

    bsp_boot_completed();
    printf("Boot completed.\n");

    hal5_console_normal_colors();
}

int main(void) 
{
    boot();

    while (1) 
    {
        cavp_secure_hash_tests();
        bsp_heartbeat();
    }

    // you shall not return
    assert (false);

    return 0;
}
