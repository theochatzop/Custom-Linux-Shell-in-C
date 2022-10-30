#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "my_shell.h"

int main(int argc, char ** argv) {
  input in ;
  built_in b;
  redirection r;

  while (1) {
    initialize_struct( & in , & b, & r);
    print_prompt();
    get_input( & in );
    remove_newline( & in );
    if (check_exit( & in )) {
      printf("You exited\n");
      break;
    }

    parse_command( & in );
    check_red( & in , & r);
    find_pos( & in , & r);

    if (count_pipes( & in ) > 0) {
      pipe_handle( & in , & r);
    } else if ((r.input == 1) || (r.output == 1) || (r.append == 1)) {
      exec_red( & in , & r);
    } else if (!check_built_in( & in , & b)) {
      exec_args( & in );
    }
  }

  free_memory( & in , & b, & r);

  return 0;
}

void initialize_struct(input * in , built_in * b, redirection * red) {
  int i, j;

  in -> line = malloc(MAX_INPUT * sizeof(char));
  if ( in -> line == NULL) {
    printf("Out of memory");
    exit(0);
  } in -> tok_array = malloc(MAX_LENGTH * sizeof(char * ));
  if ( in -> tok_array == NULL) {
    printf("Out of memory");
    exit(0);
  }

  for (i = 0; i < MAX_LENGTH; i++) {
    in -> tok_array[i] = malloc(MAX_INPUT * sizeof(char));
  }

  int size = 20 + 400; 
  
  in -> cmd_array = malloc(size * sizeof(char *));

  if (in -> cmd_array == NULL) {
    printf("Out of memory");
    exit(0);
  }

  for (i = 0; i < 20; i++) {
    in -> cmd_array[i] = malloc(20 * sizeof(char *));

    if ( in -> cmd_array[i] == NULL) {
      printf("Out of memory");
      exit(0);
    }

    for (j = 0; j < 20; j++) {
      in -> cmd_array[i][j] = malloc(20 * sizeof(char));

      if ( in -> cmd_array[i][j] == NULL) {
        printf("Out of memory");
        exit(0);
      }
    }
  }
  b -> num = 3;
  b -> built_in_array = malloc((b -> num) * sizeof(char *));
  if (b -> built_in_array == NULL) {
    printf("Out of memory");
    exit(0);
  }
  b -> built_in_array[0] = "cd";
  b -> built_in_array[1] = "help";
  b -> built_in_array[2] = "pwd";

  in -> pos = malloc(80 * sizeof(int));

  red -> input = 0;
  red -> output = 0;
  red -> append = 0;
  red -> filename1 = malloc(80 * sizeof(char));
  red -> filename2 = malloc(80 * sizeof(char));
  red -> filename3 = malloc(80 * sizeof(char));
}

void print_prompt(void) {

  printf("my_shell$ ");

}

void get_input(input * in ) {
  char buf[MAX_INPUT];

  fgets(buf, MAX_INPUT, stdin);
  strcpy( in -> line, buf);
}

int check_exit(input * in) {

  if (!strcmp( in -> line, "exit"))
    return 1;

  return 0;
}

void check_red(input *in , redirection *red) {
  int i;

  while ( in -> tok_array[i] != NULL) {
    if ((!strcmp( in -> tok_array[i], "<")))
      red -> input = 1;
    else if ((!strcmp( in -> tok_array[i], ">")))
      red -> output = 1;
    else if ((!strcmp( in -> tok_array[i], ">>")))
      red -> append = 1;
    i++;
  }
}

void find_pos(input *in , redirection *red) {
  int i = 0;

  while ( in -> tok_array[i] != NULL) {
    if (!strcmp( in -> tok_array[i], "<")) {
      red -> filename1 = in -> tok_array[i + 1];
    }
    if (!strcmp( in -> tok_array[i], ">")) {
      red -> filename2 = in -> tok_array[i + 1];
    }
    if (!strcmp( in -> tok_array[i], ">>")) {
      red -> filename3 = in -> tok_array[i + 1];
    }

    i++;
  }
}

void remove_newline(input *in) {
  int i = 0;

  while ( in -> line[i] != '\n') {
    i++;
  }
  in -> line[i] = '\0';
}

