#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "header/nonmodular.h"
#include "header/gtkmodule.h"
#include "header/carriermodule.h"
#include "header/bytesmodule.h"

//Global variables
static int digital_coord_arr_size = 0;
double *nrz_time = NULL;
bool isManchester = false;
bool finishThread = false;
bool canExit = true;
MessageInfo thread_message_info;

//Global concurrency controll variables
pthread_mutex_t thread_mutex;
pthread_cond_t thread_cond;

//Verifies if main window has been closed
gboolean main_window_status(GtkWidget* widget, gpointer gtk_data) {
	InfoWidgets* data_from_gtk = (InfoWidgets*) gtk_data;
	GtkWidget* main_thread_window = data_from_gtk->main_thread_window;

	pthread_mutex_lock(&thread_mutex);
	finishThread = true;	
	pthread_cond_signal(&thread_cond);
	pthread_mutex_unlock(&thread_mutex);

	if (canExit) {
		close(data_from_gtk->transmitter_socket);
		close(data_from_gtk->receptor_socket);
		gtk_window_close(GTK_WINDOW(data_from_gtk->receptor_window));
		free(data_from_gtk);	
		exit(0);
	}
}

void* receptor_execution(void* gtk_data) {
	//Widgets data
	InfoWidgets* data_from_gtk = (InfoWidgets*) gtk_data;

	//Receptor socket
	int receptor_socket = data_from_gtk->receptor_socket;
	
	//socket to receive data
	int returned_socket;

	//Accepts transmissor request
	returned_socket = accept(receptor_socket, NULL, NULL);
	canExit = false;
	
	//Binary structs
	ResultWithSize encoded_message_bytes;
	ResultWithSize message_with_frames;
	ResultWithSize raw_message_bits;
	ResultWithSize raw_message;
	
	//Flag
	bool isByte = true;

	//Received encoded message
	unsigned char encoded_message[131072];
	int encoded_message_length = 0;	
	
	//Label strings
	char label_bits_num_str[50];
	char label_decoded_message_str[50];

	//Setting label
	GtkWidget* label_bits_num = data_from_gtk->label_bits_num;
	GtkWidget* label_warning_1 = data_from_gtk->label_warning_1;
	GtkWidget* label_warning_2 = data_from_gtk->label_warning_2;
	GtkWidget* label_decoded_message = data_from_gtk->label_decoded_message;
	GtkWidget* receptor_window = data_from_gtk->receptor_window;	

	//While the main window hasn't been closed, the thread keeps running
	pthread_mutex_lock(&thread_mutex);
	while (true) {	
		//Flag
		isByte = true;
	
		//Wait for the message convention
		pthread_cond_wait(&thread_cond, &thread_mutex);
		
		if (finishThread) {
			break;
		}

		encoded_message_length = recv(returned_socket, &encoded_message, sizeof(encoded_message), 0);
		
		//Simulates error during transmission
		generateError(encoded_message, encoded_message_length, (int) thread_message_info.error_probability);
		
		//Print
		sprintf(label_bits_num_str, "Received %d bits", encoded_message_length);
		
		//Error detection method		
		switch(thread_message_info.error_detection_type) {
			case 0:
				message_with_frames = verifyErrorByParity(encoded_message, encoded_message_length);			
				break;
			case 1:
				message_with_frames = verifyErrorByCrc(encoded_message, encoded_message_length);
				break;
			case 2:
				message_with_frames = verifyErrorByHamming(encoded_message, encoded_message_length);
				break;
		}
		
		//Framing method
		if (message_with_frames.result_message != NULL) {
			encoded_message_bytes = convertToByteStream(message_with_frames.result_message, message_with_frames.result_message_length);
			
			switch(thread_message_info.framing_type) {
				case 0:
					raw_message = getMessageFromCharacterFrame(encoded_message_bytes.result_message, encoded_message_bytes.result_message_length);	
					break;
				case 1:	
					raw_message = getMessageFromByteFrame(encoded_message_bytes.result_message, encoded_message_bytes.result_message_length);
					break;
				case 2:
					isByte = false;
					raw_message_bits = getMessageFromBitFrame(message_with_frames.result_message, message_with_frames.result_message_length);
					break;
			}
			
			gtk_label_set_text(GTK_LABEL(label_warning_1), "  ");
			free(encoded_message_bytes.result_message);
			free(message_with_frames.result_message);	
		}else{
			gtk_label_set_text(GTK_LABEL(label_decoded_message), "  ");
			gtk_label_set_text(GTK_LABEL(label_warning_1), "An unfixable error has been detected");
		}
		
		if (!isByte) {
			raw_message = convertToByteStream(raw_message_bits.result_message, raw_message_bits.result_message_length);
			free(raw_message_bits.result_message);
		}

		if (raw_message.result_message != NULL) {
			gtk_label_set_text(GTK_LABEL(label_warning_2), "  ");	
			sprintf(label_decoded_message_str, "The decoded message is: %s", raw_message.result_message);
			gtk_label_set_text(GTK_LABEL(label_decoded_message), label_decoded_message_str);
			free(raw_message.result_message);	
		}else{
			gtk_label_set_text(GTK_LABEL(label_decoded_message), "  ");
			gtk_label_set_text(GTK_LABEL(label_warning_2), "Error in flag");	
		}

		gtk_label_set_text(GTK_LABEL(label_bits_num), label_bits_num_str);			
	}
	pthread_mutex_unlock(&thread_mutex);
	
	//Closes the receptor's window
	gtk_window_close(GTK_WINDOW(receptor_window));

	//Destroying concurrency controll variables
	pthread_mutex_destroy(&thread_mutex);
	pthread_cond_destroy(&thread_cond);

	//Closes sockets
	close(data_from_gtk->transmitter_socket);
	close(receptor_socket);
	close(returned_socket);

	//Frees memory
	free(data_from_gtk);

	exit(0);
}

