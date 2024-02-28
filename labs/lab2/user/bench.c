#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc <= 2) {
    fprintf(2, "Usage: bench num program args\n");
    exit(1);
  }
  char **args = &argv[1];
  fprintf(1, "Running benchmark of ' ");
  for (int i = 2; i < argc; i++) {
    fprintf(1, "%s ", argv[i]);
  }
  fprintf(1,"' %d times\n", atoi(argv[1]));
  for (int i = 0; i < atoi(argv[1]); i++) {
    int pid = fork();
    if (pid < 0) {
      fprintf(2, "Fork problem\n");
      exit(1);
    } else if (pid == 0) {
      exec("time", args);
      fprintf(2, "exec problem\n");
      exit(1);
    } else {
      wait(0);
    }
  }
  exit(0);
}
