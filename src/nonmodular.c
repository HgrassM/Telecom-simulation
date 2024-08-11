#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "header/bytesmodule.h"
#include "header/nonmodular.h"

NRZCoordinate* generateNrzPolarLSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power) {
	
	//Converts the message byte array to bit array and sets NRZ parameters
	unsigned char* message_bits = convertToBitStream(message, message_size);
	//Calculates message size in bits
	int message_bits_size = message_size*8;
	//Calculates data rate for a channel with or without noise
	double bps_rate = 0.0;

	if (noise_status) {
		double signal_power = 0.0;
		
		//Calculating normalized power for NRZ polar
		signal_power += pow(40.0 + 12.0, 2.0);
		signal_power += pow(40.0 - 12.0, 2.0);	

		signal_power = (1.0/2.0)*signal_power;

		//Calculating the channel capacity for a channel with noise (Shannon capacity)
		bps_rate = bandwidth*log2(1.0+(signal_power/noise_power));
	}else{
		//Calculating the channel capacity for a channel without noise (nyquist bit rate)
		bps_rate = 2.0*bandwidth;
	}

	NRZCoordinate* coordinates = (NRZCoordinate*) malloc(sizeof(NRZCoordinate)*message_bits_size);
	
	//Calculates the coordinates for a voltage/seconds system
	for (int i=0; i<message_bits_size; i++) {
		coordinates[i].time = (1.0/bps_rate)*((double)i+1.0);
		
		if (message_bits[i]) {
			coordinates[i].voltage = axis_voltage + 50.0;
		}else{
			coordinates[i].voltage = axis_voltage - 50.0;
		}
	}
	
	free(message_bits);
	return coordinates;
}

NRZCoordinate* generateNrzPolarISignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power) {
	
	//Converts the message byte array to bit array and sets NRZ parameters
	unsigned char* message_bits = convertToBitStream(message, message_size);
	//Calculates message size in bits
	int message_bits_size = message_size*8;
	//Calculates data rate for a channel with or without noise
	double bps_rate = 0.0;

	if (noise_status) {
		double signal_power = 0.0;
		
		//Calculating normalized power for NRZ polar
		signal_power += pow(40.0 + 12.0, 2.0);
		signal_power += pow(40.0 - 12.0, 2.0);	

		signal_power = (1.0/2.0)*signal_power;

		//Calculating the channel capacity for a channel with noise (Shannon capacity)
		bps_rate = bandwidth*log2(1.0+(signal_power/noise_power));
	}else{
		//Calculating the channel capacity for a channel without noise (nyquist bit rate)
		bps_rate = 2.0*bandwidth;
	}

	NRZCoordinate* coordinates = (NRZCoordinate*) malloc(sizeof(NRZCoordinate)*message_bits_size);
	
	//Calculates the coordinates for a voltage/seconds system
	
	double invert = 100.0;
	double current_voltage = axis_voltage + 50.0;
	for (int i=0; i<message_bits_size; i++) {
		coordinates[i].time = (1.0/bps_rate)*((double)i+1.0);
		
		if (message_bits[i]) {
			invert = invert*(-1.0);
			coordinates[i].voltage = current_voltage + invert;
		}else{
			coordinates[i].voltage = current_voltage;
		}

		current_voltage = coordinates[i].voltage;
	}
	
	free(message_bits);
	return coordinates;
}

NRZCoordinate* generateNrzManchesterSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power) {
	
	//Converts the message byte array to bit array and sets NRZ parameters
	unsigned char* message_bits = convertToBitStream(message, message_size);
	//Calculates message size in bits
	int message_bits_size = message_size*8;
	//Calculates signal rate for a channel with or without noise
	double signal_rate = 0.0;

	if (noise_status) {
		double signal_power = 0.0;

		//Calculating normalized power for pulses
		signal_power += pow(40.0 + 12.0, 2);
		signal_power += pow(40.0 - 12.0, 2);	

		signal_power = (1.0/2.0)*signal_power;

		//Calculating the signal oscilation for a channel with noise (Shannon capacity)
		signal_rate = bandwidth*log2(1.0+(signal_power/noise_power));
	}else{
		//Calculating the signal oscilation for a channel without noise (nyquist bit rate)
		signal_rate = 2.0*bandwidth;
	}

	NRZCoordinate* coordinates = (NRZCoordinate*) malloc(sizeof(NRZCoordinate)*message_bits_size*2);
	
	//Calculates the coordinates for a voltage/seconds system
	
	int k = 0;
	for (int i=0; i<message_bits_size; i++) {
		coordinates[k].time = (1.0/signal_rate)*((double)k+1.0);
		coordinates[k+1].time = (1.0/signal_rate)*((double)k+2.0);
		
		if (message_bits[i]) {
			coordinates[k].voltage = axis_voltage - 50.0;
			coordinates[k+1].voltage = axis_voltage + 50.0;
		}else{
			coordinates[k].voltage = axis_voltage + 50.0;
			coordinates[k+1].voltage = axis_voltage - 50.0;
		}

		k += 2;
	}
	
	free(message_bits);
	return coordinates;

}

