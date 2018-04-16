HEADERS = command_parser.h commands.h environment.h file_processing.h
OBJECTS = main.o variables.o command_parser.o commands.o environment.o file_processing.o

default: shell

%.o: %.c $(HEADERS)
	gcc -c $< -o $@ 

shell: $(OBJECTS)
	gcc $(OBJECTS) -o $@  

clean:
	-rm -f $(OBJECTS)
	-rm -f shell
