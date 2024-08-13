#ifndef BYTES_MODULE_H
#define BYTES_MODULE_H

unsigned char* convertToBitStream(unsigned char message[], int message_size);

unsigned char* convertToByteStream(unsigned char* message_bits, int message_bits_size);

void generateError(unsigned char* message_bits, int message_bits_size, int error_percentage);

#endif
