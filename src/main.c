#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "header/nonmodular.h"
#include "header/bytesmodule.h"

int main() {
	
	//Teste 1
	unsigned char message[] = {'t', 'e', 's', 't', 'e', '\0'};
	unsigned char* bit_stream = convertToBitStream(message, (int)sizeof(message));
	printf("Size of the message is : %d\n", (int)sizeof(message));

	for (int i=0; i<(8*((int)sizeof(message))); i++) {
		if (i%8 != 0) {
			printf("%d", bit_stream[i]);
		}else{
			printf("\n%d", bit_stream[i]);
		}
	}
	
	printf("\n");

	free(bit_stream);
	
	//Teste 2
	NRZCoordinate* coordinates_from_simulation = generateNrzPolarISignal(message, (int)sizeof(message), 500, 50, false);

	for (int i=0; i<8*((int)sizeof(message)); i++) {
		printf("(%.4fs, %.1fV)\n", coordinates_from_simulation[i].time, coordinates_from_simulation[i].voltage);  
	}
	
	free(coordinates_from_simulation);

	return 0;
}
