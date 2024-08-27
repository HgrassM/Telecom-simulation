#ifndef BYTES_MODULE_H
#define BYTES_MODULE_H

unsigned char* convertToBitStream(unsigned char message[], int message_size);

unsigned char* convertToByteStream(unsigned char* message_bits, int message_bits_size);

void generateError(unsigned char* message_bits, int message_bits_size, int error_percentage);

unsigned char* frameByCharacterCounting(unsigned char* raw_message, int raw_message_size);

unsigned char* frameByBytesFlag(unsigned char* raw_message, int raw_message_size);

unsigned char* frameByBitsFlag(unsigned char* raw_message_bits, int raw_message_bits_size);

unsigned char* createErrorCheckBitParity(unsigned char* received_data_bits, int received_data_bits_length);

unsigned char* generateCrcErrorCheck(unsigned char* received_data_bits, int received_data_bits_length);

unsigned char* generateHammingErrorCorrection(unsigned char* received_data_bits, int received_data_bits_length);

unsigned char* getMessageFromByteFrame(unsigned char* received_message, int message_length);

unsigned char* getMessageFromBitFrame(unsigned char* received_message_bits, int message_length);

unsigned char* verifyErrorByParity(unsigned char* received_message_bits, int message_length);

unsigned char* verifyErrorByCrc(unsigned char* received_message_bits, int message_length);

unsigned char* verifyErrorByHamming(unsigned char* received_message_bits, int message_size);

#endif
