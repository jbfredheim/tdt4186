

#include "kernel/types.h"
#include "user/user.h"



int main(int argc, char *argv[]) {
  int addr = 0;
  if (argc < 2 || argc > 3) {
    fprintf(1, "Usage: vatopa virtual_address [pid]\n");
  } else if (argc < 3) {
    //Only one argument provided, using vatopa's own PID
    addr = va2pa(atoi(argv[1]), getpid());
  } else {
    addr = va2pa(atoi(argv[1]), atoi(argv[2]));
  }
  fprintf(1, "0x%x\n", addr);

  return 0;
}
