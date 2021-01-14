#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

//#define DEBUG
#define MAX_COMMANDS 1024 
#define MAX_ARGS 1024 

/*	Author: Chu-An Tsai
	Course: CSCI 503 Operating System
	This file is a simple shell parser which parses the command line users input and print out the result by different category of types
*/

// use enum to assign different situation of files
enum RED_OP {
  NONE = 0,
  RED_IN = 1,  // <
  RED_OUT = 2, // >
  APP_OUT = 3, // >>
};

// use struct to organize my group of different types of variables
struct command {
  char* args[MAX_ARGS];
  enum RED_OP red_op;
  char* file;
  int argc;
};

// clean up 
void free_commands(struct command** commands, int n_commands) {
  int i; 
  for (i = 0; i < n_commands; i++) {
  	int j;
    for (j = 0; j < commands[i]->argc; j++) {
      free(commands[i]->args[j]);
    }
    free(commands[i]->file);
    free(commands[i]);
  }
}


// To seperate commands by "|", " ", "<", ">", and ">>"
// Also check invalid input
struct command** parse_commands(char* input, int* n_commands) {
 
  char* cmd_saveptr;
  char* subcmd_saveptr;
  struct command** commands = calloc(MAX_COMMANDS, sizeof(struct command *));
  int cmd_count = 0;
  
  // start to split the command line
  // using '|' to split the command line and check
  char* cmd_str = strtok_r(input, "|", &cmd_saveptr);
 while (cmd_str != NULL)											
  {
    // if there's no commands before pip, it is an invalid input
    if (strlen(cmd_str) == 1)
    {
      printf("#error 4: pip with no proceeding command.");
      printf("strlen = %d, %x\n", (int)strlen(cmd_str), *cmd_str);
      return NULL;
    }
 
#ifdef DEBUG
	printf("%s\n", cmd_str);
#endif
    struct command* cmd = calloc(1, sizeof(struct command));
    cmd->argc = 0;
    // using '' to split the command line and check
    char* args = strtok_r(cmd_str, " ", &subcmd_saveptr);
    while (args != NULL)  
    {
      // check if it is '<' or not    
      if (!strcmp(args, "<"))  
      {
      	// if there's no commands before "<", , it is an invalid input
        if (cmd->argc == 0)
        {
        	printf("#error: \"<\" with no proceeding command.");
        	return NULL;
        }
        cmd->red_op = RED_IN;
        args = strtok_r(NULL, " ", &subcmd_saveptr);
        // if there's no filename after "<", , it is an invalid input
        if (args == NULL)
        {
          printf("#error: \"<\" is not followed by a filename.");
        	return NULL;
        }
        cmd->file = calloc(strlen(args) + 1, sizeof(char));
        strcpy(cmd->file, args);
      }
      // check if it is '>' or not
      else if (!strcmp(args, ">"))  
      {
      	// if there's no commands before ">", , it is an invalid input
        if (cmd->argc == 0)
        {
        	printf("#error: \">\" with no proceeding command.");
        	return NULL;
        }
        cmd->red_op = RED_OUT;
        args = strtok_r(NULL, " ", &subcmd_saveptr);
        // if there's no filename after ">", , it is an invalid input
        if (args == NULL)
        {
          printf("#errors: \">\" not followed by a filename.");
        	return NULL;
        }
       	cmd->file = calloc(strlen(args) + 1, sizeof(char));
        strcpy(cmd->file, args);
      }	
	  // check if it is '>>' or not
      else if (!strcmp(args, ">>"))  
      {
      	// if there's no commands before ">>", , it is an invalid input
        if (cmd->argc == 0)
        {
        	printf("#error: \">>\" with no proceeding command.");
        	return NULL;
        }
        cmd->red_op = APP_OUT;
        args = strtok_r(NULL, " ", &subcmd_saveptr);
        // if there's no filename after ">>", , it is an invalid input
        if (args == NULL)
        {
          printf("#errors: \">>\" not followed by a filename.");
        	return NULL;
        }
        cmd->file = calloc(strlen(args) + 1, sizeof(char));
        strcpy(cmd->file, args);
      }
      // otherwise, save it into commands
      else  
      {
        cmd->args[cmd->argc] = calloc(strlen(args) + 1, sizeof(char));
        strcpy(cmd->args[cmd->argc], args);
        cmd->argc++;
      }									

      args = strtok_r(NULL, " ", &subcmd_saveptr);
    }
		commands[cmd_count] = cmd;
    cmd_count++;
    // ls | wc | cat | echo |
    cmd_str = strtok_r(NULL, "|", &cmd_saveptr);
  }

