#ifndef NON_MODULAR_H
#define NON_MODULAR_H

struct NRZCoordinate {
	float time;
	float voltage;
};

NRZCoordinate* generateNrzPolarLSignal(unsigned char message[], int message_size, float bandwidth, float axis_voltage, bool noise_status);

NRZCoordinate* generateNrzPolarISignal(unsigned char message[], int message_size, float bandwidth, float axis_voltage, bool noise_status);

NRZCoordinate* generateNrzManchesterSignal(unsigned char message[], int message_size, float bandwidth, float axis_voltage, bool noise_status);

NRZCoordinate* generateNrzDifferencialManchesterSignal(unsigned char message[], int message_size, float bandwidth, float axis_voltage, bool noise_status);

NRZCoordinate* generateNrzBipolarAMISignal(unsigned char message[], int message_size, float bandwidth, float axis_voltage, bool noise_status);

NRZCoordinate* generateNrzBipolarPseudoternarySignal(unsigned char message[], int message_size, float bandwidth, float axis_voltage, bool noise_status);

#endif

