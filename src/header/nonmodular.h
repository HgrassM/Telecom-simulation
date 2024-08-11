#ifndef NON_MODULAR_H
#define NON_MODULAR_H

typedef struct {
	double time;
	double voltage;
}NRZCoordinate;

NRZCoordinate* generateNrzPolarLSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power);

NRZCoordinate* generateNrzPolarISignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power);

NRZCoordinate* generateNrzManchesterSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power);

NRZCoordinate* generateNrzDifferencialManchesterSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power);

NRZCoordinate* generateNrzBipolarAMISignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power);

NRZCoordinate* generateNrzBipolarPseudoternarySignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status, double noise_power);

#endif
