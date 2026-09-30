#include <stdio.h>
#include "calculo.h" // O compilador vai achar este arquivo porque vamos indicar a pasta dele

int main() {
    int numero = 5;
    int resultado = somar(numero);
    
    printf("O número %d somado com 1 é: %d\n", numero, resultado);
    
    return 0;
}
