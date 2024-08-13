#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#include "header/bytesmodule.h"

//Srand flag
bool isInitialized = false;

//This function isolates bit per bit of an array of bytes and store them in another array
unsigned char* convertToBitStream(unsigned char message[], int message_size) {
	unsigned char* message_in_bits = (unsigned char*) malloc(8*message_size);

	for (int i=0; i<message_size; i++) {	
		for (int j=0; j<8; j++) {
			message_in_bits[((i*8)+7)-j] = (message[i] >> j & 1) != 0; 
		}
	}

	return message_in_bits;
}

//This function turns an array of bits into a byte array
unsigned char* convertToByteStream(unsigned char* message_bits, int message_bits_size) {
	unsigned char *message_bytes = (unsigned char*) malloc(message_bits_size/8);
	
	int byte_index = 0;

	for (int i=0; i<message_bits_size; i++) {
		if (i % 8 == 0 && i != 0) {
			message_bytes[byte_index] = (message_bytes[byte_index] & 0xF0) >> 4 | (message_bytes[byte_index] & 0x0F) << 4;
			message_bytes[byte_index] = (message_bytes[byte_index] & 0xCC) >> 2 | (message_bytes[byte_index] & 0x33) << 2;
			message_bytes[byte_index] = (message_bytes[byte_index] & 0xAA) >> 1 | (message_bytes[byte_index] & 0x55) << 1;
			byte_index += 1;	
		}
		
		if (message_bits[i]) {
			message_bytes[byte_index] |= 1 << (i%8);
		}	
	}

	return message_bytes;
}

//This function generates an error per bit based on the given probability
void generateError(unsigned char* message_bits, int message_bits_size, int error_percentage) {
	//Initializes rand
	if (!isInitialized) {
		srand(time(0));
		isInitialized = true;
	}
	
	int sorted_number = 0;
	
	for (int i=0; i<message_bits_size; i++) {
		//Calculates a random number in range [1,100]
		sorted_number = (rand() % 100)+1;
		
		//If the given number is inside the percentage probability, the bit gets inverted
		if (sorted_number <= error_percentage && error_percentage != 0) {
			if (message_bits[i]) {
				message_bits[i] = 0;
			}else{
				message_bits[i] = 1;
			}
		}

	}
}