void sock_send_message(unsigned char* raw_message, MessageInfo message_info, int transmitter_socket, GraphInfo graph_info) {
	unsigned char* message_with_frames_in_bits = NULL;
	unsigned char* raw_message_bits = NULL;
	ResultWithSize message_with_error_detection;
	ResultWithSize message_with_frames;

	int raw_message_size = strlen(raw_message) + 1;
	int raw_message_size_in_bits = raw_message_size*8;

	bool isInByte = true;


	switch (message_info.framing_type) {
		case 0:		
			message_with_frames = frameByCharacterCounting(raw_message, raw_message_size);
			break;
		case 1:			
			message_with_frames = frameByBytesFlag(raw_message, raw_message_size);
			break;
		case 2:
			isInByte = false;
			raw_message_bits = convertToBitStream(raw_message, raw_message_size);
			
			message_with_frames = frameByBitsFlag(raw_message_bits, raw_message_size_in_bits);
			free(raw_message_bits);
			break;
	}
	
	//If the framing is successfull, the message is sent
	if (message_with_frames.result_message != NULL) {	
		if (isInByte) {
			message_with_frames_in_bits = convertToBitStream(message_with_frames.result_message, message_with_frames.result_message_length);
			
			switch (message_info.error_detection_type) {
				case 0:
					message_with_error_detection = createErrorCheckBitParity(message_with_frames_in_bits, message_with_frames.result_message_length*8);
					break;
				case 1:
					message_with_error_detection = generateCrcErrorCheck(message_with_frames_in_bits, message_with_frames.result_message_length*8);
					break;
				case 2:
					message_with_error_detection = generateHammingErrorCorrection(message_with_frames_in_bits, message_with_frames.result_message_length*8);
					break;
			}
			
			free(message_with_frames.result_message);
		}else{	
			switch (message_info.error_detection_type) {
				case 0:
					message_with_error_detection = createErrorCheckBitParity(message_with_frames.result_message, message_with_frames.result_message_length);
					break;
				case 1:
					message_with_error_detection = generateCrcErrorCheck(message_with_frames.result_message, message_with_frames.result_message_length);
					break;
				case 2:
					message_with_error_detection = generateHammingErrorCorrection(message_with_frames.result_message, message_with_frames.result_message_length);
					break;
			}
			
		}
		
	}
	
	//If the error detection implementation is successfull, the message is sent
	if (message_with_error_detection.result_message != NULL) {
		send(transmitter_socket, message_with_error_detection.result_message, message_with_error_detection.result_message_length, 0);
	}
	
	//Generates graph of the physics layer signals
	generateGraph(graph_info);

	//Deallocate memory
	free(message_with_error_detection.result_message);
	free(raw_message);			

	//Signals to receptors thread
	pthread_cond_signal(&thread_cond);
}

