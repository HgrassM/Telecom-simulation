#ifndef GTK_MODULE_H
#define GTK_MODULE_H

typedef struct {
	GtkWidget *text_field;
	GtkWidget *check_button[15];
	GtkWidget *noise_spin_button;
	GtkWidget *band_spin_button;
	GtkWidget *error_prob_spin_button;
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

void draw_graph_digital(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

void draw_graph_carrier(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

gboolean sendButtonClick(GtkWidget *widget, gpointer user_data);

void activate(GtkApplication *app, gpointer user_data);

#endif
