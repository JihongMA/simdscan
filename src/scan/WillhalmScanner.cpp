/*
 * WillhalmScanner.cpp
 *
 *  Created on: Aug 25, 2017
 *      Author: harper
 */

#include <immintrin.h>
#include "WillhalmScanner.h"
#include <assert.h>

#define INT_LEN 32
#define SIMD_LEN 256

int WillhalmScanner::computeShift(int counter) {
	return 0;
}

WillhalmScanner::WillhalmScanner(int bitSize) {
	this->bitSize = bitSize;
}

WillhalmScanner::~WillhalmScanner() {
	// TODO Auto-generated destructor stub
}

void WillhalmScanner::scan(int* data, int length, int* dest, Predicate* p) {
	int counter = 0;
	int step = SIMD_LEN / INT_LEN;
	// TODO For experimental purpose, assume data is aligned for now
	assert(length % step == 0);

	__m256i prev;
	__m256i current;
	while (counter < length) {
		current = _mm256_stream_load_si256((__m256i *) (data + counter));
		// Shift data
		int shift = computeShift(counter);
		if (shift != 0) {
			current = _mm256_alignr_epi8(prev, current, shift);
		}
		// Use shuffle to make 4-byte alignment

		_mm256_permutevar8x32_epi32(current,)


		// Use shuffle to make bit-alignment

		counter += step;
		// Store last data in prev
		prev = current;
	}
}
