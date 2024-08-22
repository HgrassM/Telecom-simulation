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
		//Invert bit order per byte to maintain the correct order
		if (i % 8 == 0 && i != 0) {
			message_bytes[byte_index] = (message_bytes[byte_index] & 0xF0) >> 4 | (message_bytes[byte_index] & 0x0F) << 4;
			message_bytes[byte_index] = (message_bytes[byte_index] & 0xCC) >> 2 | (message_bytes[byte_index] & 0x33) << 2;
			message_bytes[byte_index] = (message_bytes[byte_index] & 0xAA) >> 1 | (message_bytes[byte_index] & 0x55) << 1;
			byte_index += 1;	
		}
		
		if (message_bits[i]) {
			//Logical shift to "insert" the bits into the byte
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

unsigned char* frameByCharacterCounting(unsigned char* raw_message, int raw_message_size) {
	unsigned char *frames;
	int extra_byte = 0;
	//Generate a single frame for a message smaller than 8 bytes
	if (raw_message_size < 8) {
		frames = (unsigned char*) malloc(raw_message_size + 1);

		frames[0] = raw_message_size + 1;

		for (int i=0; i<raw_message_size; i++) {
			frames[i+1] = raw_message[i];
		}
	//Generates mutiple frames if the message size is bigger than 8
	}else {
		frames = (unsigned char*) malloc(raw_message_size);
		
		int i=0;
		int iter = 0;

		while(iter < raw_message_size) {
			//Puts character count char for a 7 bytes frame
			if (i%8 == 0 && raw_message_size - iter > 8) {
				extra_byte += 1;
				frames = (unsigned char*) realloc(frames, raw_message_size + extra_byte);
				
				frames[i] = 8;
				i += 1;
				
				for (int k=0; k < 7; k++) {
					frames[i] = raw_message[iter];
					i += 1;
					iter += 1;
				}
			//Puts character count char for a less than 7 bytes frame size
			}else{
				extra_byte += 1;
				frames = (unsigned char*) realloc(frames, raw_message_size + extra_byte);

				frames[i] = (raw_message_size - iter) + 1;
				i += 1;

				while(iter < raw_message_size) {
					frames[i] = raw_message[iter];
					i += 1;
					iter += 1;
				}
			}
		}
	}

	return frames;	
}

unsigned char* frameByBytesFlag(unsigned char* raw_message, int raw_message_size) {
	unsigned char flag_char = 126;
	unsigned char esc_char = 124;
	unsigned char *frames = (unsigned char*) malloc(raw_message_size);
	
	int frames_index = 0;
	int extra_byte = 0;

	for (int iter = 0; iter<raw_message_size; iter++) {
		//Puts a flag to sinalyze the start of a frame
		if (iter == 0) {
			extra_byte += 1;
			frames = (unsigned char*) realloc(frames, raw_message_size + extra_byte);

			frames[frames_index] = flag_char;
			frames_index += 1;
		//Puts a flag to sinalyze the end of a frame and a flag to sinalyze the start of the next frame 
		}else if (iter % 8 == 0) {
			extra_byte += 2;
			frames = (unsigned char*) realloc(frames, raw_message_size + extra_byte);
			
			frames[frames_index] = flag_char;
			frames_index += 1;
			frames[frames_index] = flag_char;
			frames_index += 1;
		}
		
		//Puts an escape byte to sinalyze that the next byte is not a flag, although it is equal to the flag
		if (raw_message[iter] == flag_char) {
			extra_byte += 1;
			frames = (unsigned char*) realloc(frames, raw_message_size + extra_byte);

			frames[frames_index] = esc_char;
			frames_index += 1;
		}
		
		//Puts the message data on the frame
		frames[frames_index] = raw_message[iter];
		frames_index += 1;
	}
	
	extra_byte += 1;
	frames = (unsigned char*) realloc(frames, raw_message_size + extra_byte);

	frames[frames_index] = flag_char;	

	return frames;
}

unsigned char* frameByBitsFlag(unsigned char* raw_message_bits, int raw_message_bits_size) {
	unsigned char *frames = (unsigned char*) malloc(raw_message_bits_size);
	
	int frames_index = 0;
	int message_index = 0;
	int extra_bits = 0;
	int flag_one_count = 0;
	
	while (message_index < raw_message_bits_size) {
		//Puts a flag to sinalyze the start of the frame
		if (message_index == 0) {
			extra_bits += 8;
			frames = (unsigned char*) realloc(frames, raw_message_bits_size + extra_bits);
			
			frames[frames_index] = 0;
			frames_index += 1;

			for (int i=0; i<6; i++) {
				frames[frames_index] = 1;
				frames_index += 1;
			}

			frames[frames_index] = 0;
			frames_index += 1;

		//Puts a flag to sinalyze the end of a frame and puts a flag to sinalyze the start of the next frame
		}else if (message_index % 64 == 0) {
			extra_bits += 16;
			frames = (unsigned char*) realloc(frames, raw_message_bits_size + extra_bits);
			
			for (int k=0; k<2; k++) {
				frames[frames_index] = 0;
				frames_index += 1;

				for (int i=0; i<6; i++) {
					frames[frames_index] = 1;
					frames_index += 1;
				}

				frames[frames_index] = 0;
				frames_index += 1;
			}
		}

		//Puts the message data on the frame
		for (int i=0; i<64; i++) {
			if (message_index >= raw_message_bits_size) {
				extra_bits += 8;
				frames = (unsigned char*) realloc(frames, raw_message_bits_size + extra_bits);
			
				frames[frames_index] = 0;
				frames_index += 1;

				for (int i=0; i<6; i++) {
					frames[frames_index] = 1;
					frames_index += 1;
				}

				frames[frames_index] = 0;
				frames_index += 1;
	
				break;
			}
				
			if (flag_one_count >= 5) {
				extra_bits += 1;
				frames = (unsigned char*) realloc(frames, raw_message_bits_size + extra_bits);
					
				frames[frames_index] = 0;
				frames_index += 1;

				flag_one_count = 0;
			}

			if (raw_message_bits[message_index]) {
				flag_one_count += 1;
			}else{
				flag_one_count = 0;
			}

			frames[frames_index] = raw_message_bits[message_index];
			message_index += 1;
			frames_index += 1;	
		}
	}

	return frames;
}

unsigned char* createErrorCheckBitParity(unsigned char* received_data_bits, int received_data_bits_length) {
	unsigned char *message_with_parity_bit = (unsigned char*) malloc(received_data_bits_length + 1);
	int one_count = 0;

	for (int i=0; i<received_data_bits_length; i++) {
		if (received_data_bits[i]) {
			one_count += 1;
		}

		message_with_parity_bit[i] = received_data_bits[i];
	}

	if (one_count % 2 != 0) {
		message_with_parity_bit[received_data_bits_length] = 1;
	}else{
		message_with_parity_bit[received_data_bits_length] = 0;	
	}

	return message_with_parity_bit;
}

unsigned char* generateCrcErrorCheck(unsigned char* received_data_bits, int received_data_bits_length) {
	unsigned char crc_word[] = {1,1,1,0,1,1,0,1,1,0,1,1,1,0,0,0,1,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0};
	unsigned char *result_word = (unsigned char*) malloc(received_data_bits_length + 31);

	//Copying the message
	for (int i=0; i<received_data_bits_length; i++) {
		result_word[i] = received_data_bits[i];
	}
	
	//Adding 31 zeroes to the copied message
	for (int i=0; i<31; i++) {
		result_word[received_data_bits_length + i] = 0;
	}

	//CRC division algorithm
	for (int i=0; i<received_data_bits_length + 31; i += 32) {
		
		int crc_word_index = 0;
		for (int k=i; k<i+32; k++) {
			result_word[k] = crc_word[crc_word_index] ^ result_word[k];
			crc_word_index += 1;
		}	
	}
	
	//Storing message with key
	for (int i=0; i<received_data_bits_length; i++) {
		result_word[i] = received_data_bits[i];
	}

	return result_word;
}
