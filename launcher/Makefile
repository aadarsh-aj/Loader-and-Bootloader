CC = gcc
CFLAGS = -m32 -Wall
LOADER_DIR = ../loader
BIN_DIR = ../bin
TARGET = $(BIN_DIR)/launch

all: $(TARGET)

$(TARGET): launch.c $(LOADER_DIR)/loader.h $(BIN_DIR)/lib_simpleloader.so
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) launch.c -I$(LOADER_DIR) -L$(BIN_DIR) -l_simpleloader -Wl,-rpath,'$$ORIGIN'

clean:
	rm -f $(TARGET)