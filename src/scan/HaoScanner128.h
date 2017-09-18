/*
 * HaoScanner.h
 *
 * Use AVX2 to perform scanning
 *
 *  Created on: Aug 25, 2017
 *      Author: harper
 */

#ifndef SRC_SCAN_HAOSCANNER128_H_
#define SRC_SCAN_HAOSCANNER128_H_

#include <stdlib.h>
#include <immintrin.h>
#include "Scanner.h"

class HaoScanner128 : public Scanner {
private:
    int entrySize;
    bool aligned = true;

    int *data = NULL;
    int *dest = NULL;
    int length = 0;
    Predicate* predicate;

    __m128i *val1s = NULL;
    __m128i *val2s = NULL;
    __m128i *nval1s = NULL;
    __m128i *nval2s = NULL;
    __m128i *nmval1s = NULL;
    __m128i *nmval2s = NULL;
    __m128i *msbmasks = NULL;
    __m128i *notmasks = NULL;
public:
    HaoScanner128(int, bool);

    virtual ~HaoScanner128();

    void scan(int *, int, int *, Predicate *);

protected:
    void alignedEq();
    void unalignedEq();

    void alignedIn();
    void unalignedIn();
};


#endif /* SRC_SCAN_HAOSCANNER128_H_ */
