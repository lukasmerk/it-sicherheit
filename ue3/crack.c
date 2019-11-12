#include "sha1.h"
#include <stdlib.h>
#include <stdio.h>

#define PWLENGTH 10
#define WORDCOUNT 5

typedef struct {
	char** words;
	// number of words that can be stored in the words array
	uint32 capacity;
	// number of words that really are stored in the words array
	uint32 wordCount;
} wordVec;

/**
 * Initializes the word vector with a capacity of 16 words.
 */
wordVec *initWordVec(void) {
	wordVec *wv = (wordVec*)calloc(1, sizeof(wordVec));
	wv->wordCount = 0;
	wv->capacity = 16;
	wv->words = (char**)calloc(wv->capacity, sizeof(char*));
	return wv;
}

/**
 * Frees all memory used in the word vector including the 
 * memory allocated for the strings of the vector.
 */
void freeWordVec(wordVec *wv) {
	uint32 i;
	for (i = 0; i < wv->wordCount; i++) {
		free(wv->words[i]);
	}
	free(wv->words);
	free(wv);
}

/**
 * Doubles the capacity of the word vector.
 */
void doubleCapacity(wordVec *wv) {
	char** newWordsPointer = (char**)realloc(wv->words, wv->capacity * 2 * sizeof(char*));
	if (newWordsPointer != NULL) {
		wv->capacity = wv->capacity * 2;
		wv->words = newWordsPointer;
		return;
	}
	fprintf(stderr, "Could not double capacity of wordVec.");
	exit(1);
}

/**
 * Adds the given word to the word vector. The capacity of 
 * the vector is increased, if necessary. The word is not
 * copied, so do not free the memory occupied by word unless
 * you know what you are doing.
 */
void add(wordVec *wv, char* word) {
	if (wv->wordCount >= wv->capacity) doubleCapacity(wv);
	wv->words[wv->wordCount] = word;
	wv->wordCount++;
}


/**
 * Copies the content of the 0-terminated string line into
 * a new 0-terminated string that occupies exactly as much
 * memory as is needed to hold the content.
 */
char* copyLine(char* line) {
	uint32 count = 0;
	uint32 i;
	char* s;
	while (line[count] != 0) count++;
	count++;
	s = (char*)calloc(count, sizeof(char));
	for (i = 0; i < count; i++) {
		s[i] = line[i];
	}
	return s;
}


/**
 * Loads a list of passwords from the file implied by the
 * given fileName. This file has to contain a list of words
 * all terminated by a line-break. All of the words in the
 * list have to have at most 39 chars, otherwise this function
 * is prone to buffer-overflows. You have been warned.
 */
wordVec *loadPasswordList(char* fileName) {
	FILE *f;
	wordVec *wv = initWordVec();
	uint32 err;
	char* line = (char*)calloc(40, sizeof(char));
	f = fopen(fileName, "r");
	err = fscanf(f, "%s", line);
	while (err != EOF) {
		add(wv, copyLine(line));
		err = fscanf(f, "%s", line);
	}
	return wv;
}



void print(wordVec *wv) {
	uint32 i;
	printf("wordCount: %d", wv->wordCount);
	for (i = 0; i < wv->wordCount; i++) {
		printf("%s\n", wv->words[i]);
	}
}

/**
 * Returns TRUE if and only if the two given hashes equal.
 * The hashes have to be stored in uint32-arrays of size 5.
 */
boolean hashesEqual(uint32 *h1, uint32 *h2) {
	uint32 i;
	for (i = 0; i < 5; i++) {
		if (h1[i] != h2[i]) return FALSE;
	}
	return TRUE;
}

char* bruteForceCrack(uint32* sha1Hash, char* alphabet, uint8 alphabetSize) {
	char password[PWLENGTH+1];
	uint8 counter[PWLENGTH];
	for (int i = 0; i < PWLENGTH; i++)
	{
		password[i] = '\0';
		counter[i] = -1;
	}
	password[PWLENGTH+1] = '\0';
	counter[0] = 0;

	while (1<2)
	{
		// Passwort erzeugen
		uint8 i = 0;
		while (counter[i] != -1)
		{
			password[i] = alphabet[counter[i]];
			i++;
		}		
		
		// Passwort checken
		bitBlock* bb = forChars(password);
		uint32* sha1pw = sha1(bb);
		freeBitBlock(bb);
		if(hashesEqual(sha1Hash, sha1pw)) {
			printf("Das Passwort wurde gefunden, es lautet %s.\n", password);
			break;
		}
		free(sha1pw);

		// Counter erhöhen
		boolean uebertrag = FALSE;
		uint8 index = 0;
		do
		{
			if(index > PWLENGTH) break;
			if(counter[index] == alphabetSize-1) {
				counter[index] = 0;
				uebertrag = TRUE;
			} else {
				counter[index]++;
				uebertrag = FALSE;
			}
			index++;
		} while (uebertrag == TRUE);
	}
	printf("Wenn dir des Programm nix anderweitiges gsagt hat, hat net klappt.\n\n");

}

int main() {
	bitBlock *msg = forChars("asdfgh");
    uint32* hash = sha1(msg);
    // printWordArrayHex(hash, 5);
    freeBitBlock(msg);
	bruteForceCrack(hash, "abcdefghijklmnopqrstuvwxyz", 26);

	printf("Hash 1:\n");
	int hash1[] = {0x65caa18f, 0x6f33d5e8, 0x9493dc60, 0x8eb00551, 0x26c34997 };
	bruteForceCrack(hash1, "abcdefghijklmnopqrstuvwxyz", 26);

	printf("Hash 2:\n");
	int hash2[] = { 0xd27eb556, 0x73c666c0, 0xc12873cc, 0x6ed592bf, 0xe59ff958 };
	bruteForceCrack(hash2, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 62);

}