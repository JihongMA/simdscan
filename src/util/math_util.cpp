/*
 * math.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: harper
 */

#include <immintrin.h>
#include <stdint.h>
#include "template.h"
#include "math_util.h"

const __m128i hmask128 = _mm_setr_epi32(0, 0x80000000, 0, 0x80000000);
const __m128i zero128 = _mm_set1_epi32(0);
const __m128i lmask128 = _mm_setr_epi32(-1, 0x7fffffff, -1, 0x7fffffff);

const __m256i hmask256 = _mm256_setr_epi32(0, 0x80000000, 0, 0x80000000, 0,
                                           0x80000000, 0, 0x80000000);
const __m256i zero256 = _mm256_set1_epi32(0);
const __m256i lmask256 = _mm256_setr_epi32(-1, 0x7fffffff, -1, 0x7fffffff, -1,
                                           0x7fffffff, -1, 0x7fffffff);
const __m128i carry128 = _mm_setr_epi32(0, 0, 1, 0);

const __m256i carry256 = _mm256_setr_epi64x(0, 1, 1, 1);

__m128i mm_cmpgt_epu64(__m128i a, __m128i b) {
    __m128i ah = _mm_and_si128(a, hmask128);
    __m128i al = _mm_and_si128(a, lmask128);
    __m128i bh = _mm_and_si128(b, hmask128);
    __m128i bl = _mm_and_si128(b, lmask128);

    __m128i ahebh = _mm_cmpeq_epi64(ah, bh);
    __m128i algbl = _mm_cmpgt_epi64(al, bl);
    __m128i ahgbh = _mm_and_si128(_mm_cmpeq_epi64(ah, hmask128),
                                  _mm_cmpeq_epi64(bh, zero128));
    return _mm_or_si128(_mm_and_si128(ahebh, algbl), ahgbh);
}

__m256i mm256_cmpgt_epu64(__m256i a, __m256i b) {
    __m256i ah = _mm256_and_si256(a, hmask256);
    __m256i al = _mm256_and_si256(a, lmask256);
    __m256i bh = _mm256_and_si256(b, hmask256);
    __m256i bl = _mm256_and_si256(b, lmask256);

    __m256i ahebh = _mm256_cmpeq_epi64(ah, bh);
    __m256i algbl = _mm256_cmpgt_epi64(al, bl);
    __m256i ahgbh = _mm256_and_si256(_mm256_cmpeq_epi64(ah, hmask256),
                                     _mm256_cmpeq_epi64(bh, zero256));
    return _mm256_or_si256(_mm256_and_si256(ahebh, algbl), ahgbh);
}

__m128i mm_add_epi128_1(__m128i a, __m128i b) {
    __m128i result = _mm_add_epi64(a, b);
    __m128i carry = mm_cmpgt_epu64(a, result);
    carry = _mm_and_si128(carry,
                          _mm_setr_epi64(_mm_set_pi64x(1), _mm_set_pi64x(0)));
    carry = (__m128i) _mm_permute_pd(carry, 1);
    result = _mm_add_epi64(result, carry);
    return result;
}

__m128i mm_add_epi128_2(__m128i a, __m128i b) {
    __m128i result = _mm_add_epi64(a, b);
    __m128i result1 = _mm_add_epi64(result, carry128);
    __m128i carry = mm_cmpgt_epu64(a, result);
    carry = (__m128i) _mm_permute_pd(carry, 1);
    result = _mm_blendv_epi8(result, result1, carry);
    return result;
}

const int BLEND_TABLE_128[] = {0, 2};

__m128i mm_add_epi128_3(__m128i a, __m128i b) {
    __m128i result = _mm_add_epi64(a, b);
    __m128i result1 = _mm_add_epi64(result, carry128);
    __m128i carry = mm_cmpgt_epu64(a, result);
    int cbit = (_mm_extract_epi32(carry, 1) & 1);
    int blend = BLEND_TABLE_128[cbit];
    return (__m128i) mm_blend_pd((__m128d) result, (__m128d) result1, blend);
}

__m128i mm_add_epi128(__m128i a, __m128i b) {
    __m128i result = _mm_add_epi64(a, b);
    uint64_t r0 = (uint64_t) _mm_extract_epi64(result, 0);
    uint64_t a0 = (uint64_t) _mm_extract_epi64(a, 0);

    return (a0 > r0) ?
           _mm_add_epi64(result, carry128) : result;
}

__m128i mm_sub_epi128_1(__m128i a, __m128i b) {
    __m128i result = _mm_sub_epi64(a, b);
    __m128i carry = mm_cmpgt_epu64(result, a);
    carry = _mm_and_si128(carry,
                          _mm_setr_epi64(_mm_set_pi64x(1), _mm_set_pi64x(0)));
    carry = (__m128i) _mm_permute_pd(carry, 1);
    result = _mm_sub_epi64(result, carry);
    return result;
}

__m128i mm_sub_epi128(__m128i a, __m128i b) {
    __m128i result = _mm_sub_epi64(a, b);
    uint64_t r0 = (uint64_t) _mm_extract_epi64(result, 0);
    uint64_t a0 = (uint64_t) _mm_extract_epi64(a, 0);
    return (r0 > a0) ?
           _mm_sub_epi64(result, carry128) : result;
}

