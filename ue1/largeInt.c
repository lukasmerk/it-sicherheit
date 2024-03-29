
#include <stdio.h>
#include <stdlib.h>
#include "largeInt.h"





/**
 ** Returns the number of leading zeroes of the given argument.
 **
 ** A leading zero is a binary digit of the given arg that is set
 ** to 0 and to the left of which only appear 0's.
 **/
uint8 GetNumberOfLeadingZeroes(uint32 arg) {
    uint32 mask = 0x80000000U;
    uint8 count = 0;

    while (((arg & mask) == 0) && (count < 32)) {
        mask = mask >> 1;
        count++;
    }
    return count;
}



/**
 ** \brief Checks for leading zeroes and, based on the number of these,
 **        computes b->usedWords and b->bitSize.
 **
 ** \param[in] b The BigInt, for which to compute the usage values.
 **/
void RecomputeUsageVariables(LargeInt* b) {
    uint32 i;
    uint32* dataPointer;

    i = b->wordSize - 1;
    dataPointer = &(b->data[i]);

    while (((*dataPointer) == 0) && (i>0)) {
        i--;
        dataPointer--;
    }
    
    b->bitSize = 32 - GetNumberOfLeadingZeroes((*dataPointer)) + (i * BITSPERWORD);
    if (b->bitSize == 0) {
        b->usedWords = 0;
    } else {
        b->usedWords = i + 1;
    }
}




/**
 ** Returns TRUE if and only> if the given LargeInt is even.
 **/
boolean IsEven(const LargeInt* b) {
    uint32 mask = 0x1U;
    if(!(b->data[0] & mask)) return TRUE;
    return FALSE;
}

/**
 ** Returns TRUE if and only if the given LargeInt is odd.
 **/
boolean IsOdd(const LargeInt* b) {
    uint32 mask = 0x1U;
    if(b->data[0] & mask) return TRUE;
    return FALSE;
}

/**
 ** Initializes a new LargeInt with the given integer value.
 **
 ** \param[in] value The value that is to be passed to the new LargeInt.
 ** \param[wordSize] The number of 32-Bit-words that shall used in the new LargeInt.
 ** \return A LargeInt that has been initialized with the given value.
 **/
LargeInt* InitLargeIntWithUint32(uint32 value, uint8 wordSize) {
    LargeInt* x = (LargeInt*)calloc(1, sizeof(LargeInt));
    x->data = (uint32*)calloc(wordSize, sizeof(uint32));
    x->wordSize = wordSize;
    
    if (value == 0) {
        x->usedWords = 0;
        x->bitSize = 0;
    } else {
        x->usedWords = 0;
        x->bitSize = 32 - GetNumberOfLeadingZeroes(value);
        while (value > 0) {
            x->data[x->usedWords] = value & STANDARD_USEBIT_MASK;   
            x->usedWords++;
            value = value >> BITSPERWORD;
        } 
    }
    return x;
}


/**
 * Frees the memory of the given LargeInt.
 */
void freeLargeInt(LargeInt* x) {
    free(x->data);
    free(x);
}



/**
 ** \brief Adds the two given summands and returns the result.
 **
 ** This algorithm treats the arguments as if they were both
 ** positive, i.e. the sign of the integers is ignored.
 **
 ** \param[in] s1 The first summand.
 ** \param[in] s2 The second summand.
 ** \result The sum of s1 and s2. The word size of the result
 **         is exactly as large as needed to hold the sum.
 **
 **/
LargeInt* Add(LargeInt* s1, LargeInt* s2) {
    // Bitte implementieren!}
    //neues largeint mit bitsize
    uint32 maxwords, maxusedwords;
    if(s1->wordSize > s2->wordSize) {
        maxwords = s1->wordSize+1;
    } else {
        maxwords = s2->wordSize+1;
    }
    if(s1->usedWords > s2->usedWords) {
        maxusedwords = s1->usedWords;
    } else {
        maxusedwords = s2->usedWords;
    }
    LargeInt* ergebnis = InitLargeIntWithUint32(0, maxwords);
    uint32 i = 0;
    uint8 uebertrag = 0;

    while (i<=maxusedwords || uebertrag != 0)
    {
        ergebnis->data[i] += uebertrag;
        ergebnis->data[i] += s1->data[i];
        ergebnis->data[i] += s2->data[i];
        uebertrag = STANDARD_CALCBIT_MASK & ergebnis->data[i];
        ergebnis->data[i] = STANDARD_USEBIT_MASK & ergebnis->data[i];
        if(uebertrag>0) uebertrag = 1;
        i++;
    }
    RecomputeUsageVariables(ergebnis);
    return ergebnis;
}

