#include <stdlib.h>

unsigned char* convertToBitStream(unsigned char message[], int message_size) {
	unsigned char* message_in_bits = (unsigned char*) malloc(8*message_size);

	for (int i=0; i<message_size; i++) {	
		for (int j=0; j<8; j++) {
			message_in_bits[(i*8)+j] = (message[i] >> j & 1) != 0; 
		}
	}

	return message_in_bits;
}
