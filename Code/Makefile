CC=gcc
#CFLAGS+=-g
CFLAGS+=`pkg-config --cflags opencv`
LDFLAGS+=`pkg-config --libs opencv`

.PHONY: all clean

output: objects
	$(CC) -o scarecrow_robot *.o  $(LDFLAGS)

objects:
	$(CC) -c $(CFLAGS) src_robot/*.c src_img_proc/*.c

all: output

clean:
	rm -f *.o scarecrow_robot
