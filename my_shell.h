#ifndef MY_SHELL_H_
#define MY_SHELL_H_

#define MAX_INPUT 100
#define MAX_LENGTH 1000

typedef struct input
{
  char *line;
  char **tok_array;
  int no_cmds;
  int *pos;
  char ***cmd_array;
} input;

typedef struct built_in
{
  char **built_in_array;
  int num;
} built_in;

typedef struct redirection
{
  int input, output, append;
  char *filename1, *filename2, *filename3;
} redirection;

void initialize_struct(input *in, built_in *b, redirection *red);
void print_prompt(void);
void get_input(input *in);
int check_exit(input *in);
void check_red(input *in, redirection *red);
void find_pos(input *in, redirection *red);
void remove_newline(input *in);
void parse_command(input *in);
void exec_args(input *in);
void exec_red(input *in, redirection *red);
int my_cd(input *in);
int my_pwd(void);
int my_help(built_in *b);
void execute_buit_in(int pos, input *in, built_in *b);
int check_built_in(input *in, built_in *b);
int count_pipes(input *in);
void loop_pipe(input *in, redirection *red);
void pipe_handle(input *in, redirection *red);
int is_delim(char c, char *delim);
char *my_strtok(char *s, char *delim);
void free_memory(input *in, built_in *b, redirection *red);

#endif //MY_SHELL_H_