#include <stdio.h>
#include <string.h>


struct notas {
    float matematica;
    float portuguesa;
    float historia;
};

struct aluno {
    int RA;
    char nome[50];
    struct notas nota;
};

float media(float nota1, float nota2, float nota3) {        
    return (nota1 + nota2 + nota3) / 3;
}


int main() { // função principal    
    /*struct aluno aluno[10];
    aluno[0].RA = 100100;
    strcpy(aluno[0].nome, "João");
    aluno[0].nota.matematica = 10.0;
    aluno[0].nota.portuguesa = 9.0;
    aluno[0].nota.historia = 8.0;

    printf("RA: %d\n", aluno[0].RA);
    printf("Nome: %s\n", aluno[0].nome);
    printf("Nota Matematica: %.1f\n", aluno[0].nota.matematica);
    printf("Nota Portuguesa: %.1f\n", aluno[0].nota.portuguesa);
    printf("Nota Historia: %.1f\n", aluno[0].nota.historia);
    printf("Média: %.1f\n", media(aluno[0].nota.matematica, aluno[0].nota.portuguesa, aluno[0].nota.historia));*/
  
    return 0; // SE DER CERTO, RETORNA 0    
} // fim da função principal
