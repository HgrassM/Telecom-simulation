#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

#ifndef GTK_MODULE_H
#define GTK_MODULE_H

typedef struct {
	GtkWidget *text_field;
	GtkWidget *check_button[15];
	GtkWidget *noise_spin_button;
	GtkWidget *band_spin_button;
	GtkWidget *error_prob_spin_button;
	GtkWidget *main_thread_window;
	GtkWidget *receptor_window;
	GtkWidget *label_bits_num;
	GtkWidget *label_warning_1;
	GtkWidget *label_warning_2;
	GtkWidget *label_decoded_message;
	int receptor_socket;
	int transmitter_socket;
	int connection_status;
	struct sockaddr_in transmitter_address;
}InfoWidgets;

typedef struct {
	char *message_str;
	double noise_power;
	double bandwidth;
	int digital_mod_type;
	int carrier_mod_type;
}GraphInfo;

typedef struct {
	double error_probability;
	int error_detection_type;
	int framing_type;
}MessageInfo;

gboolean main_window_status(GtkWidget* widget, gpointer gtk_data);

void* receptor_execution(void* gtk_data);

void generateGraph(GraphInfo graph_info);

void sock_send_message(unsigned char* raw_message, MessageInfo message_info, int transmitter_socket, GraphInfo graph_info);

void draw_graph_digital(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

void draw_graph_carrier(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

gboolean sendButtonClick(GtkWidget *widget, gpointer user_data);

void activate(GtkApplication *app, gpointer user_data);

#endif
