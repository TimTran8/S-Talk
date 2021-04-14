TARGET = sender.o receiver.o setup.o s-talk.o
CXX = gcc
CFLAGS = -W -Wall -Wpedantic -g -D _POSIX_C_SOURCE=200809L -pthread

all: clean build

build: $(TARGET)
	$(CXX) $(CFLAGS) $(TARGET) instructorList.o -o s-talk 

$(TARGET): %.o: %.c
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -f s-talk sender.o receiver.o setup.o s-talk.o