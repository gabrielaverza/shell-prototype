#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TAM_MAX_ARG 1000 // tamanho máximo da string que armazena a entrada do usuário
#define MAX_CMDS 100 // número máximo de comandos em que a entrada pode ser dividida
#define TRUE 1

/* CÓDIGOS DE RETORNO */
#define SAIR 1
#define SUCESSO 0
#define ERRO_ENTRADA -1
#define ERRO_PROCESSO -2

// Função para remover as aspas de um argumento
void remover_aspas(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] != '"') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

// Separa em comandos individuais com base no operador enviado por parametro
int divide_comandos(char *input, const char *operador, char **aux_comandos) {
    // printf("Dividindo comandos com operador: %s\n", operador);
    char *token = strtok(input, operador);
    int qtde_comandos = 0;
    while (token != NULL && qtde_comandos < TAM_MAX_ARG - 1) {
        // printf("Token: %s\n", token);
        aux_comandos[qtde_comandos++] = token;
        token = strtok(NULL, operador);
        // printf("%d", qtde_comandos);
    }
    aux_comandos[qtde_comandos] = NULL; // Indica fim da lista 
    return qtde_comandos;
}

void executa_comandos (int qtde, char **aux_comandos) {
    int fd[2];
    int fd_in = 0;

    for (int j = 0; j < qtde; j++) {
        pipe(fd);
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Erro ao criar processo\n");
            exit(ERRO_PROCESSO);
        } else if (pid == 0) {
            // processo filho executa o comando
            dup2(fd_in, 0);
            if (j != qtde - 1) { // não redireciona a saída se este for o último comando
                dup2(fd[1], 1);
            }
            close(fd[0]);
            // dividir o comando em argumentos

            char *argumentos[TAM_MAX_ARG]; 
            int qtde_argumentos = divide_comandos(aux_comandos[j], " ", argumentos);

            // // printf("Executando comando %d:\n", j);
            // for (int l = 0; l < qtde_argumentos; l++) {
            //     printf("Argumento %d: %s\n", l, argumentos[l]);
            // }

            // remover as aspas do último argumento (se houver)
            if (qtde_argumentos > 0) {
                remover_aspas(argumentos[qtde_argumentos - 1]);
            }

            // executar o comando
            if (execvp(argumentos[0], argumentos) == -1) {
                fprintf(stderr, "Erro ao executar o comando\n");
                exit(ERRO_PROCESSO);
            }
        } else {
            // processo pai
            wait(NULL);
            close(fd[1]);
            fd_in = fd[0];
        }
    }
}

int main() {
    // Entrada do usuário
    char entrada[TAM_MAX_ARG];
    
    while (TRUE) {
        fprintf(stderr, "Digite um comando, ou 'sair' para encerrar:> ");
        fgets(entrada, sizeof(entrada), stdin);

        // Remove o caractere de nova linha do final da entrada, para garantir que a string esteja terminada corretamente apos a leitura
        entrada[strcspn(entrada, "\n")] = 0;

        if (strlen(entrada) == 0 || entrada[0] == '\0') {
            fprintf(stderr, "Entrada vazia ou com caracteres inválidos.\n");
            exit(ERRO_ENTRADA);
        } else if (strcmp(entrada, "sair") == 0) {
            printf("Até logo...");
            exit(SAIR);
        } 

        // Separa a entrada em comandos
        char *comandos[TAM_MAX_ARG]; 
        int qtde_comandos = divide_comandos(entrada, "|", comandos);

        // printf("Comandos separados:\n");
        // for (int j = 0; j < qtde_comandos; j++) {
        //     printf("Comando %d: %s\n", j, comandos[j]);
        // }

        executa_comandos(qtde_comandos, comandos);
    }
    
    return SUCESSO;
}
