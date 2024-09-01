telecomSim: src/main.c
	gcc `pkg-config --cflags gtk4` src/bytesmodule.c src/nonmodular.c src/carriermodule.c src/gtkmodule.c src/main.c  -o bin/telecomSim `pkg-config --libs gtk4` -lm -pthread

