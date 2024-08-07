#ifndef NON_MODULAR_H
#define NON_MODULAR_H

typedef struct {
	double time;
	double voltage;
}NRZCoordinate;

NRZCoordinate* generateNrzPolarLSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status);

NRZCoordinate* generateNrzPolarISignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status);

NRZCoordinate* generateNrzManchesterSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status);

NRZCoordinate* generateNrzDifferencialManchesterSignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status);

NRZCoordinate* generateNrzBipolarAMISignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status);

NRZCoordinate* generateNrzBipolarPseudoternarySignal(unsigned char message[], int message_size, double bandwidth, double axis_voltage, bool noise_status);

#endif
