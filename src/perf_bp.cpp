#include <immintrin.h>
#include <random>
#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <ostream>
#include "scan/Scanner.h"
#include "util/encode.h"
#include "scan/bitpack/WillhalmScanner128.h"
#include "scan/bitpack/HaoScanner128.h"
#include "scan/bitpack/HaoScanner256.h"
#include "scan/bitpack/HaoScanner512.h"
#include "scan/bitpack/BitWeaverHScanner128.h"
#include "scan/bitpack/BitWeaverHScanner256.h"
#include "scan/bitpack/BitWeaverHScanner512.h"
#include "scan/delta/TrivialDeltaScanner.h"
#include "scan/delta/SimdDeltaScanner32.h"
#include "scan/delta/SimdDeltaScanner16.h"
#include "scan/bitpack/TrivialBPScanner.h"
#include "scan/bitpack/WillhalmScanner256.h"
#include "scan/bitpack/WillhalmUnpackerScanner.h"

//#pragma GCC push_options
//#pragma GCC optimize ("O0")

int bp_throughput(Scanner *scanner, uint64_t num, int entrySize, int *input, int *encoded, int *output) {
    // Prepare random numbers
    int max = (1 << entrySize) - 1;

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, max); // distribution in range [1, 6]

    for (int i = 0; i < num; i++) {
        input[i] = dist(rng);
    }

    encode(input, encoded, num, entrySize);

    // Large enough

    auto x = dist(rng);
//    Predicate p(opr_in, x / 2, x);
    Predicate p(opr_eq, x, 0);

//    struct timeval tp;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */

//    gettimeofday(&tp, NULL);
//    long start, elapse;
//    start = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    scanner->scan(encoded, num, output, &p);

//    gettimeofday(&tp, NULL);
//    elapse = tp.tv_sec * 1000 + tp.tv_usec / 1000 - start;

    clock_gettime(CLOCK_MONOTONIC, &end);	/* mark the end time */
    uint64_t elapse = (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)*1000000;

    return num / elapse;
}

int bwh_throughput(Scanner *scanner, uint64_t num, int entrySize, int *input, int *encoded, int *output) {
    // Prepare random numbers
    int max = (1 << entrySize) - 1;

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, max); // distribution in range [1, 6]

    for (int i = 0; i < num; i++) {
        input[i] = dist(rng);
    }

    bitweaverh_encode(input, encoded, num, entrySize);

    // Large enough

    auto x = dist(rng);
//    Predicate p(opr_in, x / 2, x);
    Predicate p(opr_eq, x, 0);

//    struct timeval tp;

//    gettimeofday(&tp, NULL);
//    long start, elapse;
//    start = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */

    scanner->scan(encoded, num, output, &p);

//    printf("%d\n", output[123214]);
//    gettimeofday(&tp, NULL);
//    elapse = tp.tv_sec * 1000 + tp.tv_usec / 1000 - start;
    clock_gettime(CLOCK_MONOTONIC, &end);	/* mark the end time */
    uint64_t elapse = (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)*1000000;

    return num / elapse;
}
//#pragma GCC pop_options
int main(int argc, char **argv) {
    uint64_t num = 100000000;

    int *bp_input = new int[num];
    int *bp_encoded = (int *) aligned_alloc(64, sizeof(int) * (2 * num));
    int *bp_output = (int *) aligned_alloc(64, sizeof(int) * (2 * num));

    int MAX_REPEAT = 1;

    for (int es = 5; es < 32; es++) {
        uint32_t h512 = 0;
        uint32_t uh512 = 0;
        uint32_t bwh256 = 0;
        uint32_t bwh512 = 0;
        uint32_t w = 0;
        uint32_t trivial = 0;
        for (int repeat = 0; repeat < MAX_REPEAT; repeat++) {
            bwh512 += bwh_throughput(new BitWeaverHScanner512(es), num, es, bp_input, bp_encoded, bp_output);
            bwh256 += bwh_throughput(new BitWeaverHScanner256(es), num, es, bp_input, bp_encoded, bp_output);
            trivial += bp_throughput(new TrivialBPScanner(es), num, es, bp_input, bp_encoded, bp_output);
            uh512 += bp_throughput(new HaoScanner512(es), num, es, bp_input, bp_encoded, bp_output);
            w += bp_throughput(new WillhalmUnpackerScanner(es), num, es, bp_input, bp_encoded, bp_output);
        }
        std::cout << es << "," << uh512 / MAX_REPEAT << "," << bwh256 / MAX_REPEAT << ","
                  << bwh512 / MAX_REPEAT << "," << w / MAX_REPEAT
                  << "," << trivial / MAX_REPEAT << std::endl;
    }

    delete[] bp_input;
    free(bp_encoded);
    free(bp_output);
}

