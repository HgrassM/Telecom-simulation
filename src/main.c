#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <pthread.h>

#include "header/gtkmodule.h"

int main(int argc, char *argv[]) {
	GtkApplication *app;
	int status;

	app = gtk_application_new("org.gtk.nomedoapp", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);


	pthread_exit(NULL);
}