void draw_graph_carrier(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data) {
	SinCoordinate *data_to_draw = (SinCoordinate*) user_data;
	
	double xc = 0.0;
	double yc = 300.0;
	
	cairo_set_line_width(cr, 1.5);
	cairo_set_source_rgba(cr, 107.0, 221.0, 0.0, 1.0);

	//Horizontal axis
	cairo_move_to(cr, xc, yc);
	cairo_line_to(cr, (double)width, yc);
	
	cairo_stroke_preserve(cr);

	//Vertical axis
	cairo_move_to(cr, 1.5, 0.0);
	cairo_line_to(cr, xc, (double)height);

	cairo_stroke(cr);
	
	//Waves around x axis
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgba(cr, 255.0, 0.0, 0.0, 1.0);

	xc = 2.5;
	cairo_move_to(cr, xc, yc);

	int i=0;
	while (!data_to_draw[i].isLast) {
		cairo_line_to(cr, xc + data_to_draw[i].time, yc + data_to_draw[i].voltage);
		
		cairo_line_to(cr, xc + data_to_draw[i].time, yc + data_to_draw[i].voltage);
		
		i += 1;	
	}

	cairo_stroke(cr);

	//Draws numbers on the x axis
	xc = 25.0;
	yc = 290.0;
	char number_str[100];
	cairo_set_font_size(cr, 8.0);
	cairo_set_source_rgba(cr, 0.0, 160.0, 0.0, 1.0);

	if (!isManchester) {
		for (int i=0; i<digital_coord_arr_size; i++) {
			cairo_move_to(cr, xc, yc);

			sprintf(number_str, "%.2fms", nrz_time[i]*1000.0);
			cairo_show_text(cr, number_str);

			xc += 40;	
		}
	}else{
		for (int i=1; i<digital_coord_arr_size; i+=2) {
			cairo_move_to(cr, xc, yc);

			sprintf(number_str, "%.2fms", nrz_time[i]*1000.0);
			cairo_show_text(cr, number_str);

			xc += 40;	
		}
	}

	cairo_stroke(cr);

	//Draws numbers on the y axis
	xc = 12.0;
	yc = 253.0;
	int num = 52;

	for (int i=0; i<2; i++) {
		cairo_move_to(cr, xc, yc);

		sprintf(number_str, "%dV", num);
		cairo_show_text(cr, number_str);
		
		num -= 24;
		yc += 100;	
	}
	
	//Draws lines to set the numerical values on the x axis
	xc = 42.5;
	yc = 295.0;
	cairo_set_source_rgba(cr, 32.0, 93.0, 255.0, 1.0);

	if (!isManchester) {
		for (int i=0; i<digital_coord_arr_size; i++) {
			cairo_move_to(cr, xc, yc);

			cairo_line_to(cr, xc, yc + 10.0);

			xc += 40.0;
		}
	}else{
		for (int i=1; i<digital_coord_arr_size; i+=2) {
			cairo_move_to(cr, xc, yc);

			cairo_line_to(cr, xc, yc + 10.0);

			xc += 40.0;
		}
	}

	cairo_stroke(cr);
	
	//Draws lines to set the numerical values on the y axis
	
	xc = 0.1;
	yc = 250.0;

	for (int i=0; i<2; i++) {
		cairo_move_to(cr, xc, yc);

		cairo_line_to(cr, xc+10.0, yc);

		yc += 100;	
	}

	cairo_stroke(cr);

	free(nrz_time);
	free(data_to_draw);

	isManchester = false;
}