LargeInt* Multiply(LargeInt* m1, LargeInt* m2) {
    uint32 maxwords, maxusedwords, minreqwords, words;
    if(m1->wordSize > m2->wordSize) {
        maxwords = m1->wordSize;
    } else {
        maxwords = m2->wordSize;
    }
    if(m1->usedWords > m2->usedWords) {
        maxusedwords = m1->usedWords;
    } else {
        maxusedwords = m2->usedWords;
    }
    minreqwords = maxusedwords*2;
    if(minreqwords>maxwords) {
        words = minreqwords;
    } else {
        words = maxwords;
    }
    LargeInt* ergebnis = InitLargeIntWithUint32(0, words);
    uint32 i;
    uint32 j;
    uint32 overflow;
    for(i = 0; i<m1->usedWords; i++)
    {
        overflow = 0;
        for(j=0; j<m2->usedWords; j++)
        {
            uint32 blockindex = i+j;
            uint32 mulergebnis = m1->data[i]*m2->data[j];
            uint32 teilergebnis = mulergebnis + overflow + ergebnis->data[blockindex];
            ergebnis->data[blockindex] = teilergebnis & STANDARD_USEBIT_MASK;
            overflow = teilergebnis & STANDARD_CALCBIT_MASK;
            overflow >>= BITSPERWORD;
            // continue;

            // uint32 k = blockindex;
            // uint32 uebertrag = 0;
            // do
            // {
            //     ergebnis->data[k] += uebertrag;
            //     uebertrag = ergebnis->data[k] & STANDARD_CALCBIT_MASK;
            //     ergebnis->data[k] &= STANDARD_USEBIT_MASK;
            //     uebertrag >>= BITSPERWORD;
            //     k++;
            // } while (uebertrag != 0);

            // uint16 carry = teilergebnis & STANDARD_CALCBIT_MASK;
            // carry >>= BITSPERWORD;
            // ergebnis->data[blockindex+1] += carry;
            
            // k = blockindex+1;
            // uebertrag = 0;

            // do
            // {
            //     ergebnis->data[k] += uebertrag;
            //     uebertrag = ergebnis->data[k] & STANDARD_CALCBIT_MASK;
            //     ergebnis->data[k] &= STANDARD_USEBIT_MASK;
            //     uebertrag >>= BITSPERWORD;
            //     k++;
            // } while (uebertrag != 0);
            // j++;
        }
        if(overflow > 0)
        ergebnis->data[i+j] = overflow;
    }
    RecomputeUsageVariables(ergebnis);
    return ergebnis;
}


void printLargeInt(LargeInt *x) {
    int i = x->bitSize - 1;
    while (i >= 0) {
        int wordIndex = i / BITSPERWORD;
        int bitIndex = i % BITSPERWORD;
        uint32 testBit = 1 << bitIndex;
        if ((x->data[wordIndex] & testBit) != 0) {
            printf("1");
        } else {
            printf("0");
        }
        i--;
    }
    printf("\n");
}



// Verstehen Sie die untige main-Funktion bitte als Anstoß zum 
// Testen Ihres Codes. Fügen Sie weitere, sinnvolle Tests hinzu!
int main() {
    LargeInt* x = InitLargeIntWithUint32(70000, 5);
    LargeInt* y = InitLargeIntWithUint32(80000, 5);
    printLargeInt(x);
    printLargeInt(y);
    printf("%i\n", IsEven(x));
    printf("%i\n", IsEven(y));
    printf("%i\n", IsOdd(x));
    printf("%i\n", IsOdd(y));
    LargeInt* z = Add(x, y);
    printLargeInt(z);
    LargeInt* u = Multiply(x,y);
    printLargeInt(u);
    printf("100001011101110010011100000000000\n");
    LargeInt* test = Multiply(InitLargeIntWithUint32(10,5),InitLargeIntWithUint32(10,5));
    printLargeInt(test);
    return 0;
}



