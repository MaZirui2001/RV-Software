#include <devtest.h>

void hello() {
  for (int i = 0; i < 10; i ++) {
    printf("%d: Hello, Zircon 2024!\n", i);
  }
}

int main(int argc, char **args){
  ioe_init();
  hello();
  return 0;
}