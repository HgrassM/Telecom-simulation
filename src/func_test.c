#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "header/nonmodular.h"
#include "header/bytesmodule.h"
#include "header/carriermodule.h"

int main() {
	unsigned char message[] = "testeabcdefghijklmnopq";
	unsigned char *bits = convertToBitStream(message, 23);	
	
	printf("%s\n", message);

	for (int i=0; i<23*8; i++) {
		printf("%d", bits[i]);
	}
	printf("\n");

	for (int i=0; i<23; i++) {
		printf("%d\n", message[i]);
	}

	
	//generateError(bits, 6*8, 10);

	unsigned char *message2 = convertToByteStream(bits, 23*8);

	printf("%s\n", message2);
	
	unsigned char *frames = frameByBytesFlag(message2, 23);

	for (int i=0; i<23+6; i++) {
		printf("%d\n", frames[i]);
	}
	
	printf("\n");

	unsigned char *frames_decode = getMessageFromByteFrame(frames, 23+6);

	for (int i=0; i<23; i++) {
		printf("%d\n", frames_decode[i]);
	}
	
	printf("---------------------------------------------------------------------------\n");

	unsigned char *frames2 = createErrorCheckBitParity(bits, 23*8);
	int counter = 0;

	for (int i=0; i<(23*8)+1; i++) {
		printf("%d", frames2[i]);
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		if (frames2[i]) {
			counter += 1;
		}
	}

	printf("\n");

	printf("Number of 1's: %d\n", counter);

	printf("\n\n");

	unsigned char *decode_parity = verifyErrorByParity(frames2, (23*8)+1);
	
	for (int i=0; i<(23*8); i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", decode_parity[i]);
	}

	printf("\n");
	
	printf("---------------------------------------------------------------------------\n");

	unsigned char *frames3 = frameByBitsFlag(bits, 23*8);	

	for (int i=0; i<((23*8) + (6*8)); i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", frames3[i]);
	}

	printf("\n");
	printf("\n");

	unsigned char *frames3_decode = getMessageFromBitFrame(frames3, (23*8) + (6*8));
	
	for (int i=0; i<(23*8); i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", frames3_decode[i]);
	}

	printf("\n");

	printf("---------------------------------------------------------------------------\n");

	unsigned char *crc_word = generateCrcErrorCheck(bits, 23*8); 

	for (int i=0; i<((23*8)+31); i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", crc_word[i]);
	}

	printf("\n\n");

	unsigned char *crc_word_decode = verifyErrorByCrc(crc_word, (23*8)+31); 

	for (int i=0; i<23*8; i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", crc_word_decode[i]);
	}

	printf("\n");

	printf("---------------------------------------------------------------------------\n");

	unsigned char *hamming_word = generateHammingErrorCorrection(bits, 23*8); 
	hamming_word[190] = 1;

	for (int i=0; i<((23*8)+8); i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", hamming_word[i]);
	}

	printf("\n\n");

	unsigned char *hamming_word_decode = verifyErrorByHamming(hamming_word, (23*8)+8); 

	for (int i=0; i<23*8; i++) {
		
		if (i%8 == 0 && i!=0) {
			printf("\n");
		}

		printf("%d", hamming_word_decode[i]);
	}

	printf("\n");
	
	free(hamming_word_decode);	
	free(crc_word_decode);
	free(decode_parity);
	free(frames3_decode);
	free(frames_decode);
	free(hamming_word);
	free(crc_word);
	free(frames3);
	free(frames2);
	free(bits);
	free(message2);
	free(frames);

	return 0;
}
