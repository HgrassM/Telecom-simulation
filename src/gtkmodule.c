#include <gtk/gtk.h>

//This function executes when the gtk app object is activated
static void activate(GtkApplication *app, gpointer uder_data) {
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
	GtkWidget *noise_spin_button;
	GtkWidget *send_button;
	GtkWidget *check_box[9];

	//Declaring text field's and spin button's label
	GtkWidget *text_field_label;
	GtkWidget *spin_button_label_horizontal;
	GtkWidget *spin_button_label_vertical;

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
	
	noise_spin_button_adjustment = gtk_adjustment_new(1000.0, 1.0, 1000000000.0, 1.0, 0.1, 0.0);
	noise_spin_button = gtk_spin_button_new(noise_spin_button_adjustment, 10.0, 3);
	
	send_button = gtk_button_new_with_label("Send message");
	
	spin_button_label_horizontal = gtk_label_new("Watt");
	spin_button_label_vertical = gtk_label_new("Define the channel's noise power");
	
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

	gtk_box_append(GTK_BOX(main_vertical_container), text_field_label);
	gtk_box_append(GTK_BOX(main_vertical_container), text_field);
	gtk_box_append(GTK_BOX(main_vertical_container), spin_button_label_vertical);
	gtk_box_append(GTK_BOX(main_vertical_container), noise_spin_button_horizontal_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_digital_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_digital_container);
	gtk_box_append(GTK_BOX(main_vertical_container), check_box_carrier_label);
	gtk_box_append(GTK_BOX(main_vertical_container), main_check_box_carrier_container);
	gtk_box_append(GTK_BOX(main_vertical_container), send_button);

	//Setting window's configuration
	gtk_window_set_title(GTK_WINDOW(main_window), "Telecom simulation");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 300, 600);

	gtk_window_set_child(GTK_WINDOW(main_window), main_vertical_container);

	gtk_window_present(GTK_WINDOW(main_window));
}
