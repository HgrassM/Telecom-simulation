#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>

#include "header/bytesmodule.h"
#include "header/carriermodule.h"

SinCoordinate* generateAskModulation(unsigned char message[], double bandwidth, int message_size) {
	double amplitude = 50.0;
	double start = 0.0;
	int count = 0;
	
	unsigned char *message_bits = convertToBitStream(message, message_size);
	SinCoordinate *new_coordinates = (SinCoordinate*) malloc(sizeof(SinCoordinate));

	for (int i=0; i<message_size*8; i++) {
		for (double j=start; j<start+40.0; j += 0.1) {
			if (message_bits[i]) {
				new_coordinates[count].voltage = amplitude*sin(2.0*3.14*bandwidth*j);
				new_coordinates[count].time = j;
			}else{
				new_coordinates[count].voltage = 0.0;
				new_coordinates[count].time = j;
			}

			new_coordinates[count].isLast = false;
			
			new_coordinates = (SinCoordinate*) realloc(new_coordinates, sizeof(SinCoordinate)*(count + 2));

			count += 1;	
		}

		start += 40.0;
	}

	new_coordinates[count].voltage = -1.0;
	new_coordinates[count].time = -1.0;
	new_coordinates[count].isLast = true;

	free(message_bits);

	return new_coordinates;
}

SinCoordinate* generateFskModulation(unsigned char message[], double bandwidth, int message_size) {
	double amplitude = 50.0;
	double start = 0.0;
	int count = 0;

	unsigned char *message_bits = convertToBitStream(message, message_size);
	SinCoordinate *new_coordinates = (SinCoordinate*) malloc(sizeof(SinCoordinate));

	for (int i=0; i<message_size*8; i++) {
		for (double j=start; j<start+40.0; j+=0.1) {
			if (message_bits[i]) {
				new_coordinates[count].voltage = amplitude*sin(2.0*3.14*bandwidth*j);
				new_coordinates[count].time = j;
			}else{
				new_coordinates[count].voltage = amplitude*sin((bandwidth/4.0)*j);
				new_coordinates[count].time = j;	
			}
			
			new_coordinates[count].isLast = false;

			new_coordinates = (SinCoordinate*) realloc(new_coordinates, sizeof(SinCoordinate)*(count+2));
			count += 1;	
		}

		start += 40.0;
	}
	
	new_coordinates[count].voltage = -1.0;
	new_coordinates[count].time = -1.0;
	new_coordinates[count].isLast = true;

	free(message_bits);

	return new_coordinates;
}

SinCoordinate* generate8qamModulation(unsigned char message[], double bandwidth, int message_size) {
	double Ai = 0.0;
	double Aq = 0.0;
	double start = 0.0;
	int count = 0;

	unsigned char *message_bits = convertToBitStream(message, message_size);
	SinCoordinate *new_coordinates = (SinCoordinate*) malloc(sizeof(SinCoordinate));

	for (int i=0; i<message_size*8; i+=3) {
		unsigned char bits[3]; 
		if (i+2 >= message_size*8) { 
			int k = 0;
			while (i < message_size*8) {
				bits[k] = message_bits[i];
				i += 1;
				k += 1;
			}

			while (k<3) {
				bits[k] = 0;
				k += 1;
			}
		}else{
			bits[0] = message_bits[i];
			bits[1] = message_bits[i+1];
			bits[2] = message_bits[i+2];
		}
		
		char *str = (char*) malloc(4);
		for (int i=0; i<3; i++) {
			sprintf(str, "%d%d%d", bits[0], bits[1], bits[2]);
		}
		
		for (double j=start; j<start+120.0; j+=0.1) {
			
			char *str2 = (char*) malloc(4);
			str2[0] = '0';
			str2[1]	= '0';
			str2[2] = '0';
			str2[3] = '\0';

			if (strcmp(str, str2) == 0) {
				Ai = 1.0*cos(2.0*3.14*bandwidth*j);
				Aq = 0.0*sin(2.0*3.14*bandwidth*j);
			}		
			str2[2] = '1';
			if (strcmp(str, str2) == 0) {
				Ai = 0.75*cos(2.0*3.14*bandwidth*j);
				Aq = 0.75*sin(2.0*3.14*bandwidth*j);
			}
			str2[1] = '1';
			str2[2] = '0';
			if (strcmp(str, str2) == 0) {
				Ai = -0.75*cos(2.0*3.14*bandwidth*j);
				Aq = 0.75*sin(2.0*3.14*bandwidth*j);
			}			
			str2[2] = '1';
			if (strcmp(str, str2) == 0) {
				Ai = 0.0*cos(2.0*3.14*bandwidth*j);
				Aq = 1.0*sin(2.0*3.14*bandwidth*j);
			}
			str2[0] = '1';
			str2[1] = '0';
			str2[2] = '0';
			if (strcmp(str, str2) == 0) {
				Ai = 0.75*cos(2.0*3.14*bandwidth*j);
				Aq = -0.75*sin(2.0*3.14*bandwidth*j);
			}
			str2[2] = '1';
			if (strcmp(str, str2) == 0) {
				Ai = 1.0*cos(2.0*3.14*bandwidth*j);
				Aq = -1.0*sin(2.0*3.14*bandwidth*j);
			}
			str[1] = '1';
			str[2] = '0';
			if (strcmp(str, str2) == 0) {
				Ai = -1.0*cos(2.0*3.14*bandwidth*j);
				Aq = 0.0*sin(2.0*3.14*bandwidth*j);
			}
			str[2] = '1';
			if (strcmp(str, str2) == 0) {
				Ai = -0.75*cos(2.0*3.14*bandwidth*j);
				Aq = -0.75*sin(2.0*3.14*bandwidth*j);
			}
			
			new_coordinates[count].voltage = 50*(Ai - Aq);
			new_coordinates[count].time = j;

			new_coordinates[count].isLast = false;
			new_coordinates = (SinCoordinate*) realloc(new_coordinates, sizeof(SinCoordinate)*(count+2));

			count += 1;

			free(str2);
		}

		new_coordinates[count].voltage = -1.0;
		new_coordinates[count].time = -1.0;
		new_coordinates[count].isLast = true;
		
		start += 120.0;

		free(str);
	}
	
	free(message_bits);

	return new_coordinates;
}
