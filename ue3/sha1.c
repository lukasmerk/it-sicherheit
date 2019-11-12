#include <stdio.h>
#include <stdlib.h>
#include "sha1.h"


bitBlock *initBitBlock(uint32 wordCount) {
	bitBlock *bb = (bitBlock*)calloc(1, sizeof(bitBlock));
	bb->data = (uint32*)calloc(wordCount, sizeof(uint32));
	bb->wordCount = wordCount;
	return bb;
}

void freeBitBlock(bitBlock *bb) {
	free(bb->data);
	free(bb);
}



bitBlock *pad(bitBlock *block) {
	uint32 ibs = block->usedBits;
	if (ibs % 8 == 0) ibs++;
	uint32 inputWordSize = ibs / 32;
	if (ibs % 32 != 0) inputWordSize++;
	uint32 rest = 512 - (ibs % 512);
	if (rest < 64) rest = rest + 512;
	uint32 newWordSize = inputWordSize + (rest / 32);
	bitBlock *result = initBitBlock(newWordSize);
	uint32 i;
	for (i = 0; i < block->usedBits / 32; i++) {
		result->data[i] = block->data[i];
	}
	if (block->usedBits % 32 == 0) {
		result->data[i] = (1 << 31);
	} else {
		result->data[i] = block->data[i];
		if (block->usedBits % 8 == 0) {
			result->data[i] = result->data[i] | (1 << (31-(block->usedBits % 32)));
		}
	}
	result->data[result->wordCount - 1] = block->usedBits;
	return result;
}

uint32 f1(uint32 b, uint32 c, uint32 d) {
    return (b & c) | ((~b) & d);
}

uint32 f2(uint32 b, uint32 c, uint32 d) {
    return b ^ c ^ d;
}

uint32 f3(uint32 b, uint32 c, uint32 d) {
    return (b & c) | (b & d) | (c & d);
}



uint32 *sha1(bitBlock *message) {
	uint32* result = (uint32*)calloc(5, sizeof(uint32));
	uint32 a0 = 0x67452301;
	uint32 b0 = 0xEFCDAB89;
	uint32 c0 = 0x98BADCFE;
	uint32 d0 = 0x10325476;
	uint32 e0 = 0xC3D2E1F0;
	uint32 a1, b1, c1, d1, e1;
    uint32 rot5HighMask = 0b11111 << 27;
	uint32 rot5LowMask = ~rot5HighMask;
    uint32 k[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
    uint32 (*f[4])(uint32, uint32, uint32) = {f1, f2, f3, f2};
	uint32 msf[16];
	uint32 roundCounter;
    uint32 globalWordCounter = 0;
    uint32 i;    
    uint32 z;

    message = pad(message);


    while (globalWordCounter < message->wordCount) {
        a1 = a0;
        b1 = b0;
        c1 = c0;
        d1 = d0;
        e1 = e0;

        for (i = 0; i < 16; i++, globalWordCounter++) {
            msf[i] = message->data[globalWordCounter];
        }

        for (roundCounter = 0; roundCounter < 80; roundCounter++) {
            uint32 stage = roundCounter / 20;
            if (roundCounter >= 16) {
                z = msf[(roundCounter - 16) % 16] ^
                    msf[(roundCounter - 14) % 16] ^
                    msf[(roundCounter - 8) % 16] ^
                    msf[(roundCounter - 3) % 16];
                msf[roundCounter % 16] = (z << 1) | ((z & 0x80000000) >> 31);
            }

            z = e1 +
                f[stage](b1, c1, d1) +
                (((a1 & rot5HighMask) >> 27) + (a1 << 5)) +
                msf[roundCounter % 16] +
                k[stage];
            e1 = d1;
            d1 = c1;
            c1 = ((b1 & 3) << 30) + (b1 >> 2);
            b1 = a1;
            a1 = z;
        }
        a0 = a0 + a1;
        b0 = b0 + b1;
        c0 = c0 + c1;
        d0 = d0 + d1;
        e0 = e0 + e1;
    }

    result[0] = a0;
    result[1] = b0;
    result[2] = c0;
    result[3] = d0;
    result[4] = e0;
    freeBitBlock(message);
	return result;
}


void printBinary(uint32 x) {
	sint32 i;
	uint32 mask = 1 << 31;
	for (i = 31; i >= 0; i--) {
		printf("%d", (x & mask) >> i);
		mask = mask >> 1;
	}
}


void printWordArrayBits(uint32 *a, uint32 size) {
    uint32 i;
    for (i = 0; i < size; i++) {
        if (i < 10) printf(" ");
        printf("%d  ", i);
        printBinary(a[i]);
        printf("\n");
    }
}


void printWordArrayHex(uint32 *a, uint32 size) {
    uint32 i;
    for (i = 0; i < size; i++) 
        printf("%08x", a[i]);
    printf("\n");
}


void printBitBlock(bitBlock *block) {
	printf("Number of Words %d\n", block->wordCount);
    printf("Number of Bits %d\n", block->usedBits);
    printWordArrayBits(block->data, block->wordCount);
}

void printNumberScala(uint32 size) {
    uint32 i;
    for (i = 0; i < size; i++) printf("%d", (i % 10));
    printf("\n");
}


bitBlock *forChars(char *msg) {
    uint32 count = 0;
    while (msg[count] != 0) count++;
    uint32 wordCount = count / 4 + ((count % 4) != 0);
    bitBlock *bb = initBitBlock(wordCount);
    bb->usedBits = count * 8;
    uint32 i;
    for (i = 0; i < count; i++) {
        bb->data[i / 4] = bb->data[i / 4] | (msg[i] << ((3 - (i % 4)) * 8));
    }
    return bb;
}




void test() {
    bitBlock *msg = forChars("schorschelblubber");
    uint32* hash = sha1(msg);
    printWordArrayHex(hash, 5);
    free(hash);
    freeBitBlock(msg);
}