NRZCoordinate* generateNrzDifferencialManchesterSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power) {
	
	//Converts the message byte array to bit array and sets NRZ parameters
	unsigned char* message_bits = convertToBitStream(message, message_size);
	//Calculates message size in bits
	int message_bits_size = message_size*8;
	//Calculates signal rate for a channel with or without noise
	double signal_rate = 0.0;

	if (noise_status) {
		double signal_power = 0.0;

		//Calculating normalized power for pulses
		signal_power += pow(40.0 + 12.0, 2);
		signal_power += pow(40.0 - 12.0, 2);	

		signal_power = (1.0/2.0)*signal_power;

		//Calculating the signal oscilation for a channel with noise (Shannon capacity)
		signal_rate = bandwidth*log2(1.0+(signal_power/noise_power));
	}else{
		//Calculating the signal oscilation for a channel without noise (nyquist bit rate)
		signal_rate = 2.0*bandwidth;
	}

	NRZCoordinate* coordinates = (NRZCoordinate*) malloc(sizeof(NRZCoordinate)*message_bits_size*2);
	
	//Calculates the coordinates for a voltage/seconds system
	
	int k = 0;
	double invert = 50.0;
	for (int i=0; i<message_bits_size; i++) {
		coordinates[k].time = (1.0/signal_rate)*((double)k+1.0);
		coordinates[k+1].time = (1.0/signal_rate)*((double)k+2.0);
		
		if (message_bits[i]) {
			invert = (invert*-1.0);
		}

		coordinates[k].voltage = axis_voltage + invert;
		coordinates[k+1].voltage = axis_voltage - invert;

		k += 2;
	}
	
	free(message_bits);
	return coordinates;

}

NRZCoordinate* generateNrzBipolarAMISignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power) {
	
	//Converts the message byte array to bit array and sets NRZ parameters
	unsigned char* message_bits = convertToBitStream(message, message_size);
	//Calculates message size in bits
	int message_bits_size = message_size*8;
	//Calculates data rate for a channel with or without noise
	double bps_rate = 0.0;

	if (noise_status) {
		double signal_power = 0.0;
		
		//Calculating normalized power for NRZ bipolar
		signal_power += pow(40.0 + 12.0, 2);
		signal_power += pow(40.0 - 12.0, 2);	

		signal_power = (1.0/2.0)*signal_power;

		//Calculating the channel capacity for a channel with noisei (Shannon capacity)
		bps_rate = bandwidth*log2(1.0+(signal_power/noise_power));
	}else{
		//Calculating the channel capacity for a channel without noise (nyquist bit rate)
		bps_rate = 2.0*bandwidth*log2(3.0);
	}

	NRZCoordinate* coordinates = (NRZCoordinate*) malloc(sizeof(NRZCoordinate)*message_bits_size);
	
	//Calculates the coordinates for a voltage/seconds system
	double invert = -1.0;
	for (int i=0; i<message_bits_size; i++) {
		coordinates[i].time = (1.0/bps_rate)*((double)i+1.0);
		
		if (message_bits[i]) {
			invert = invert*(-1.0);
			coordinates[i].voltage = axis_voltage + (50.0*invert);
		}else{
			coordinates[i].voltage = axis_voltage;
		}
	}
	
	free(message_bits);
	return coordinates;
}

NRZCoordinate* generateNrzBipolarPseudoternarySignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power) {
	
	//Converts the message byte array to bit array and sets NRZ parameters
	unsigned char* message_bits = convertToBitStream(message, message_size);
	//Calculates message size in bits
	int message_bits_size = message_size*8;
	//Calculates data rate for a channel with or without noise
	double bps_rate = 0.0;

	if (noise_status) {
		double signal_power = 0.0;
		
		//Calculating normalized power for NRZ bipolar
		signal_power += pow(40.0 + 12.0, 2);
		signal_power += pow(40.0 - 12.0, 2);	

		signal_power = (1.0/2.0)*signal_power;

		//Calculating the channel capacity for a channel with noisei (Shannon capacity)
		bps_rate = bandwidth*log2(1.0+(signal_power/noise_power));
	}else{
		//Calculating the channel capacity for a channel without noise (nyquist bit rate)
		bps_rate = 2.0*bandwidth*log2(3.0);
	}

	NRZCoordinate* coordinates = (NRZCoordinate*) malloc(sizeof(NRZCoordinate)*message_bits_size);
	
	//Calculates the coordinates for a voltage/seconds system
	double invert = -1.0;
	for (int i=0; i<message_bits_size; i++) {
		coordinates[i].time = (1.0/bps_rate)*((double)i+1.0);
		
		if (message_bits[i]) {
			coordinates[i].voltage = axis_voltage;
		}else{
			invert = invert*(-1.0);
			coordinates[i].voltage = axis_voltage + (50.0*invert);
		}
	}
	
	free(message_bits);
	return coordinates;
}