void draw_graph_digital(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data) {
	NRZCoordinate *data_to_draw = (NRZCoordinate*) user_data;
	
	double xc = 0.0;
	double yc = 300.0;
	
	cairo_set_line_width(cr, 1.5);
	cairo_set_source_rgba(cr, 107.0, 221.0, 0.0, 1.0);

	//Horizontal axis
	cairo_move_to(cr, xc, yc);
	cairo_line_to(cr, (double)width, yc);
	
	cairo_stroke_preserve(cr);

	//Vertical axis
	cairo_move_to(cr, 1.5, 0.0);
	cairo_line_to(cr, xc, (double)height);

	cairo_stroke(cr);
	
	//Waves around x axis
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgba(cr, 255.0, 0.0, 0.0, 1.0);

	xc = 2.5;
	cairo_move_to(cr, xc, yc);
	for (int i=0; i<digital_coord_arr_size; i++) {
		cairo_line_to(cr, xc, data_to_draw[i].voltage);
		
		xc += 40.0;
		cairo_line_to(cr, xc, data_to_draw[i].voltage);	
	}	

	cairo_stroke(cr);
	
	//Draws numbers on the x axis
	xc = 25.0;
	yc = 290.0;
	char number_str[100];
	cairo_set_font_size(cr, 8.0);
	cairo_set_source_rgba(cr, 0.0, 160.0, 0.0, 1.0);

	for (int i=0; i<digital_coord_arr_size; i++) {
		cairo_move_to(cr, xc, yc);

		sprintf(number_str, "%.2fms", data_to_draw[i].time*1000.0);
		cairo_show_text(cr, number_str);

		xc += 40;	
	}	

	cairo_stroke(cr);

	//Draws numbers on the y axis
	xc = 12.0;
	yc = 253.0;
	int num = 52;

	for (int i=0; i<2; i++) {
		cairo_move_to(cr, xc, yc);

		sprintf(number_str, "%dV", num);
		cairo_show_text(cr, number_str);
		
		num -= 24;
		yc += 100;	
	}

	cairo_stroke(cr);
	
	//Draws lines to set the numerical values on the x axis	
	xc = 42.5;
	yc = 295.0;
	cairo_set_source_rgba(cr, 32.0, 93.0, 255.0, 1.0);
	
	for (int i=0; i<digital_coord_arr_size; i++) {
		cairo_move_to(cr, xc, yc);

		cairo_line_to(cr, xc, yc + 10.0);

		xc += 40.0;
	}

	cairo_stroke(cr);

	//Draws lines to set the numerical values on the y axis
	
	xc = 0.1;
	yc = 250.0;

	for (int i=0; i<2; i++) {
		cairo_move_to(cr, xc, yc);

		cairo_line_to(cr, xc+10.0, yc);

		yc += 100;	
	}

	cairo_stroke(cr);

	free(data_to_draw);
}

