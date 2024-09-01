#ifndef BYTES_MODULE_H
#define BYTES_MODULE_H

typedef struct {
	unsigned char* result_message;
	int result_message_length;
}ResultWithSize;

unsigned char* convertToBitStream(unsigned char message[], int message_size);

ResultWithSize convertToByteStream(unsigned char* message_bits, int message_bits_size);

void generateError(unsigned char* message_bits, int message_bits_size, int error_percentage);

ResultWithSize frameByCharacterCounting(unsigned char* raw_message, int raw_message_size);

ResultWithSize frameByBytesFlag(unsigned char* raw_message, int raw_message_size);

ResultWithSize frameByBitsFlag(unsigned char* raw_message_bits, int raw_message_bits_size);

ResultWithSize createErrorCheckBitParity(unsigned char* received_data_bits, int received_data_bits_length);

ResultWithSize generateCrcErrorCheck(unsigned char* received_data_bits, int received_data_bits_length);

ResultWithSize generateHammingErrorCorrection(unsigned char* received_data_bits, int received_data_bits_length);

ResultWithSize getMessageFromCharacterFrame(unsigned char* received_message, int message_length);

ResultWithSize getMessageFromByteFrame(unsigned char* received_message, int message_length);

ResultWithSize getMessageFromBitFrame(unsigned char* received_message_bits, int message_length);

ResultWithSize verifyErrorByParity(unsigned char* received_message_bits, int message_length);

ResultWithSize verifyErrorByCrc(unsigned char* received_message_bits, int message_length);

ResultWithSize verifyErrorByHamming(unsigned char* received_message_bits, int message_size);

#endif
