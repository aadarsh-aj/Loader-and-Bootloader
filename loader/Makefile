CC = gcc
CFLAGS = -m32 -fPIC -Wall
BIN_DIR = ../bin
TARGET = $(BIN_DIR)/lib_simpleloader.so

all: $(TARGET)

$(TARGET): loader.c loader.h
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -shared -Wl,-soname,lib_simpleloader.so -o $(TARGET) loader.c

clean:
	rm -f $(TARGET)