//Send button callback that generates a wave graph
void generateGraph(GraphInfo graph_info) { 		
	NRZCoordinate *digital_coordinates;
	digital_coord_arr_size = 0;

	SinCoordinate *carrier_coordinates;

	switch (graph_info.digital_mod_type) {
		case 0:
			digital_coordinates = generateNrzPolarLSignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*8;
			break;	
		case 1:
			digital_coordinates = generateNrzPolarISignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*8;
			break;	
		case 2:
			digital_coordinates = generateNrzManchesterSignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*16;

			isManchester = true;
			break;	
		case 3:
			digital_coordinates = generateNrzDifferencialManchesterSignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*16;

			isManchester = true;
			break;	
		case 4:
			digital_coordinates = generateNrzBipolarAMISignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*8;
			break;	
		case 5:
			digital_coordinates = generateNrzBipolarPseudoternarySignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*8;
			break;	
	}
	
	nrz_time = (double*) malloc(sizeof(double)*digital_coord_arr_size);

	for (int i=0; i<digital_coord_arr_size; i++) {
		nrz_time[i] = digital_coordinates[i].time;
	}

	switch (graph_info.carrier_mod_type) {
		case 0:
			carrier_coordinates = generateAskModulation(graph_info.message_str,
							graph_info.bandwidth,
							strlen(graph_info.message_str));	
			break;
		case 1:

			carrier_coordinates = generateFskModulation(graph_info.message_str,
							graph_info.bandwidth,
							strlen(graph_info.message_str));
			break;

		case 2:

			carrier_coordinates = generate8qamModulation(graph_info.message_str,
							graph_info.bandwidth,
							strlen(graph_info.message_str));
			break;
	}
	
	//Digital modulation window definition
	GtkWidget *digital_graph_window = gtk_window_new();
	GtkWidget *digital_drawing_area = gtk_drawing_area_new();
	GtkWidget *digital_scrolled_window = gtk_scrolled_window_new();
	GtkAdjustment *digital_scroll_adjustment;

	//Digital modulation scrolled window configuration
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(digital_scrolled_window), digital_drawing_area);
	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(digital_drawing_area), 10000); 
	

	//Digital modulation window configuration
	gtk_window_set_title(GTK_WINDOW(digital_graph_window), "Modulated digital signal");
	gtk_window_set_default_size(GTK_WINDOW(digital_graph_window), 900, 600);
	gtk_window_set_child(GTK_WINDOW(digital_graph_window), digital_scrolled_window);
	gtk_window_present(GTK_WINDOW(digital_graph_window));

	//Digital modulation drawing area configuration
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(digital_drawing_area), draw_graph_digital, digital_coordinates, NULL);

	//Carrier modulation window definition
	GtkWidget *carrier_graph_window = gtk_window_new();
	GtkWidget *carrier_drawing_area = gtk_drawing_area_new();
	GtkWidget *carrier_scrolled_window = gtk_scrolled_window_new();
	GtkAdjustment *carrier_scroll_adjustment;

	//Carrier modulation scrolled window configuration
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(carrier_scrolled_window), carrier_drawing_area);
	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(carrier_drawing_area), 10000); 
	

	//Carrier modulation window configuration
	gtk_window_set_title(GTK_WINDOW(carrier_graph_window), "Modulated carrier signal");
	gtk_window_set_default_size(GTK_WINDOW(carrier_graph_window), 900, 600);
	gtk_window_set_child(GTK_WINDOW(carrier_graph_window), carrier_scrolled_window);
	gtk_window_present(GTK_WINDOW(carrier_graph_window));

	//Carrier modulation drawing area configuration
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(carrier_drawing_area), draw_graph_carrier, carrier_coordinates, NULL);

	//Deallocating memory
	free(graph_info.message_str);
}

gboolean sendButtonClick(GtkWidget *widget, gpointer user_data) {
	InfoWidgets *data = (InfoWidgets*) user_data;
	GtkWidget *text = (GtkWidget*) data->text_field;
	GtkWidget **check_array = (GtkWidget**) data->check_button;
	GtkWidget *band_spin = (GtkWidget*) data->band_spin_button;
	GtkWidget *noise_spin = (GtkWidget*) data->noise_spin_button;
	GtkWidget *error_spin = (GtkWidget*) data->error_prob_spin_button;

	int d_value = 0;
	int c_value = 0;
	int error_value = 0;
	int frame_value = 0;
	
	//If the tranasmitter's socket isn't connected to the receptor's socket, the connection is made
	while (data->connection_status == -1) {

		//Connecting the transmitter's socket to the receptor's socket
		data->connection_status = connect(data->transmitter_socket, (struct sockaddr*) &data->transmitter_address, sizeof(data->transmitter_address));
		
		if (data->connection_status == -1) {
			g_print("Error: %s\n", strerror(errno));
			close(data->transmitter_socket);

			data->transmitter_socket = socket(AF_INET, SOCK_STREAM, 0);	
		}
	}

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));	
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	
	gtk_text_buffer_get_bounds(buffer, &start_iter, &end_iter);
	
	for (int i=0; i<15; i++) {
		if (i<6) {
			if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_array[i]))) {
				d_value = i;
			}
		}else if (i<9) {
			if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_array[i]))) {
				c_value = i-6;
			}
		}else if (i<12) {
			if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_array[i]))) {
				frame_value = i-9;
			} 
		}else{
			if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_array[i]))) {
				error_value = i-12;
			}
		}
	}
	
	GraphInfo graph_info = {
		gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE),
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(noise_spin)),
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(band_spin)),
		d_value,
		c_value
	};

	MessageInfo message_info = {
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(error_spin)),
		error_value,
		frame_value
	};
	
	//Message config
	pthread_mutex_lock(&thread_mutex);
	thread_message_info = message_info;
	pthread_mutex_unlock(&thread_mutex);	
	
	sock_send_message(gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE), message_info, data->transmitter_socket, graph_info);
	return true;
}

