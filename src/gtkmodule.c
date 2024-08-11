#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "header/nonmodular.h"
#include "header/gtkmodule.h"
#include "header/carriermodule.h"

static int digital_coord_arr_size;

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
	free(data_to_draw);
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
		
		xc += 10.0;
		cairo_line_to(cr, xc, data_to_draw[i].voltage);	
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
			break;	
		case 3:
			digital_coordinates = generateNrzDifferencialManchesterSignal(graph_info.message_str, 
							strlen(graph_info.message_str),
							graph_info.bandwidth,
							300.0,
							true,
							graph_info.noise_power);
			digital_coord_arr_size = strlen(graph_info.message_str)*16;
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

}

gboolean sendButtonClick(GtkWidget *widget, gpointer user_data) {
	InfoWidgets *data = (InfoWidgets*) user_data;
	GtkWidget *text = (GtkWidget*) data->text_field;
	GtkWidget **check_array = (GtkWidget**) data->check_button;
	GtkWidget *band_spin = (GtkWidget*) data->band_spin_button;
	GtkWidget *noise_spin = (GtkWidget*) data->noise_spin_button;

	int d_value = 0;
	int c_value = 0;

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));	
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	
	gtk_text_buffer_get_bounds(buffer, &start_iter, &end_iter);
	
	for (int i=0; i<9; i++) {
		if (i<6) {
			if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_array[i]))) {
				d_value = i;
			}
		}else{
			if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_array[i]))) {
				c_value = i-6;
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
	
	generateGraph(graph_info);
}

//This function executes when the gtk app object emits an 'activate' signal
void activate(GtkApplication *app, gpointer user_data) {
	//Declaring window
	GtkWidget *main_window;

	//Declaring containers
	GtkWidget *main_vertical_container;
	GtkWidget *vertical_check_box_container[9];
	GtkWidget *main_check_box_digital_container;
	GtkWidget *main_check_box_carrier_container;
	GtkWidget *noise_spin_button_horizontal_container;
	
	//Declaring text field
	GtkWidget *text_field;
	
	//Declaring buttons
	GtkAdjustment *noise_spin_button_adjustment;
	GtkAdjustment *band_spin_button_adjustment;
	GtkWidget *noise_spin_button;
	GtkWidget *band_spin_button;
	GtkWidget *send_button;
	GtkWidget *check_box[9];

	//Declaring text field's and spin button's label
	GtkWidget *text_field_label;
	GtkWidget *spin_button_label_horizontal;
	GtkWidget *spin_button_label_vertical;
	GtkWidget *band_spin_button_label;

	//Declaring check box row's label
	GtkWidget *check_box_digital_label;
	GtkWidget *check_box_carrier_label;

	//Declaring check box's label containing digital modulation types
	GtkWidget *digital_type_label[6];
	
	//Declaring check box's label containing carrier modulation types
	GtkWidget *carrier_type_label[3];
		
	//Initializing window and its components
	main_window = gtk_application_window_new(app);
	
	noise_spin_button_horizontal_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	main_vertical_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	main_check_box_digital_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	main_check_box_carrier_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);

	text_field = gtk_text_view_new();
	text_field_label = gtk_label_new("Type a message");
	
	noise_spin_button_adjustment = gtk_adjustment_new(1000.0, 1.0, 1000000000000.0, 1.0, 0.1, 0.0);
	band_spin_button_adjustment = gtk_adjustment_new(1000.0, 1.0, 1000000000000.0, 1.0, 0.1, 0.0); 
	noise_spin_button = gtk_spin_button_new(noise_spin_button_adjustment, 10.0, 3);
	band_spin_button = gtk_spin_button_new(band_spin_button_adjustment, 10.0, 3);
	
	send_button = gtk_button_new_with_label("Send message");
	
	spin_button_label_horizontal = gtk_label_new("Watt");
	band_spin_button_label = gtk_label_new("Hz");
	spin_button_label_vertical = gtk_label_new("Define the channel's noise power and the bandwidth's frequency");
	
	check_box_digital_label = gtk_label_new("Choose one type of digital modulation");
	check_box_carrier_label = gtk_label_new("Choose one type of carrier modulation");

	for (int i=0; i<9; i++) {
		vertical_check_box_container[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
		check_box[i] = gtk_check_button_new();
	}

	for (int i=0; i<6; i++) {
		digital_type_label[i] = gtk_label_new(NULL);
	}

	for (int i=0; i<3; i++) {
		carrier_type_label[i] = gtk_label_new(NULL);
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

	//Appending components to their respective containers
	for (int i=0; i<9; i++) {
		if (i<6) {
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), check_box[i]);
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), digital_type_label[i]);
		}else{
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), check_box[i]);
			gtk_box_append(GTK_BOX(vertical_check_box_container[i]), carrier_type_label[i-6]);
		}
	}
	
	for (int i=0; i<9; i++) {
		if (i<6) {
			gtk_box_append(GTK_BOX(main_check_box_digital_container), vertical_check_box_container[i]);
		}else{
			gtk_box_append(GTK_BOX(main_check_box_carrier_container), vertical_check_box_container[i]);
		}
	}

	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), noise_spin_button);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), spin_button_label_horizontal);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), band_spin_button);
	gtk_box_append(GTK_BOX(noise_spin_button_horizontal_container), band_spin_button_label);

	gtk_box_append(GTK_BOX(main_vertical_container), text_field_label);
	gtk_box_append(GTK_BOX(main_vertical_container), text_field);
	gtk_box_append(GTK_BOX(main_vertical_container), spin_button_label_vertical);
	gtk_box_append(GTK_BOX(main_vertical_container), noise_spin_button_horizontal_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_digital_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_digital_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_carrier_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_carrier_container);
	gtk_box_append(GTK_BOX(main_vertical_container), send_button);
	
	//Setting check button's group
	for (int i=0; i<9; i++) {
		if (i<6 && i!=0) {
			gtk_check_button_set_group(GTK_CHECK_BUTTON(check_box[i]), GTK_CHECK_BUTTON(check_box[0]));
		}else if (i>6) {
			gtk_check_button_set_group(GTK_CHECK_BUTTON(check_box[i]), GTK_CHECK_BUTTON(check_box[6]));
		}
	}

	//Binding the callback functions to the widgets	
	
	InfoWidgets *data = (InfoWidgets*) malloc(sizeof(InfoWidgets));

	data->text_field = text_field;

	for (int i=0; i<9; i++) {
		data->check_button[i] = check_box[i];
	}

	data->noise_spin_button = noise_spin_button;
	data->band_spin_button = band_spin_button;

	g_signal_connect(send_button, "clicked", G_CALLBACK(sendButtonClick), data); 	

	//Setting window's configuration
	gtk_window_set_title(GTK_WINDOW(main_window), "Telecom simulation");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 300, 600);

	gtk_window_set_child(GTK_WINDOW(main_window), main_vertical_container);

	gtk_window_present(GTK_WINDOW(main_window));
}
