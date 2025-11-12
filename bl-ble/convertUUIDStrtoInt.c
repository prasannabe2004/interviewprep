/*
 * uuid128.c — Parse 128-bit Bluetooth/BLE UUIDs into 128-bit integer
 * Uses native __uint128_t for maximum simplicity
 * Works for strings like: "550e8400-e29b-41d4-a716-446655440000"
 * Author: ChatGPT (OpenAI)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/* ---------- 128-bit Type Definition ---------- */

typedef __uint128_t uuid128_t;

/* ---------- Internal Helpers ---------- */

/* Convert single hex char -> nibble */
static unsigned char hex_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    if (c >= 'a' && c <= 'f')
        return (unsigned char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F')
        return (unsigned char)(c - 'A' + 10);
    return 0xFF;
}

/* ---------- UUID Parser ---------- */

/*
 * parse_uuid128:
 *   Parses a 128-bit Bluetooth UUID string into a 128-bit uuid128_t.
 *
 * Args:
 *   str  : 128-bit UUID string, may contain dashes/spaces
 *   out  : pointer to uuid128_t to fill
 *
 * Returns:
 *   0 on success
 *  -1 on invalid input (not a valid 128-bit UUID)
 */
int parse_uuid128(const char *str, uuid128_t *out)
{
    if (!str || !out)
        return -1;

    /* Collect only hex digits */
    char hex[33];
    int hlen = 0;
    for (const char *p = str; *p && hlen < 32; ++p)
    {
        if (isxdigit((unsigned char)*p))
            hex[hlen++] = *p;
    }
    hex[hlen] = '\0';

    /* Only accept 128-bit UUIDs (32 hex digits) */
    if (hlen != 32)
    {
        return -1;
    }

    /* Parse hex string into 128-bit integer */
    *out = 0;
    for (int i = 0; i < 32; ++i)
    {
        unsigned char nib = hex_nibble(hex[i]);
        if (nib == 0xFF)
            return -1;
        *out = (*out << 4) | (uuid128_t)nib;
    }
    return 0;
}

/* ---------- Debug/Utility ---------- */

/* Print uuid128_t as continuous lowercase hex (no dashes) */
void print_uuid128(const uuid128_t *u)
{
    if (!u)
        return;
    const char *digits = "0123456789abcdef";
    
    /* Extract bytes from 128-bit integer */
    unsigned char bytes[16];
    uuid128_t temp = *u;
    for (int i = 15; i >= 0; --i)
    {
        bytes[i] = (unsigned char)(temp & 0xFF);
        temp >>= 8;
    }

    for (int i = 0; i < 16; ++i)
    {
        putchar(digits[(bytes[i] >> 4) & 0xF]);
        putchar(digits[bytes[i] & 0xF]);
    }
}

int main(void)
{
    printf("Using native 128-bit integers (sizeof = %zu bytes)\n\n", sizeof(uuid128_t));
    
    const char *tests[] = {
        "550e8400-e29b-41d4-a716-446655440000", // 128-bit UUID
        "12345678-1234-5678-9abc-def012345678", // 128-bit UUID
        "00000000-0000-1000-8000-00805f9b34fb", // Bluetooth Base UUID
        " 550e8400 - e29b - 41d4 - a716 - 446655440000 ", // With spaces
        "180D",                                 // Invalid: too short
        "12345678",                             // Invalid: too short
        NULL};

    for (int i = 0; tests[i]; ++i)
    {
        uuid128_t u;
        if (parse_uuid128(tests[i], &u) == 0)
        {
            printf("Input: %-40s -> ", tests[i]);
            print_uuid128(&u);
            printf("\n");
        }
        else
        {
            printf("Input: %-40s -> parse error\n", tests[i]);
        }
    }
    return 0;
}