//This function executes when the gtk app object emits an 'activate' signal
void activate(GtkApplication *app, gpointer user_data) {
	//Declaring thread instances
	pthread_t receptor_thread;
	
	//Declaring window
	GtkWidget *main_window;

	//Declaring containers
	GtkWidget *main_vertical_container;
	GtkWidget *vertical_check_box_container[15];
	GtkWidget *main_check_box_digital_container;
	GtkWidget *main_check_box_carrier_container;
	GtkWidget *noise_spin_button_horizontal_container;
	GtkWidget *main_check_box_error_detection_container;
	GtkWidget *main_check_box_frame_container;
	
	//Declaring text field
	GtkWidget *text_field;
	
	//Declaring buttons
	GtkAdjustment *noise_spin_button_adjustment;
	GtkAdjustment *band_spin_button_adjustment;
	GtkAdjustment *error_prob_spin_button_adjustment;
	GtkWidget *noise_spin_button;
	GtkWidget *band_spin_button;
	GtkWidget *error_prob_spin_button;
	GtkWidget *send_button;
	GtkWidget *check_box[15];

	//Declaring text field's and spin button's label
	GtkWidget *text_field_label;
	GtkWidget *spin_button_label_horizontal;
	GtkWidget *spin_button_label_vertical;
	GtkWidget *band_spin_button_label;
	GtkWidget *error_prob_spin_button_label;

	//Declaring check box row's label
	GtkWidget *check_box_digital_label;
	GtkWidget *check_box_carrier_label;
	GtkWidget *check_box_error_detection_label;
	GtkWidget *check_box_frame_label;

	//Declaring check box's label containing digital modulation types
	GtkWidget *digital_type_label[6];
	
	//Declaring check box's label containing carrier modulation types
	GtkWidget *carrier_type_label[3];
	
	//Declaring check box's label conatining frame types
	GtkWidget *frame_type_label[3];

	//Declaring check box's label conatining frame types
	GtkWidget *error_detection_type_label[3];

	//Initializing window and its components
	main_window = gtk_application_window_new(app);
	
	noise_spin_button_horizontal_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	main_vertical_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	main_check_box_digital_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 35);
	main_check_box_carrier_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 210);
	main_check_box_error_detection_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 175);
	main_check_box_frame_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 170); 

	text_field = gtk_text_view_new();
	text_field_label = gtk_label_new("Type a message");
	
	noise_spin_button_adjustment = gtk_adjustment_new(1000.0, 100.0, 1744.0, 1.0, 0.1, 0.0);
	band_spin_button_adjustment = gtk_adjustment_new(1000.0, 1.0, 1000000000000.0, 1.0, 0.1, 0.0);
	error_prob_spin_button_adjustment = gtk_adjustment_new(0.0, 0.0, 100.0, 1.0, 0.1, 0.0);
	noise_spin_button = gtk_spin_button_new(noise_spin_button_adjustment, 10.0, 3);
	band_spin_button = gtk_spin_button_new(band_spin_button_adjustment, 10.0, 3);
	error_prob_spin_button = gtk_spin_button_new(error_prob_spin_button_adjustment, 10.0, 3);
	
	send_button = gtk_button_new_with_label("Send message");
	
	spin_button_label_horizontal = gtk_label_new("Watt");
	band_spin_button_label = gtk_label_new("Hz");
	error_prob_spin_button_label = gtk_label_new("%");
	spin_button_label_vertical = gtk_label_new("Define the channel's noise power, the bandwidth's frequency and the error probability");
	
	check_box_digital_label = gtk_label_new("Choose one type of digital modulation");
	check_box_carrier_label = gtk_label_new("Choose one type of carrier modulation");
	check_box_frame_label = gtk_label_new("Choose one type of framing method");
	check_box_error_detection_label = gtk_label_new("Choose one type of error detection method");

	for (int i=0; i<15; i++) {
		vertical_check_box_container[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
		check_box[i] = gtk_check_button_new();
	}

	for (int i=0; i<6; i++) {
		digital_type_label[i] = gtk_label_new(NULL);
	}

	for (int i=0; i<3; i++) {
		carrier_type_label[i] = gtk_label_new(NULL);
	}

	for (int i=0; i<3; i++) {
		frame_type_label[i] = gtk_label_new(NULL);
	}

	for (int i=0; i<3; i++) {
		error_detection_type_label[i] = gtk_label_new(NULL);
	}
	
	//Setting the modulation type's label
	gtk_label_set_label(GTK_LABEL(digital_type_label[0]), "NRZ-L");
	gtk_label_set_label(GTK_LABEL(digital_type_label[1]), "NRZ-I");
	gtk_label_set_label(GTK_LABEL(digital_type_label[2]), "Manchester");
	gtk_label_set_label(GTK_LABEL(digital_type_label[3]), "D.Manchester");
	gtk_label_set_label(GTK_LABEL(digital_type_label[4]), "AMI");
	gtk_label_set_label(GTK_LABEL(digital_type_label[5]), "Pseudoternary");

	gtk_label_set_label(GTK_LABEL(carrier_type_label[0]), "ASK");
	gtk_label_set_label(GTK_LABEL(carrier_type_label[1]), "FSK");
	gtk_label_set_label(GTK_LABEL(carrier_type_label[2]), "8-QAM");

	gtk_label_set_label(GTK_LABEL(frame_type_label[0]), "Char count");
	gtk_label_set_label(GTK_LABEL(frame_type_label[1]), "Bytes flag");
	gtk_label_set_label(GTK_LABEL(frame_type_label[2]), "Bits flag");

	gtk_label_set_label(GTK_LABEL(error_detection_type_label[0]), "Parity bit");
	gtk_label_set_label(GTK_LABEL(error_detection_type_label[1]), "CRC");
	gtk_label_set_label(GTK_LABEL(error_detection_type_label[2]), "Hamming code");

	//Appending components to their respective containers
	for (int i=0; i<15; i++) {
		if (i<6) {
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), check_box[i]);
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), digital_type_label[i]);
		}else if (i<9){
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), check_box[i]);
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), carrier_type_label[i-6]);
		}else if (i<12){
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), check_box[i]);
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), frame_type_label[i-9]);
		}else{
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), check_box[i]);
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), error_detection_type_label[i-12]);
		}
	}
	
	for (int i=0; i<15; i++) {
		if (i<6) {
			gtk_box_append(GTK_BOX(main_check_box_digital_container), vertical_check_box_container[i]);
		}else if (i<9){
			gtk_box_append(GTK_BOX(main_check_box_carrier_container), vertical_check_box_container[i]);
		}else if (i<12){
			gtk_box_append(GTK_BOX(main_check_box_frame_container), vertical_check_box_container[i]);
		}else{
			gtk_box_append(GTK_BOX(main_check_box_error_detection_container), vertical_check_box_container[i]);
		}
	}

	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), noise_spin_button);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), spin_button_label_horizontal);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), band_spin_button);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), band_spin_button_label);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), error_prob_spin_button);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), error_prob_spin_button_label);

	gtk_box_append(GTK_BOX(main_vertical_container), text_field_label);
	gtk_box_append(GTK_BOX(main_vertical_container), text_field);
	gtk_box_append(GTK_BOX(main_vertical_container), spin_button_label_vertical);
	gtk_box_append(GTK_BOX(main_vertical_container), noise_spin_button_horizontal_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_digital_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_digital_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_carrier_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_carrier_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_frame_label);		
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_frame_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_error_detection_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_error_detection_container);
	gtk_box_append(GTK_BOX(main_vertical_container), send_button);
		
	
	//Setting check button's group
	for (int i=0; i<15; i++) {
		if (i<6 && i!=0) {
			gtk_check_button_set_group(GTK_CHECK_BUTTON(check_box[i]), GTK_CHECK_BUTTON(check_box[0]));
		}else if (i>6 && i<9) {
			gtk_check_button_set_group(GTK_CHECK_BUTTON(check_box[i]), GTK_CHECK_BUTTON(check_box[6]));
		}else if (i>9 && i<12) {
			gtk_check_button_set_group(GTK_CHECK_BUTTON(check_box[i]), GTK_CHECK_BUTTON(check_box[9]));
		}else if (i>12) {
			gtk_check_button_set_group(GTK_CHECK_BUTTON(check_box[i]), GTK_CHECK_BUTTON(check_box[12]));
		}
	}	

	//Receptor's socket
	int receptor_socket = -1; 
	receptor_socket = socket(AF_INET, SOCK_STREAM, 0);

	//Address of the receptor's socket
	struct sockaddr_in receptor_address;
	receptor_address.sin_family = AF_INET;
	receptor_address.sin_port = htons(8000);
	receptor_address.sin_addr.s_addr = INADDR_ANY;

	//Binding the receptor's socket to the address and listen for connections
	if (bind(receptor_socket, (struct sockaddr*) &receptor_address, sizeof(receptor_address)) == -1) {
		g_print("%s\n", strerror(errno));
		close(receptor_socket);
		exit(1);
	}

	if (listen(receptor_socket, 1) == -1) {
		g_print("%s\n", strerror(errno));
		close(receptor_socket);
		exit(1);
	}

	//Transmitter's socket
	int transmitter_socket = -1;
	transmitter_socket = socket(AF_INET, SOCK_STREAM, 0);

	//Transmitter's address
	struct sockaddr_in transmitter_address;
	transmitter_address.sin_family = AF_INET;
	transmitter_address.sin_port = htons(8000);
	transmitter_address.sin_addr.s_addr = INADDR_ANY;	
	
	//GTK widgets for receptor's window
	GtkWidget* receptor_window = gtk_window_new();
	
	GtkWidget* title_label = gtk_label_new("RECEPTOR'S CONSOLE");
	GtkWidget* label_bits_num = gtk_label_new("");
	GtkWidget* label_warning_1 = gtk_label_new("");
	GtkWidget* label_warning_2 = gtk_label_new("");
	GtkWidget* label_decoded_message = gtk_label_new("");

	GtkWidget* vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	//Appending to the box and window
	gtk_box_append(GTK_BOX(vertical_box), title_label);
	gtk_box_append(GTK_BOX(vertical_box), label_bits_num);
	gtk_box_append(GTK_BOX(vertical_box), label_warning_1);
	gtk_box_append(GTK_BOX(vertical_box), label_warning_2);
	gtk_box_append(GTK_BOX(vertical_box), label_decoded_message);
	
	gtk_window_set_deletable(GTK_WINDOW(receptor_window), FALSE);
	gtk_window_set_title(GTK_WINDOW(receptor_window), "Receptor window");
	gtk_window_set_default_size(GTK_WINDOW(receptor_window), 300, 300);
	gtk_window_set_child(GTK_WINDOW(receptor_window), vertical_box);	

	//Setting function's data
	
	InfoWidgets *data = (InfoWidgets*) malloc(sizeof(InfoWidgets));

	data->text_field = text_field;

	for (int i=0; i<15; i++) {
		data->check_button[i] = check_box[i];
	}

	data->noise_spin_button = noise_spin_button;
	data->band_spin_button = band_spin_button;
	data->error_prob_spin_button = error_prob_spin_button;
	data->main_thread_window = main_window;
	data->receptor_socket = receptor_socket;
	data->transmitter_socket = transmitter_socket;
	data->connection_status = -1;
	data->transmitter_address = transmitter_address;
	data->label_bits_num = label_bits_num;
	data->label_warning_1 = label_warning_1;
	data->label_warning_2 = label_warning_2;
	data->label_decoded_message = label_decoded_message;
	data->receptor_window = receptor_window;	
	
	//Initializing concurrency controll variables
	pthread_mutex_init(&thread_mutex, NULL);
	pthread_cond_init(&thread_cond, NULL);

	//Creating receptror's thread routine
	if (pthread_create(&receptor_thread, NULL, &receptor_execution, (void*) data) != 0) {
		g_print("Error creating thread");
		close(transmitter_socket);
		close(receptor_socket);
		exit(1);
	}
	
	//Setting send button callback function
	g_signal_connect(send_button, "clicked", G_CALLBACK(sendButtonClick), data);
	g_signal_connect_after(main_window, "destroy", G_CALLBACK(main_window_status), data);

	//Setting window's configuration
	gtk_window_set_title(GTK_WINDOW(main_window), "Telecom simulation");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 300, 600);

	gtk_window_set_child(GTK_WINDOW(main_window), main_vertical_container);
	
	//Display window to the user
	gtk_window_present(GTK_WINDOW(main_window));
	gtk_window_present(GTK_WINDOW(receptor_window));
}
