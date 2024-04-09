#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define STR_LEN 1000 // tam max da string que armazena a entrada do usuario
#define MAX_CMDS 100 // num max de comandos em que a entrada pode ser dividida
#define TRUE 1

/* CODIGOS DE RETORNO */
#define SAIR 1
#define SUCESSO 0
#define ERRO_PROCESSO -1

int main() {
    // string que armazena entrada do usuario
    char entrada[STR_LEN];
    // vetor de ponteiros que armazena os comandos fornecidos na entrada
    char *comandos[MAX_CMDS];
    
    while(TRUE) {
        fprintf(stderr, "Digite um comando, ou 'sair' para encerrar:> ");
        fgets(entrada, sizeof(entrada), stdin);

        // remove o caractere de nova linha do final da entrada, para garantir que a string esteja terminada corretamente apos a leitura
        entrada[strcspn(entrada, "\n")] = 0;

        if (strlen(entrada) == 0 || entrada[0] == '\0') {
            perror("Entrada vazia ou com caracteres invalidos.");
            exit(SAIR);
        } else if (strcmp(entrada, "sair") == 0) {
            printf("Ate logo...");
            exit(SAIR);
        } 

        // separa a entrada em tokens, tendo como delimitador o espaço, para obter os comandos
        char *token = strtok(entrada, " ");
        int i = 0;
        while (token != NULL && i < MAX_CMDS - 1) {
            comandos[i++] = token;
            token = strtok(NULL, " ");
        }
        comandos[i] = NULL;
        
        // executar o comando
        pid_t pid = fork();
        if (pid == 0) {
            // processo filho executa comando
            if (execvp(comandos[0], comandos) == -1) {
                perror("Erro ao executar o comando");
                exit(ERRO_PROCESSO);
            }
            exit(SUCESSO);
        } else if (pid > 0) {
            // processo pai aguarda termino do filho
            int status;
            waitpid(pid, &status, 0);
            exit(status);
        } else {
            // erro ao criar processo
            perror("Erro ao criar processo");
            exit(ERRO_PROCESSO);
        }
    }

    return SUCESSO;
}
