#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <gtk/gtk.h>

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
ResultWithSize convertToByteStream(unsigned char* message_bits, int message_bits_size) {
	unsigned char *message_bytes = (unsigned char*) malloc(message_bits_size/8);
	
	for (int i=0; i<message_bits_size/8; i++) {
		message_bytes[i] = 0;
	}
	
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
	
	message_bytes[byte_index] = (message_bytes[byte_index] & 0xF0) >> 4 | (message_bytes[byte_index] & 0x0F) << 4;
	message_bytes[byte_index] = (message_bytes[byte_index] & 0xCC) >> 2 | (message_bytes[byte_index] & 0x33) << 2;
	message_bytes[byte_index] = (message_bytes[byte_index] & 0xAA) >> 1 | (message_bytes[byte_index] & 0x55) << 1;
	
	
	ResultWithSize result = {message_bytes, (message_bits_size/8)};
	
	return result;	
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

ResultWithSize frameByCharacterCounting(unsigned char* raw_message, int raw_message_size) {
	unsigned char *frames;
	int extra_byte = 0;
	//Generate a single frame for a message smaller than 8 bytes
	if (raw_message_size < 8) {
		extra_byte += 1;
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

	ResultWithSize result = {frames, raw_message_size + extra_byte};
	
	return result;	
}

ResultWithSize frameByBytesFlag(unsigned char* raw_message, int raw_message_size) {
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

	ResultWithSize result = {frames, raw_message_size + extra_byte};
	
	return result;
}

ResultWithSize frameByBitsFlag(unsigned char* raw_message_bits, int raw_message_bits_size) {
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

	ResultWithSize result = {frames, raw_message_bits_size + extra_bits};
	
	return result;
}

ResultWithSize createErrorCheckBitParity(unsigned char* received_data_bits, int received_data_bits_length) {
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

	ResultWithSize result = {message_with_parity_bit, received_data_bits_length + 1};
	
	return result;
}

ResultWithSize generateCrcErrorCheck(unsigned char* received_data_bits, int received_data_bits_length) {
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
	for (int i=0; i<received_data_bits_length + 31; i++) {
		
		if (result_word[i] != 0 && (received_data_bits_length + 31) - i > 31) {
			int crc_word_index = 0;
			for (int k=i; k<i+32; k++) {
				result_word[k] = crc_word[crc_word_index] ^ result_word[k];
				crc_word_index += 1;
			}
		}
	}
	
	//Storing message with key
	for (int i=0; i<received_data_bits_length; i++) {
		result_word[i] = received_data_bits[i];
	}

	ResultWithSize result = {result_word, received_data_bits_length + 31};
	
	return result;
}

ResultWithSize generateHammingErrorCorrection(unsigned char* received_data_bits, int received_data_bits_length) {
	unsigned char *result_message = (unsigned char*) malloc(received_data_bits_length);
	
	int parity = 1;
	int exponent = 0;
	int parity_bits_num = 0;
	int iter = 0;
	int message_index = 0;

	//Add parity bits to the message
	while (iter<received_data_bits_length) {
		if (iter == parity-1) {
			parity_bits_num += 1;
			result_message = (unsigned char*) realloc(result_message, received_data_bits_length + parity_bits_num);

			result_message[iter] = 0;

			exponent += 1;
			parity = (int) pow(2.0, (double) exponent);
		}
		
		iter += 1;
	}
	
	exponent = 0;
	parity = 1;
	iter = 0;
	while (iter < received_data_bits_length + parity_bits_num) {
		if (iter != parity-1) {
			result_message[iter] = received_data_bits[message_index];
			message_index += 1;
		}else{
			exponent += 1;
			parity = (int) pow(2.0, (double) exponent);
		}

		iter += 1;
	}
	
	//Assigning the correct value to the parity bits
	parity = 1;
	exponent = 0;
	iter = 0;
	int parity_bit_sum = 0;

	while (parity < received_data_bits_length) {
		iter = parity - 1;
		
		while (iter < received_data_bits_length + parity_bits_num) {
			for (int i=0; i<parity; i++) {
				if (iter < received_data_bits_length + parity_bits_num) {
					parity_bit_sum += result_message[iter];
					iter += 1;
				}
			}

			iter += parity;
		}
		
		if (parity_bit_sum % 2 == 0) {
			result_message[parity-1] = 0;
		}else{
			result_message[parity-1] = 1;
		}

		exponent += 1;
		parity = (int) pow(2.0, (double) exponent);

		parity_bit_sum = 0;
	}
		
	ResultWithSize result = {result_message, received_data_bits_length + parity_bits_num};
	
	return result;	
}

ResultWithSize getMessageFromCharacterFrame(unsigned char* received_message, int message_length) {
	unsigned char* result_message = (unsigned char*) malloc(message_length);

	int frame_byte_num = (int)received_message[0];
	int received_message_index = 0;
	int result_message_index = 0;
	int extra_bytes = 0;
	
	//Gets N bytes based on the character counting byte value
	while (received_message_index < message_length) {
		extra_bytes += 1;
		
		for (int i=0; i<frame_byte_num; i++) {
			if (received_message_index >= message_length) {
				free(result_message);
				ResultWithSize result = {NULL, 0};
				return result;
			} 
			
			if (i != 0) {
				result_message[result_message_index] = received_message[received_message_index];
				result_message_index += 1;
			}
			
			received_message_index += 1;
		}
		
		frame_byte_num = (int)received_message[received_message_index];
	}
	
	ResultWithSize result = {result_message, message_length - extra_bytes};
	
	return result;
}

ResultWithSize getMessageFromByteFrame(unsigned char* received_message, int message_length) {
	unsigned char* raw_message = (unsigned char*) malloc(message_length);
	unsigned char flag_char = 126;
	unsigned char esc_char = 124;
	int raw_message_index = 0;
	bool isEscActive = false;
	bool nextIsStartFlag = true;
	bool errorInFlag = false;
	
	int extra_bytes = 0;

	//Remove the flags
	for (int i=0; i<message_length; i++) {
		if (received_message[i] == esc_char) {
			isEscActive = true;
			extra_bytes += 1;
		}
		
		if ((nextIsStartFlag && received_message[i] != flag_char) || (i == message_length - 1 && received_message[i] != flag_char)) {
			errorInFlag = true;
		}else if (received_message[i] == flag_char && isEscActive) {
			isEscActive = false;
		}else if (received_message[i] == flag_char) {
			nextIsStartFlag = !nextIsStartFlag;
			extra_bytes += 1;		
		}else{
			raw_message[raw_message_index] = received_message[i];
			raw_message_index += 1;
			isEscActive = false;
		}
	}

	//Verifies if there are errors on the flags
	if (errorInFlag) {
		free(raw_message);
		ResultWithSize result = {NULL, 0};
		return result;
	}

	ResultWithSize result = {raw_message, message_length - extra_bytes};
	
	return result;
}

ResultWithSize getMessageFromBitFrame(unsigned char* received_message_bits, int message_length) {
	unsigned char* raw_message = (unsigned char*) malloc(message_length);
	int one_counter = 0;
	int byte_index = 0;
	int raw_message_index = 0;
	bool isFlag = false;
	bool nextIsStartFlag = true;
	bool removeZero = false;
	bool flagHasError = false;
	
	int extra_bits = 0;

	unsigned char byte[8] = {0};
		

	for (int i=0; i<message_length; i++) {
		
		if (received_message_bits[i]) {
			one_counter += 1;
			
			byte[byte_index] = received_message_bits[i];
			byte_index += 1;
		}else{	
			
			if (one_counter == 5) {
				removeZero = true;
				isFlag = false;
				extra_bits += 1;
			}else if (one_counter == 6) {
				isFlag = true;	
			}

			if (!removeZero) {
				byte[byte_index] = received_message_bits[i];
				byte_index += 1;
			}
			
			removeZero = false;
			one_counter = 0;
		}

		if (byte_index == 8) {
			if ((nextIsStartFlag && !isFlag) || (i == message_length - 1 && !isFlag)) {
				flagHasError = true;
			}
			
			if (!isFlag) {
				for (int k=0; k<8; k++) {
					raw_message[raw_message_index] = byte[k];
					raw_message_index += 1;	
				}
			}else{
				nextIsStartFlag = !nextIsStartFlag;
				extra_bits += 8;	
			}
			
			isFlag = false;
			byte_index = 0;
		}
	}

	if (flagHasError) {
		free(raw_message);
		ResultWithSize result = {NULL, 0};
		return result;
	}

	ResultWithSize result = {raw_message, message_length - extra_bits};
	
	return result;
}

ResultWithSize verifyErrorByParity(unsigned char* received_message_bits, int message_length) {
	unsigned char* raw_message = (unsigned char*) malloc(message_length - 1);
	int one_count = 0;
	int parity_bit = 0;
	
	//Counts the number of 1's 
	for (int i=0; i<message_length; i++) {
		if (i == message_length - 1) {
			parity_bit = received_message_bits[i];
			break;
		}
		
		if (received_message_bits[i]) {
			one_count += 1;
		}

		raw_message[i] = received_message_bits[i];
	}
	
	//Verifies if parity bit is correct
	if (one_count % 2 == 0 && parity_bit != 0) {
		free(raw_message);
		ResultWithSize result = {NULL, 0};
		return result;
	}else if (one_count % 2 != 0 && parity_bit != 1){
		free(raw_message);
		ResultWithSize result = {NULL, 0};
		return result;
	}

	ResultWithSize result = {raw_message, message_length - 1};
	
	return result;
}

ResultWithSize verifyErrorByCrc(unsigned char* received_message_bits, int message_length) {
	unsigned char crc_word[] = {1,1,1,0,1,1,0,1,1,0,1,1,1,0,0,0,1,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0};
	unsigned char *result_word = (unsigned char*) malloc(message_length);
	unsigned char *raw_message = (unsigned char*) malloc(message_length - 31);
	bool hasError = false;
	
	//Copying message
	for (int i=0; i<message_length; i++) {
		result_word[i] = received_message_bits[i];

		if (i < message_length-31) {
			raw_message[i] = received_message_bits[i];
		}
	}

	//CRC division algorithm
	for (int i=0; i<message_length; i++) {
		
		if (result_word[i] != 0 && (message_length) - i > 31) {
			int crc_word_index = 0;
			for (int k=i; k<i+32; k++) {
				result_word[k] = crc_word[crc_word_index] ^ result_word[k];
				crc_word_index += 1;
			}
		}
	}

	//Verifies if result is equals zero
	for (int i=0; i<message_length; i++) {
		if (result_word[i]) {
			hasError = true;
		}
	}

	if (hasError) {
		free(result_word);
		free(raw_message);
		ResultWithSize result = {NULL, 0};
		return result;
	}
	
	free(result_word);
	ResultWithSize result = {raw_message, message_length - 31};
	
	return result;
}

ResultWithSize verifyErrorByHamming(unsigned char* received_message_bits, int message_size) {
	unsigned char* raw_message = (unsigned char*) malloc(message_size);
	int* ones_array = (int*) malloc(sizeof(int));
	int* ones_array2 = (int*) malloc(sizeof(int));
	
	//If there is an error, it tries to correct it
	int ones_array_index = 0;
	int ones_array_index2 = 0;
	int result = 0;
	int result2 = 0;

	for (int i=0; i<message_size; i++) {
		if (received_message_bits[i]) {
			ones_array[ones_array_index] = i+1;
			ones_array_index += 1;

			ones_array = (int*) realloc(ones_array, sizeof(int)*(ones_array_index + 1));
		}
	}
	
	result = ones_array[0];
	for (int i=1; i<ones_array_index; i++) {
		result = result ^ ones_array[i];
	}
	
	if (result != 0) {
		if (received_message_bits[result - 1]) {
			received_message_bits[result - 1] = 0;
		}else{
			received_message_bits[result - 1] = 1;
		}

		//Check if error was corrected
		for (int i=0; i<message_size; i++) {
			if (received_message_bits[i]) {
				ones_array2[ones_array_index2] = i+1;
				ones_array_index2 += 1;

				ones_array2 = (int*) realloc(ones_array2, sizeof(int)*(ones_array_index2 + 1));
			}
		}
	
		result2 = ones_array2[0];
		for (int i=1; i<ones_array_index2; i++) {
			result2 = result2 ^ ones_array2[i];
		}

		if (result2 != 0) {
			free(ones_array);
			free(ones_array2);
			free(raw_message);
			ResultWithSize result = {NULL, 0};
			return result;
		}
	}

	//Removing parity bits to return the raw message
	int parity = 1;
	int message_index = 0;
	int exponent = 0;
	
	int extra_bits = 0;
	
	for (int i=0; i<message_size; i++) {
		if (i+1 == parity) {
			exponent += 1;
			parity = (int) pow(2.0, (double) exponent);
			
			extra_bits += 1;
		}else{		
			raw_message[message_index] = received_message_bits[i];
			message_index += 1;
		}
	}

	free(ones_array);
	free(ones_array2);
	
	ResultWithSize result_m = {raw_message, message_size - extra_bits};
	
	return result_m;
}
