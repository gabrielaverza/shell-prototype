#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ENTRADA 1000 // tamanho máximo da string que armazena a entrada do usuário
#define MAX_DIV 100 // número máximo de comandos/argumentos em que a entrada pode ser dividida
#define TRUE 1

/* CÓDIGOS DE RETORNO */
#define SAIR 1
#define SUCESSO 0
#define ERRO_ENTRADA -1
#define ERRO_PROCESSO -2
#define ERRO_COMANDO -3
#define ERRO_PIPE -4

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
    char *token = strtok(input, operador);
    int qtde_comandos = 0;
    while (token != NULL && qtde_comandos < MAX_DIV - 1) {
        aux_comandos[qtde_comandos++] = token;
        token = strtok(NULL, operador);
    }
    aux_comandos[qtde_comandos] = NULL; // Indica fim da lista 
    return qtde_comandos;
}

void executa_comandos (int qtde, char **aux_comandos) {
    int fd[2]; // Armazena descritores do pipe
    int fd_in = 0; // Armazena descritor de entrada do pipe

    for (int j = 0; j < qtde; j++) {
        if (pipe(fd) < 0) {
            fprintf(stderr, "Erro ao criar pipe\n");
            exit(ERRO_PIPE);
        }

        pid_t pid = fork();
        if (pid == 0){
            // processo filho executa o comando
            dup2(fd_in, 0); // 0 = stdin
            if (j != qtde - 1) { // não redireciona a saída se este for o último comando
                dup2(fd[1], 1); // 1 = sdtout
            }
            close(fd[0]);
            
            // dividir o comando em argumentos
            char *argumentos[MAX_DIV]; 
            int qtde_argumentos = divide_comandos(aux_comandos[j], " ", argumentos);

            // remover as aspas de cada argumento (se houver)
            for (int i = 0; i < qtde_argumentos; i++) {
                remover_aspas(argumentos[i]);
            }

            // executar o comando
            if (execvp(argumentos[0], argumentos) == -1) {
                fprintf(stderr, "Erro ao executar o comando\n");
                exit(ERRO_COMANDO);
            }
        } else if (pid > 0) {
            // processo pai
            wait(NULL);
            close(fd[1]);
            fd_in = fd[0];
        } else {
            fprintf(stderr, "Erro ao criar processo\n");
            exit(ERRO_PROCESSO);
        }
    }
}

int main() {
    // Entrada do usuário
    char entrada[MAX_ENTRADA];
    
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
        char *comandos[MAX_DIV]; 
        int qtde_comandos = divide_comandos(entrada, "|", comandos);

        executa_comandos(qtde_comandos, comandos);
    }
    
    return SUCESSO;
}
