CC = gcc
LDFLAGS = -lglut -lGLU -lGL -lm -lrt -pthread
OBJECTS = parent helper sender display receiver encoders spy master
EXECUTABLE = main

.PHONY: all clean run

all: $(OBJECTS)

parent: parent.c
	$(CC) $< -o $@ $(LDFLAGS)

helper: helper.c
	$(CC) $< -o $@ $(LDFLAGS)

sender: sender.c
	$(CC) $< -o $@ $(LDFLAGS)

display: display.c
	$(CC) $< -o $@ $(LDFLAGS)
receiver: receiver.c
	$(CC) $< -o $@ $(LDFLAGS)
encoders: encoders.c
	$(CC) $< -o $@ $(LDFLAGS)
spy: spy.c
	$(CC) $< -o $@ $(LDFLAGS)
master: master.c
	$(CC) $< -o $@ $(LDFLAGS)
clean:
	rm -f $(OBJECTS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(ARGS)
