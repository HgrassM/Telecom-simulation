#ifndef CARRIER_MODULE_H
#define CARRIER_MODULE_H

typedef struct {
	double voltage;
	double time;
	bool isLast;
}SinCoordinate;

SinCoordinate* generateAskModulation(unsigned char message[], double bandwidth, int message_size);

SinCoordinate* generateFskModulation(unsigned char message[], double bandwidth, int message_size); 

SinCoordinate* generate8qamModulation(unsigned char message[], double bandwidth, int message_size);

#endif
