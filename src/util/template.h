/*
 * template.h
 *
 *  Created on: Sep 5, 2017
 *      Author: harper
 */

#ifndef SRC_UTIL_TEMPLATE_H_
#define SRC_UTIL_TEMPLATE_H_

#include <immintrin.h>

int mm_extract_epi32(__m128i data, int index);
__m128i mm_align_epi8(__m128i a, __m128i b, int offset);
__m256d mm256_blend_pd(__m256d a, __m256d b, int imm);

#endif /* SRC_UTIL_TEMPLATE_H_ */