/* See LICENSE file for copyright and license details. */
#include <string.h>
#include "util.h"

void string_replace(char *str, const char find, const char replace)
{
    char *pt = str;
    while (*pt) {
        if (*pt == find)
            *pt = replace;
        pt++;
    }
}

void string_remove(char *str, const char *rem)
{
    char        *pt     = str;
    const char  *rpt    = rem;
    int         length  = strlen(rem);

    while (*pt) {
        if (*pt == *rpt) {
            rpt++;
            if (!*rpt) {
                pt++;
                memmove(pt - length, pt, 1 + strlen(pt));
                rpt = rem;
                pt -= length;
                continue;
            }
        } else {
            rpt = rem;
        }
        pt++;
    }
}

void string_trim(char *str)
{
    char *pt = str;

    while (*pt == ' ' || *pt  == '\n')
        pt++;

    if (strlen(pt) == 0) {
        str[0] = '\0';
    } else {
        memmove(str, pt, 1 + strlen(pt));
        pt = str + strlen(str) - 1;

        while (*pt == ' ' || *pt  == '\n')
            pt--;

        memmove(pt + 1, str + strlen(str), strlen(pt + 1));
    }
}

void string_remove_redundent_spaces(char *str)
{
    char *dest = str;

    while (*str != '\0') {
        while ((*str == ' ' && *(str + 1) == ' ')
            || (*str == ' ' && *(str + 1) == '\n')
            || (*str == ' ' && *(str - 1) == '\n'))
            str++;
        *dest++ = *str++;
    }
    *dest = '\0';
}

void dictionary_replace(char *str, const Dictionary *dict, int size)
{
    const Dictionary    *entry  = dict;
    const Dictionary    *end    = dict + size;
    const char          *ent;
    const char          *rep;
    char                *ptr;
    int                 ent_len;
    int                 rep_len;

    while (entry < end) {
        ent     = entry->entity;
        rep     = entry->replacement;
        ent_len = strlen(ent);
        rep_len = strlen(rep);

        if (ent_len < rep_len) {
            entry++;
            continue;
        }

        ptr = str;
        while (*ptr) {
            if (*ptr == *ent++) {
                if (*ent == '\0') {
                    ptr -= (ent_len - 1);

                    while (*rep)
                        *ptr++ = *rep++;

                    rep -= rep_len;
                    memmove(
                        ptr,
                        ptr + ent_len - rep_len,
                        1 + strlen(ptr + ent_len - rep_len)
                    );
                    continue;
                }
            } else if (*ptr == entry->entity[0]) {
                ent = entry->entity + 1;
            } else {
                ent = entry->entity;
            }
            ptr++;
        }
        entry++;
    }
}

int tiny_pow(int x, int y)
{
    int z = x;

    if (y == 0)
        return 1;

    while (y-- > 1)
        z *= x;

    return z;
}

int hex_value(char c)
{
    if (c >= 0x30 && c <= 0x39)
        return c - 0x30;
    else if (c >= 0x41 && c <= 0x46)
        return c - 0x37;
    else if (c >= 0x61 && c <= 0x66)
        return c - 0x57;
    else
        return -1;
}

void ucs_to_utf8(char *ucs)
{
    unsigned    u       = 0;
    unsigned    c       = 0;
    int         length  = strlen(ucs);
    char        *pt     = ucs;

    while (*pt)
        u += hex_value(*pt++) * tiny_pow(16, length - 1 - c++);

    pt = ucs;

    if (u <= 0x7f) {
        *pt++ = u;
    } else if (u <= 0x7ff) {
        *pt++ = 0xc0 | (u >> 6);
        *pt++ = 0x80 | ((u >> 0) & 0x3f);
    } else if (u <= 0xffff) {
        *pt++ = 0xe0 | (u >> 12);
        *pt++ = 0x80 | ((u >> 6) & 0x3f);
        *pt++ = 0x80 | ((u >> 0) & 0x3f);
    } else if (u <= 0x1fffff) {
        *pt++ = 0xf0 | (u >> 18);
        *pt++ = 0x80 | ((u >> 12) & 0x3f);
        *pt++ = 0x80 | ((u >> 6) & 0x3f);
        *pt++ = 0x80 | ((u >> 0) & 0x3f);
    } else {
        *pt++ = 0x3f;
    }

    *pt = '\0';
}

void unicode_decode(char *str)
{
    char        buff[5];
    char        *pt         = str;
    char        *bpt        = buff;
    const char  indic[2]    = { '\\', 'u' };
    int         c           = 0;
    int         j           = 0;

    while (*pt) {
        if (c < 2 && *pt == indic[c]) {
            c++;
        } else if (c >= 2 && c < 6 && hex_value(*pt) != -1) {
            *bpt++ = *pt;
            *bpt = '\0';
            c++;
        } else if (c == 6) {
            ucs_to_utf8(buff);
            bpt = buff;
            pt -= c;
            j = 0;

            while (*bpt) {
                *pt++ = *bpt++;
                j++;
            }

            memmove(pt, pt + c - j, 1 + strlen(pt + c - j));

            bpt = buff;
            c = 0;
            continue;
        } else {
            bpt = buff;
            c = 0;
        }
        pt++;
    }
}
