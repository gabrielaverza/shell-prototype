#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ENTRADA 1000
#define MAX_DIV 100
#define TRUE 1

#define SAIR 1
#define SUCESSO 0
#define ERRO_ENTRADA -1
#define ERRO_PROCESSO -2
#define ERRO_COMANDO -3
#define ERRO_PIPE -4

void remover_aspas(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] != '"') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

int divide_comandos(char *input, const char *operador, char **aux_comandos) {
    int qtde_comandos = 0;
    char *token;
    char *rest = input;
    int in_quotes = 0;

    token = strtok(rest, operador);
    while (token != NULL && qtde_comandos < MAX_DIV - 1) {
        if (in_quotes) {
            strcat(aux_comandos[qtde_comandos - 1], " ");
            strcat(aux_comandos[qtde_comandos - 1], token);
            if (token[strlen(token) - 1] == '"') {
                in_quotes = 0;
            }
        } else {
            aux_comandos[qtde_comandos++] = token;
            if (token[0] == '"') {
                in_quotes = 1;
                if (token[strlen(token) - 1] == '"') {
                    in_quotes = 0;
                }
            }
        }
        token = strtok(NULL, operador);
    }

    aux_comandos[qtde_comandos] = NULL;
    
    // printf("Comandos divididos:\n");
    // for (int i = 0; i < qtde_comandos; i++) {
    //     printf("Comando %d: %s\n", i, aux_comandos[i]);
    // }

    return qtde_comandos;
}

int executa_comando(char *comando) {
    // printf("Executando comando: %s\n", comando);
    
    char *argumentos[MAX_DIV];
    int qtde_argumentos = divide_comandos(comando, " ", argumentos);
    
    for (int i = 0; i < qtde_argumentos; i++) {
        remover_aspas(argumentos[i]);
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(argumentos[0], argumentos) == -1) {
            fprintf(stderr, "Erro ao executar o comando\n");
            exit(ERRO_COMANDO);
        }
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        // printf("Status do comando: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
        // exit(status);
    } else {
        fprintf(stderr, "Erro ao criar processo\n");
        exit(ERRO_PROCESSO);
    }
    return SUCESSO;
}

void trata_operadores(char *comando) {
    int resultado_anterior = SUCESSO;

    char *operador = strstr(comando, "&&");
    if (operador != NULL) {
        *operador = '\0'; // Substitui o operador pelo terminador de string
        char *comando1 = comando;
        char *comando2 = operador + 2; // Pula o operador "&&"
        
        resultado_anterior = executa_comando(comando1);
        if (resultado_anterior == SUCESSO) {
            executa_comando(comando2);
        }
    } else {
        operador = strstr(comando, "||");
        if (operador != NULL) {
            *operador = '\0'; // Substitui o operador pelo terminador de string
            char *comando1 = comando;
            char *comando2 = operador + 2; // Pula o operador "||"
            
            resultado_anterior = executa_comando(comando1);
            if (resultado_anterior != SUCESSO) {
                executa_comando(comando2);
            }
        } else {
            // Se não encontrar operadores, executa o comando normalmente
            executa_comando(comando);
        }
    }
}

void executa_comandos_pipe(int qtde, char **comandos) {
    int fd[2];
    int fd_in = 0;

    for (int j = 0; j < qtde; j++) {
        if (pipe(fd) < 0) {
            fprintf(stderr, "Erro ao criar pipe\n");
            exit(ERRO_PIPE);
        }

        pid_t pid = fork();
        if (pid == 0){
            dup2(fd_in, 0);
            if (j < qtde - 1) {
                dup2(fd[1], 1);
            }
            close(fd[0]);

            // printf("Executando comando via pipe: %s\n", comandos[j]);

            trata_operadores(comandos[j]);
            exit(SUCESSO);
        } else if (pid > 0) {
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
    char entrada[MAX_ENTRADA];
    
    while (TRUE) {
        fprintf(stderr, "Digite um comando, ou 'sair' para encerrar:> ");
        fgets(entrada, sizeof(entrada), stdin);
        entrada[strcspn(entrada, "\n")] = 0;

        if (strlen(entrada) == 0 || entrada[0] == '\0') {
            fprintf(stderr, "Entrada vazia ou com caracteres inválidos.\n");
            exit(ERRO_ENTRADA);
        } else if (strcmp(entrada, "sair") == 0) {
            printf("Até logo...\n");
            exit(SAIR);
        }

        char *comandos[MAX_DIV];
        int qtde_comandos = divide_comandos(entrada, "|", comandos);

        // printf("Comandos separados:\n");
        // for (int j = 0; j < qtde_comandos; j++) {
        //     printf("Comando %d: %s\n", j, comandos[j]);
        // }

        // printf("Quantidade de comandos: %d\n", qtde_comandos);

        executa_comandos_pipe(qtde_comandos, comandos);
    }
    return SUCESSO;
}
