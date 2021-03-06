//
// Created by harper on 3/20/18.
//

#include <cstdio>
#include "WillhalmUnpackerScanner.h"
#include "../../util/unpack/u256/Small16Unpacker.h"
#include "../../util/unpack/u256/TrivialUnpacker.h"
#include "../../util/unpack/u256/Large16Unpacker.h"
#include "../../util/unpack/u256/Large32Unpacker.h"
#include "../../util/unpack/u256/Small32Unpacker.h"

WillhalmUnpackerScanner::WillhalmUnpackerScanner(int es) {
    this->entrySize = es;
    if (es > 25 && es < 32) {
        unpacker = new Large32Unpacker(es);
    } else if (es == 32) {
        unpacker = new TrivialUnpacker();
    } else {
        unpacker = new Small32Unpacker(es);
    }
}

WillhalmUnpackerScanner::~WillhalmUnpackerScanner() {
    delete unpacker;
}


void WillhalmUnpackerScanner::scan(int *input, uint64_t length, int *output, Predicate *p) {

    uint8_t *bytein = (uint8_t *) input;

    __m256i a32 = _mm256_set1_epi32(p->getVal1());
    __m256i a16 = _mm256_set1_epi16(p->getVal1());

    __mmask8 *mask32out = (__mmask8 *) output;

    uint64_t byteOffset = 0;
    uint8_t offset = 0;
    uint64_t numEntryDone = 0;
    uint64_t outputCounter = 0;

    switch (p->getOpr()) {
        case opr_eq:
        case opr_neq:

            while (numEntryDone < length) {
                __m256i current = unpacker->unpack(bytein + byteOffset, offset);

                __mmask8 result = _mm256_cmpeq_epi32_mask(current, a32);
                mask32out[outputCounter++] = result;

                numEntryDone += 8;
                uint64_t bitAdvance = offset + 8 * entrySize;
                byteOffset += bitAdvance / 8;
                offset = bitAdvance % 8;
            }
            break;
        case opr_less:
            while (numEntryDone < length) {
                __m256i current = unpacker->unpack(bytein + byteOffset, offset);

                __mmask8 lower = _mm256_cmp_epi32_mask(current, a32, _MM_CMPINT_LT);

                mask32out[outputCounter++] = lower;

                numEntryDone += 8;
                uint64_t bitAdvance = offset + 8 * entrySize;
                byteOffset += bitAdvance / 8;
                offset = bitAdvance % 8;
            }
            break;
    }
}