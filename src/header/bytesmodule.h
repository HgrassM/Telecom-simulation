#ifndef BYTES_MODULE_H
#define BYTES_MODULE_H

unsigned char* convertToBitStream(unsigned char message[], int message_size);

unsigned char* convertToByteStream(unsigned char* message_bits, int message_bits_size);

void generateError(unsigned char* message_bits, int message_bits_size, int error_percentage);

unsigned char* frameByCharacterCounting(unsigned char* raw_message, int raw_message_size);

unsigned char* frameByBytesFlag(unsigned char* raw_message, int raw_message_size);

unsigned char* frameByBitsFlag(unsigned char* raw_message_bits, int raw_message_bits_size);

#endif