void parse_command(input *in) {
  int i = 0;
  char *arg;

  arg = my_strtok( in -> line, " \t\r\n\a");
  while (arg != NULL) {
    in -> tok_array[i] = arg;
    i++;
    arg = my_strtok(NULL, " \t\r\n\a");
  }

  in -> tok_array[i] = NULL;
}

void exec_args(input *in) {
  int status;

  pid_t pid = fork();

  if (pid == -1) {
    printf("\nFailed forking child..");
    perror("fork failed");
    return;
  } else if (pid == 0) {
    if (execvp( in -> tok_array[0], in -> tok_array) < 0) {
      printf("\nCould not execute command");
      perror("exec failed");
    }
    exit(0);
  } else {
    waitpid(pid, & status, 0);
  }
}

void exec_red(input *in , redirection *red) {
  int status;
  int i, fd0, fd1, fd2;
  int j = 0;
  int k = 0, m = 0;

  while ( in -> tok_array[k] != NULL) {
    if (!(strcmp( in -> tok_array[k], ">")) || !(strcmp( in -> tok_array[k], "<")) || !(strcmp( in -> tok_array[k], ">>"))) {
      in -> tok_array[k] = NULL;
    }
    k++;
  }

  pid_t pid = fork();

  if (pid == -1) {
    printf("Failed forking child\n");
    perror("Fork failed");
    return;
  } else if (pid == 0) {

    if (red -> input) {

      if ((fd0 = open(red -> filename1, O_RDONLY)) < 0) {
        printf("error opening file\n");
        perror("open failed");
      }
      dup2(fd0, STDIN_FILENO);
      close(fd0);
    }

    if (red -> output) {

      if ((fd1 = open(red -> filename2, O_CREAT | O_TRUNC | O_WRONLY, 0664)) < 0) {
        printf("error creating file\n");
        perror("open failed");
      }
      dup2(fd1, STDOUT_FILENO);
      close(fd1);
    }

    if (red -> append) {
      if ((fd2 = open(red -> filename3, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
        printf("error opening file\n");
        perror("append failed");
      }

      dup2(fd2, STDOUT_FILENO);
      close(fd2);

    }

    if (execvp( in -> tok_array[0], in -> tok_array) < 0) {
      printf("Could not execute command\n");
      perror("exec failed");
    }
    exit(0);
  } else {

    waitpid(pid, & status, 0);

  }

}

int my_cd(input *in) {
  if ( in -> tok_array[1] == NULL) {
    printf(" Please enter a path to cd\n");
  } else {
    if (chdir( in -> tok_array[1]) > 0) {
      perror("dash");
      return -1;
    }
  }
  return 1;
}

int my_pwd(void) {
  char cwd[1024];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    fprintf(stdout, "%s\n", cwd);
  } else {
    printf("getcwd() error!\n");
  }

  return 1;
}

int my_help(built_in *b) {
  int i = 0;

  printf("\n");

  printf("Welcome to my shell!\n");
  printf("The following built-in commands are supported as well as all other UNIX commands\n");
  printf("\n");

  for (i = 0; i < b -> num; i++) {
    printf("%s\n", b -> built_in_array[i]);
  }

  printf("\n");

  return 1;
}

void execute_buit_in(int pos, input *in , built_in *b) {

  switch (pos) {
  case 0:
    my_cd( in );
    break;
  case 1:
    my_help(b);
    break;
  case 2:
    my_pwd();
    break;
  default:
    break;
  }

}

int check_built_in(input *in , built_in *b) {
  int i = 0;
  if ( in -> tok_array[0] == NULL) {
    return 0;
  }

  for (i = 0; i < b -> num; i++) {
    if (!strcmp( in -> tok_array[0], b -> built_in_array[i])) {
      execute_buit_in(i, in , b);
      return 1;
    }
  }

  return 0;
}

int count_pipes(input *in) {
  int i = 0, cnt = 0, j = 0;
  int first = 0;
  int last;

  while ( in -> tok_array[i] != NULL) {

    if (!strcmp( in -> tok_array[i], "|")) {
      last = (i - 1); in -> pos[cnt] = last - first + 1;
      cnt++;
      first = i + 1;
    }
    i++;
  } in -> no_cmds = cnt + 1;
  last = (i - 1); in -> pos[cnt] = last - first + 1;

  return cnt;
}

void loop_pipe(input *in, redirection *red) {
  int fdpipe[2];
  pid_t pid;
  int status,i;
  int fd0, fd1, fd2;

  int tmpin = dup(0);
  int tmpout = dup(1);

  if (((red -> input) == 1)) {

    if ((fd0 = open(red -> filename1, O_RDONLY)) < 0) {
      printf("error opening input file\n");
      perror("error opening file\n");
    }
  } else {
    fd0 = dup(tmpin);
  }

  for (i = 0; i < (( in -> no_cmds)); i++) {
    dup2(fd0, 0);
    close(fd0);

    if (i == (( in -> no_cmds) - 1)) {

      if (((red -> output) == 1)) {

        if ((fd1 = open(red -> filename2, O_CREAT | O_TRUNC | O_WRONLY, 0664)) < 0) {
          printf("error creating file\n");
          perror("creat failed");
        }
      } else if (((red -> append) == 1)) {
        if ((fd1 = open(red -> filename3, O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
          printf("error opening file\n");
          perror("append failed");
        }
      } else {
        fd1 = dup(tmpout);
      }
    } else {
      pipe(fdpipe);
      fd1 = fdpipe[1];
      fd0 = fdpipe[0];
    }

    dup2(fd1, 1);
    close(fd1);

    pid = fork();
    if (pid == -1) {
      printf("failed forking\n");
      perror("Fork failed");
      exit(-1);
    }
    if (pid == 0) {
      if (execvp( in -> cmd_array[i][0], in -> cmd_array[i]) < 0) {
        printf("error executing command\n");
        perror("exec failed");
        exit(-1);
      }
    }
  }
  dup2(tmpin, 0);
  dup2(tmpout, 1);
  close(tmpin);
  close(tmpout);

  waitpid(pid, & status, 0);
}

void pipe_handle(input *in , redirection *red) {
  int x = in -> no_cmds;
  int i, j, pos;
  int m = 0, l = 0, k = 0;
  char filename[20];

  k = 0;

  for (j = 0; j < ( in -> no_cmds); j++) {

    for (i = 0; i < in -> pos[j]; i++) {
      in -> cmd_array[j][i] = in -> tok_array[k];
      k++;
    }
    k++; in -> cmd_array[j][i] = NULL;
  }

  in -> cmd_array[ in -> no_cmds] = NULL;

  l = 0;
  m = 0;

  for (j = 0; j < in -> no_cmds; j++) {
    for (i = 0; i < in -> pos[j]; i++) {
      if ((!strcmp( in -> cmd_array[j][i], ">")) || (!strcmp( in -> cmd_array[j][i], "<")) || (!strcmp( in -> cmd_array[j][i], ">>"))) {
        in -> cmd_array[j][i] = NULL;
      }
    }
  }

  loop_pipe( in , red);
}


int is_delim(char c, char *delim) {
  while ( * delim != '\0') {
    if (c == * delim)
      return 1;
    delim++;
  }
  return 0;
}

char *my_strtok(char *s, char *delim) {
  static char *p;
  if (!s) {
    s = p;
  }
  if (!s) {
    return NULL;
  }

  while (1) {
    if (is_delim( *s, delim)) {
      s++;
      continue;
    }
    if ( * s == '\0') {
      return NULL;
    }

    break;
  }

  char *ret = s;
  while (1) {
    if ( *s == '\0') {
      p = s;
      return ret;
    }
    if (is_delim( *s, delim)) {
      *s = '\0';
      p = s + 1;
      return ret;
    }
    s++;
  }
}


void free_memory(input *in , built_in *b, redirection *red) {
  int i, j;

  for (i = 0; i < MAX_LENGTH; i++) {
    free( in -> tok_array[i]);
  }

  for (i = 0; i < 20; i++) {
    for (j = 0; j < 20; j++) {
      free( in -> cmd_array[i][j]);
    }
    free( in -> cmd_array[i]);
  }
  free( in -> cmd_array);
  free( in -> tok_array);
  free( in -> line);
  free( in -> pos);
  free(b -> built_in_array);
  free(red -> filename1);
  free(red -> filename2);
  free(red -> filename3);
}
