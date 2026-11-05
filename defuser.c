#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t lo = (uint32_t)challenge;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    uint32_t a = (UINT32_C(3) * hi)
               ^ (lo + UINT32_C(0xff82));
    uint32_t b = (mid + UINT32_C(0x7bec)) ^ lo;

    values[0] = (UINT32_C(3) * b + a) ^ UINT32_C(0x5384);

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = ((uint32_t)(challenge >> 9) * UINT32_C(2)
                 & UINT32_C(0x3e)) + UINT32_C(5);

    for (unsigned int i = 1; i < 6; ++i) {
        uint32_t part =
            (uint32_t)((challenge >> (16 + i)) & UINT64_C(0x3f));

        values[i] = (UINT32_C(3) * values[i - 1]) ^ (part + i);
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index = (uint32_t)challenge & UINT32_C(7);
    uint32_t x = (uint32_t)(challenge >> 3)
               ^ (uint32_t)(challenge >> 32);
    uint32_t result;

    switch (index) {
    case 0:
        result = UINT32_C(5) * x + UINT32_C(0xca0a);
        break;
    case 1:
        result = UINT32_C(3) * x + UINT32_C(0xf59a);
        break;
    case 2:
        result = UINT32_C(5) * x + UINT32_C(0x1b503);
        break;
    case 3:
        result = (x ^ UINT32_C(0x4070)) + (x >> 1);
        break;
    case 4:
        result = UINT32_C(5) * x + UINT32_C(0x5a9b);
        break;
    case 5:
        result = (x ^ UINT32_C(0xdf52)) + (x >> 2);
        break;
    case 6:
        result = (UINT32_C(2) * x) ^ UINT32_C(0xe6e0);
        break;
    case 7:
        result = (UINT32_C(4) * x) ^ UINT32_C(0x27a7);
        break;
    default:
        return 1;
    }

    values[0] = index;
    values[1] = result;
    return write_answer(out, cap, values, count);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low,
                            uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + UINT32_C(0x98)) ^ key;
    }

    uint32_t child;
    if (target < mid) {
        child = phase4_walk(target, low, mid - 1, key);
        return (UINT32_C(3) * child)
             ^ (mid + key) ^ UINT32_C(0x0a);
    }

    child = phase4_walk(target, mid + 1, high, key);
    return (UINT32_C(3) * child)
         ^ (mid + key) ^ UINT32_C(0x06);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)challenge & UINT32_C(0x7f);
    uint32_t key = (uint32_t)(challenge >> 7)
                 ^ (uint32_t)(challenge >> 32);

    values[0] = phase4_walk(target, 0, 127, key);

    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    /* Nodes ordered by address: 0x405ba0, 0x405bb0, ... */
    static const int next[12] = {
        3, 9, 11, 8, -1, 7, 2, 0, 6, 5, 4, 10
    };
    static const uint32_t ids[12] = {
        0x339, 0x932, 0xb59, 0x72e,
        0x045, 0x63f, 0x12b, 0x83b,
        0xa15, 0x219, 0x435, 0x577
    };
    static const uint32_t data[12] = {
        0x131f, 0x2753, 0x354b, 0x380e,
        0x5e7a, 0x2cba, 0xe7cf, 0x3254,
        0xef79, 0x1ff9, 0xddd9, 0x7aa7
    };
    static const int heads[8] = {
        1, 9, 5, 7, 0, 3, 8, 6
    };

    int node = heads[challenge & UINT64_C(7)];
    unsigned int steps =
        (unsigned int)((challenge >> 3) & UINT64_C(3)) + 1;

    uint32_t acc = (uint32_t)(challenge >> 6)
                 ^ (uint32_t)(challenge >> 32)
                 ^ UINT32_C(0xa5d7);
    uint32_t last_id = 0;

    for (unsigned int i = 0; i < steps; ++i) {
        if (node < 0 || node >= 12) {
            return 1;
        }
        last_id = ids[node];
        acc = ((acc + data[node]) * UINT32_C(5)) ^ last_id;
        node = next[node];
    }

    values[0] = acc;
    values[1] = last_id;
    return write_answer(out, cap, values, count);
}

static uint32_t phase6_apply(uint32_t index, uint32_t x) {
    /* Table mapping: 3, 4, 0, 5, 1, 0, 4, 2 */
    switch (index) {
    case 0:
        return (x ^ UINT32_C(0x9735)) + UINT32_C(4) * x;
    case 1:
    case 6:
        return ((x >> 3) ^ x) + UINT32_C(0x77db);
    case 2:
    case 5:
        return ((x << 3) | (x >> 29)) ^ UINT32_C(0xad4e);
    case 3:
        return (UINT32_C(4) * x + UINT32_C(0x6a94))
             ^ (UINT32_C(5) * x);
    case 4:
        return (x >> 2) | (x << 30);
    case 7:
        return (x + UINT32_C(0xf7d9)) ^ (x >> 3);
    default:
        return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);
    uint32_t first = (uint32_t)challenge & UINT32_C(7);
    uint32_t second = (uint32_t)(challenge >> 3) & UINT32_C(7);

    uint32_t a = phase6_apply(first, x);
    uint32_t b = phase6_apply(second, a ^ UINT32_C(0x8073));
    values[0] = b + UINT32_C(0x5d66);

    return write_answer(out, cap, values, count);
}
int solve_secret(uint64_t challenge, uint64_t context,
                 char *out, size_t cap) {
    uint32_t a = (uint32_t)challenge
               ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context
               ^ (uint32_t)(context >> 32);
    uint32_t first = (uint32_t)context & UINT32_C(3);

    uint32_t x = a ^ b;
    uint32_t mixed = ((x >> 7) ^ UINT32_C(0xc9df))
                   + (x ^ UINT32_C(0xbdb2));

    uint32_t second = mixed
                    ^ ((first * UINT32_C(0xbdb2))
                       ^ (uint32_t)(context >> 16));

    uint32_t third = second
                   ^ (b + first + UINT32_C(0xc9df) + (a >> 7));

    int size = snprintf(
        out, cap,
        "AAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAA"
        "OPEN"
        "%" PRIu32 " %" PRIu32 " %" PRIu32,
        first, second, third
    );

    return size < 0 || (size_t)size >= cap;
}