const int BLEND_ADD_256[] = {0, 2, 4, 2, 8, 10, 4, 10, 0, 6, 4, 6, 8, 6, 4,
                             6, 0, 2, 12, 2, 8, 10, 12, 10, 0, 14, 12, 14, 8, 14, 12, 14};

__m256i mm256_add_epi256(__m256i a, __m256i b) {
    __m256i result = _mm256_add_epi64(a, b);
    __m256i result1 = _mm256_add_epi64(result, carry256);

    uint64_t a2 = _mm256_extract_epi64(a, 2);
    uint64_t a3 = _mm256_extract_epi64(a, 1);
    uint64_t a4 = _mm256_extract_epi64(a, 0);

    int c21 = a2 > (uint64_t) _mm256_extract_epi64(result1, 2);
    int c31 = a3 > (uint64_t) _mm256_extract_epi64(result1, 1);
    int c2 = a2 > (uint64_t) _mm256_extract_epi64(result, 2);
    int c3 = a3 > (uint64_t) _mm256_extract_epi64(result, 1);
    int c4 = a4 > (uint64_t) _mm256_extract_epi64(result, 0);
    // The sequence is c2^1, c3^1, c2, c3, c4
    int blendIdx = c21 << 4 | c31 << 3 | c2 << 2 | c3 << 1 | c4;
    int blend = BLEND_ADD_256[blendIdx];
    return (__m256i) mm256_blend_pd((__m256d) result, (__m256d) result1,
                                    blend);
}

__m256i mm256_add_epi256_2(__m256i a, __m256i b) {
    __m256i result = _mm256_add_epi64(a, b);
    __m256i result1 = _mm256_add_epi64(result, carry256);

    __m256i carry = mm256_cmpgt_epu64(a, result);
    __m256i carry1 = mm256_cmpgt_epu64(a, result1);

    // The sequence is c2^1, c3^1, c2, c3, c4
    __m256i maskedc = _mm256_and_si256(carry,
                                       _mm256_setr_epi64x(0x0101010101010101, 0x0202020202020202,
                                                          0x0404040404040404, 0));
    __m256i maskedc1 = _mm256_and_si256(carry1,
                                        _mm256_setr_epi64x(0, 0x0808080808080808, 0x1010101010101010, 0));
    __m256i cs = _mm256_add_epi64(maskedc, maskedc1);

    // Move data into 128 lane
    __m256i inlane = _mm256_permutevar8x32_epi32(cs,
                                                 _mm256_setr_epi32(0, 2, 4, 6, 6, 6, 6, 6));
    __m256i shuffle = _mm256_shuffle_epi8(inlane,
                                          _mm256_setr_epi64x(0xffffffffff080400, -1, -1, -1));

    int blendIdx = _mm256_extract_epi32(shuffle, 0);
    blendIdx = (blendIdx | (blendIdx >> 8) | (blendIdx >> 16)) & 0xff;
    int blend = BLEND_ADD_256[blendIdx];
    return (__m256i) mm256_blend_pd((__m256d) result, (__m256d) result1,
                                    blend);
}

__m256i mm256_add_epi256_1(__m256i a, __m256i b) {
    __m256i result = _mm256_add_epi64(a, b);
    __m256i result1 = _mm256_add_epi64(result, carry256);

    __m256i carry = mm256_cmpgt_epu64(a, result);
    __m256i carry1 = mm256_cmpgt_epu64(a, result1);

    // The sequence is c2^1, c3^1, c2, c3, c4
    int c21 = _mm256_extract_epi32(carry1, 4);
    int c31 = _mm256_extract_epi32(carry1, 2);
    int c2 = _mm256_extract_epi32(carry, 4);
    int c3 = _mm256_extract_epi32(carry, 2);
    int c4 = _mm256_extract_epi32(carry, 0);

    int blendIdx = (c21 & 16) | (c31 & 8) | (c2 & 4) | (c3 & 2) | (c4 & 1);

    int blend = BLEND_ADD_256[blendIdx];
    return (__m256i) mm256_blend_pd((__m256d) result, (__m256d) result1,
                                    blend);
}

const int BLEND_SUB_256[] = {0, 2, 4, 2, 8, 10, 4, 10, 0, 6, 4, 6, 8, 6, 4, 6, 0, 2, 12, 2, 8, 10, 12, 10, 0, 14, 12,
                             14, 8, 14, 12, 14};

__m256i mm256_sub_epi256(__m256i a, __m256i b) {
    __m256i result = _mm256_sub_epi64(a, b);
    __m256i result1 = _mm256_sub_epi64(result, carry256);

    uint64_t a0 = _mm256_extract_epi64(a, 0);
    uint64_t a1 = _mm256_extract_epi64(a, 1);
    uint64_t a2 = _mm256_extract_epi64(a, 2);

    int c2 = ((uint64_t) _mm256_extract_epi64(result, 2)) > a2;
    int c1 = ((uint64_t) _mm256_extract_epi64(result, 1)) > a1;
    int c0 = ((uint64_t) _mm256_extract_epi64(result, 0)) > a0;
    int c11 = ((uint64_t) _mm256_extract_epi64(result1, 1)) > a1;
    int c21 = ((uint64_t) _mm256_extract_epi64(result1, 2)) > a2;

    int blendIdx = c21 << 4 | c11 << 3 | c2 << 2 | c1 << 1 | c0;
    int blend = BLEND_SUB_256[blendIdx];
    return (__m256i) mm256_blend_pd((__m256d) result, (__m256d) result1,
                                    blend);
}