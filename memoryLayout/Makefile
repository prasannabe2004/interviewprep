CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -L. -lshared -Wl,-rpath,.

all: libshared.so main

# Create the Shared Library
libshared.so: libshared.c
	$(CC) $(CFLAGS) -shared -o libshared.so libshared.c

# Create the Main Executable
main: main.c libshared.so
	$(CC) $(CFLAGS) -o main main.c $(LDFLAGS)

clean:
	rm -f main libshared.so