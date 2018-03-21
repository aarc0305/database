#include <stdio.h>
#include <stdlib.h>
#include <string.h> /*strcmp() needs this library.*/

// InputBuffer save the message from the terminal
typedef struct InputBuffer_t {
	char* buffer;
	size_t buffer_length; // Because the second parameter of getline is a *size_t variable
	ssize_t input_length;
} InputBuffer;

// The function to create the new input buffer
InputBuffer* new_input() {
	InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
	input_buffer -> buffer = NULL;
	input_buffer -> buffer_length = 0;
	input_buffer -> input_length = 0;
	return input_buffer;
}

void print_prompt() {
	printf("db > ");
}

// read the message from the termianl and save it in the input_buffer
void read_input(InputBuffer* input_buffer) {
	ssize_t read_bytes = getline(&(input_buffer -> buffer), &(input_buffer -> buffer_length), stdin);
	if (read_bytes <= 0) {
		printf("Fail to read from the terminal!\n");
		return;
	}
	input_buffer -> input_length = read_bytes - 1;
	input_buffer -> buffer[read_bytes - 1] = 0; 
}

int main() {
	InputBuffer* input_buffer = new_input();
	for (;;) {
		print_prompt();
		read_input(input_buffer);

		if (strcmp(input_buffer -> buffer, ".exit") == 0) {
			printf("Bye bye!\n");
			return 0;
		} else {
			printf("Wrong format!\n");
		}
	}
}