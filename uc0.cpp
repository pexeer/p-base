#include <stdio.h>
#include <ucontext.h>

char func1_stack[16384];
char func2_stack[16384];

ucontext_t uctx, uctx_main;

void func1(void) {
  printf("hell0,world!\n");
}

int main(int argc, char *argv[]) {
  int ret = getcontext(&uctx);
  printf("getcontext ret=%d\n", ret);

  uctx.uc_stack.ss_sp = func1_stack;
  uctx.uc_stack.ss_size = sizeof(func1_stack);
  uctx.uc_link = &uctx_main;
  makecontext(&uctx, func1, 0);

  ret = getcontext(&uctx_main);

  printf("getcontext for uctx_main ret=%d\n", ret);

  ret = setcontext(&uctx);

  printf("setcontext ret=%d\n", ret);

  return 0;
}
