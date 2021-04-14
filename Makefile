TARGET = ./src/sender.o ./src/receiver.o ./src/setup.o ./src/s-talk.o
CXX = gcc
CFLAGS = -W -Wall -Wpedantic -g -D _POSIX_C_SOURCE=200809L -pthread

all: clean build

build: $(TARGET)
	$(CXX) $(CFLAGS) $(TARGET) ./src/instructorList.o -o s-talk 

sender.o: $(SENDER)
	
$(TARGET): %.o: %.c
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -f s-talk ./src/sender.o ./src/receiver.o ./src/setup.o ./src/s-talk.o