#include <stdio.h>

int fibonacci(int x){
  if(x == 1){
    return 1;
  }
  else{
    if(x==2){
      return 1;
    }
  else{
    return fibonacci(x - 1) + fibonacci(x - 2);
  }
  }
}


int main(void) {
  int x;

  printf("Insira um numero: ");
  scanf("%d",&x);

  printf("O %d numero de fibonacci é %d", x,fibonacci(x));
  
  return 0;
}
// Programa básico feito em sala de aula para ver os numeros de Fibonacci