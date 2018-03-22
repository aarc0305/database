#include <stdio.h>
#include <stdlib.h>
#include <string.h> /*strcmp() needs this library.*/
#include <stdint.h> /* uint32_t needs this library */

const uint32_t ID_SIZE = 4;
const uint32_t USERNAME_SIZE = 32;
const uint32_t EMAIL_SIZE = 255;
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = 4;
const uint32_t EMAIL_OFFSET = 36;
const uint32_t ROW_SIZE = 4 + 32 + 255; 
const uint32_t PAGE_SIZE = 4096;
const uint32_t TABLE_MAX_PAGES = 100;
const uint32_t TABLE_MAX_ROWS = 29100;
const uint32_t ROWS_PER_PAGE = 4096 / 291;

//Enum of the meta command state
typedef enum MetaCommandResult_t {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

//Enum of the prepare command state
typedef enum PrepareStatementResult_t {
	PREPARE_SUCCESS,
	PREPARE_SYNTAX_ERROR,	
	UNRECOGNIZED_PREPARE_COMMAND
} PrepareStatementResult;

// Enum of the result of execute
typedef enum ExecuteResult_t {
    	EXECUTE_SUCCESS,
	EXECUTE_TABLE_FULL	
} ExecuteResult;

// InputBuffer save the message from the terminal
typedef struct InputBuffer_t {
	char* buffer;
	size_t buffer_length; // Because the second parameter of getline is a *size_t variable
	ssize_t input_length;
} InputBuffer;

typedef enum StatementType_t {
	STATEMENT_INSERT,
	STATEMENT_SELECT
} StatementType;

typedef struct Row_t {
    uint32_t id;
    char username[32];
    char email[255];
} Row;

typedef struct Statement_t {
	StatementType type;
	Row row_to_insert;
} Statement;

typedef struct  Table_t {
    void* pages[100];
    uint32_t num_rows;
} Table;

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

// The function handling with the meta command
MetaCommandResult handleMetaCommand(InputBuffer* input_buffer) {
	if (strcmp(input_buffer -> buffer, ".exit") == 0) {
		printf("Bye bye!\n");
		exit(EXIT_SUCCESS);
	} else {
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

// The function preparing the statement
PrepareStatementResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
	if (strncmp(input_buffer -> buffer, "insert", 6) == 0) {
		// This statement is an insert statement
		statement -> type = STATEMENT_INSERT;
		printf("%s\n", input_buffer -> buffer);
    		int sscan_num = sscanf(input_buffer -> buffer, "insert %d %s %s", &(statement -> row_to_insert.id), statement -> row_to_insert.username, statement -> row_to_insert.email); 
		if (sscan_num < 3) {
			printf("%d\n", sscan_num);
  			printf("The format of insert command is not correct!\n");
			return PREPARE_SYNTAX_ERROR; 
		}
		return PREPARE_SUCCESS;
	} else if (strncmp(input_buffer -> buffer, "select", 6) == 0) {
		statement -> type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	} else {
		return UNRECOGNIZED_PREPARE_COMMAND;
	}
}

// Get the address to save the row
void* row_slot(Table* table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page  = table -> pages[page_num];
    if (!page) {
        table -> pages[page_num] = malloc(PAGE_SIZE);
        page = table -> pages[page_num];    
    }
    uint32_t row_offset = row_num / ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
} 
// Serialize
void serialize(Row* row, void* destination) {
    memcpy(destination + ID_OFFSET, &(row -> id), ID_SIZE); 
    memcpy(destination + USERNAME_OFFSET, row -> username, USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, row -> email, EMAIL_SIZE); 
}

// Execute insert
ExecuteResult execute_insert(Table* table, Statement* statement) {
    if (table -> num_rows >= TABLE_MAX_ROWS) {
        printf("The table is full\n");
        return EXECUTE_TABLE_FULL;
    }
    Row* row_to_insert = &(statement -> row_to_insert);
    serialize(row_to_insert, row_slot(table, table -> num_rows));
    table -> num_rows += 1;

    return EXECUTE_SUCCESS;

}

void execute_statement(Statement* statement) {
    switch (statement -> type) {
        case STATEMENT_INSERT:
            printf("Insert!\n");
            break;
	case STATEMENT_SELECT:
            printf("Select\n");
	    break;
    }
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

		if (input_buffer -> buffer[0] == '.') {
			switch (handleMetaCommand(input_buffer)) {
				case (META_COMMAND_SUCCESS):
					continue;
				case (META_COMMAND_UNRECOGNIZED_COMMAND):
					printf("Unrecognied meta command!\n");
					continue;
			}
		}
		Statement statement;
		if (prepare_statement(input_buffer, &statement) == PREPARE_SUCCESS) {
			execute_statement(&statement);
		} else {
			// Unrecognized prepare command
			printf("Unrecognized prepare command!\n");
			continue;			
		}
	}
}
