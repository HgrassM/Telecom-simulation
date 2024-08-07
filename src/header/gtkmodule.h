#include "nonmodular.h"

#ifndef GTK_MODULE_H
#define GTK_MODULE_H

typedef struct {
	NRZCoordinate coordinates[16];
	int coord_arr_size;
}PointsData;

typedef struct {
	GtkWidget *text_field;
	GtkWidget *check_button[9];
	GtkWidget *noise_spin_button;
	GtkWidget *band_spin_button;
}InfoWidgets;

typedef struct {
	char *message_str;
	double noise_power;
	double bandwidth;
	int digital_mod_type;
	int carrier_mod_type;
}GraphInfo;

void draw_graph(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

gboolean sendButtonClick(GtkWidget *widget, gpointer user_data);

void activate(GtkApplication *app, gpointer user_data);

#endif