  *n_commands = cmd_count;
  return commands;
    
}

// replace the space at the end of the line to '\0'
// get rid of the space at the end of line
void trim(char* buffer) {
  int i;
  for (i = strlen(buffer) - 1; buffer[i] == ' '; i--) {
  	buffer[i] = '\0';
  }
}

// check if the command has appropriate file redirection symbol

bool has_valid_redirects(struct command** commands, int n_commands) {
  // make sure only the first command has input redirection
  int i;
  for (i = 1; i < n_commands; i++) {
    if (commands[i]->red_op == RED_IN) {
      return false;
    }
  }
  int j;// make sure only the last command has output redirection
  for (j = 0; j < n_commands - 1; j++) {
    if (commands[j]->red_op == RED_OUT || commands[i]->red_op == APP_OUT) {
      return false;
    }
  }
  return true;
}


// get rid of space at the end of line 
// and check if pip shows up at the end of line
bool has_trailing_pipe(char* input) {
  trim(input);
	if (input[strlen(input) - 1] == '|') {
  	return true;
  }
  return false;
}


int check_backgrounded(char* input) {

  /* Allocate and zero memory for our string */
  //char*  ptr;
  char*  background;

  background = (char *)calloc(strlen(input), sizeof(char));

  strcpy(background, input);
  trim(background);
  background = strchr(background, '&');

  if (background == NULL) {
  	return 0;
  }
  else if (background[strlen(background) - 2] != '&') {
  			return -1;
  }
  else {
    return 1;
  }
}

// print results
void print_command(struct command** commands, int cmd_count, bool is_backgrounded) {
  printf("Commands:");
  int i;
  for (i = 0; i < cmd_count; i++) {
  	int j;
    for (j = 0; j < commands[i]->argc; j++) {
      printf(" %s", commands[i]->args[j]);
      }
      if(i != (cmd_count - 1))
      {
    	printf(",");
      }
 
  }
  char* input_file = "None";
  if (commands[0]->red_op == RED_IN) {
    input_file = commands[0]->file;
  }
  printf("\nInput file: %s", input_file);
  char* output_file = "None";
  if (commands[cmd_count - 1]->red_op == RED_OUT || commands[cmd_count - 1]->red_op == APP_OUT) {
    output_file = commands[cmd_count - 1]->file;
  }
  printf("\nOutput file: %s", output_file);
  // char* is_backgrounded_str = is_backgrounded ? "Yes" : "No";
  char* is_backgrounded_str;
  if (is_backgrounded)
     is_backgrounded_str = "Yes";
  else
    is_backgrounded_str = "No";
  printf("\nBackground or not: %s", is_backgrounded_str);
}

// main function to run the simple shell
int main (int argc, char** argv) {
  char* input = NULL;
  size_t n = 0;
  bool is_backgrounded = false;
  int n_commands;
  
  while (1) {
    printf("Shell>");
    getline(&input, &n, stdin);
    // Exit if type in exit
    if(strcmp(input, "exit\n") == 0){
        printf("%s\n", input);    
    	exit(1);
    }
    size_t num_allocated = strlen(input);
    if (num_allocated == -1) {
      printf("Could not allocate buffer for input.\n");
      continue;
    }

    int result = check_backgrounded(input);

    if (result == -1) {
      printf("\n#error 1: make sure '&' only appear in the end of the command line.\n");
      continue;
    }
    else {
    	is_backgrounded = result;
    }

/*
    // if "|" shows up at the end of line, it's an error
    if (has_trailing_pipe(input)) {
      printf("#error: a pip with no succeeding command.\n");
      continue;
    }
*/
    struct command** commands = parse_commands(input, &n_commands); 


/*
    if (has_valid_redirects(commands, n_commands)) {
      printf("\n#error: make sure only the first subcommand have \"<\", and only the last subcommand have \">\".\n");
      continue;
    } 
*/  
    if(commands != NULL)
    {  
    	print_command(commands, n_commands, is_backgrounded);
    	free_commands(commands, n_commands);
    	free(commands);
    	free(input);
    	input = NULL;
    	n = 0;
    	printf("\n");
    }
  }
  return EXIT_SUCCESS;
}

