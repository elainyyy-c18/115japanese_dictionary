#include "../include/utf8.h"
#include <string.h>

uint32_t utf8_decode(const uint8_t** p, const uint8_t* end) 
{
    if (!p || !*p || *p >= end) return 0xFFFDu;

    uint8_t lead = **p;
    int len  = utf8_char_len(lead);

    if (len == 0 || *p + len > end) 
    {
        (*p)++; 
        return 0xFFFDu;
    }

    static const uint8_t MASK[] = {0, 0x7Fu, 0x1Fu, 0x0Fu, 0x07u};
    uint32_t cp = lead & MASK[len];
    (*p)++;

    for (int i = 1; i < len; i++) 
    {
        uint8_t cont = **p;
        if ((cont & 0xC0u) != 0x80u) return 0xFFFDu;
        cp = (cp << 6) | (cont & 0x3Fu);
        (*p)++;
    }
    return cp;
}

int utf8_encode(uint32_t cp, uint8_t* buf) 
{
    if (cp < 0x80u) 
    {
        buf[0] = (uint8_t)cp;
        return 1;
    }
    else if (cp < 0x800u) 
    {
        buf[0] = (uint8_t)(0xC0u | (cp >> 6));
        buf[1] = (uint8_t)(0x80u | (cp & 0x3Fu));
        return 2;
    }
    else if (cp < 0x10000u) 
    {
        buf[0] = (uint8_t)(0xE0u | (cp >> 12));
        buf[1] = (uint8_t)(0x80u | ((cp >> 6) & 0x3Fu));
        buf[2] = (uint8_t)(0x80u | (cp & 0x3Fu));
        return 3;
    }
    else if (cp < 0x110000u) 
    {
        buf[0] = (uint8_t)(0xF0u | (cp >> 18));
        buf[1] = (uint8_t)(0x80u | ((cp >> 12) & 0x3Fu));
        buf[2] = (uint8_t)(0x80u | ((cp >> 6) & 0x3Fu));
        buf[3] = (uint8_t)(0x80u | (cp & 0x3Fu));
        return 4;
    }
    return 0;
}

size_t utf8_cp_count(const uint8_t* s, size_t byte_len) 
{
    size_t count = 0;
    const uint8_t* end = s + byte_len;
    while (s < end) 
    {
        int n = utf8_char_len(*s);
        s += (n > 0) ? (size_t)n : 1u;
        count++;
    }
    return count;
}

size_t utf8_common_prefix_bytes_n(const uint8_t* a, size_t la, const uint8_t* b, size_t lb) 
{
    size_t i = 0, lim = (la < lb) ? la : lb;
    while (i < lim && a[i] == b[i]) i++;
    while (i > 0 && (a[i] & 0xC0u) == 0x80u) i--;
    return i;
}

size_t utf8_common_prefix_bytes(const uint8_t* a, const uint8_t* b) 
{
    size_t i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    while (i > 0 && (a[i] & 0xC0u) == 0x80u) i--;
    return i;
}