/* ************************************************* */
/* Sistemas Operacionais - Implementacao de um shell */
/*                                                   */
/* Gabriel de Mello Cambuy Ferreira                  */
/* Gabriela Carregari Verza                          */
/* Melissa Frigi Mendes                              */
/* Raissa Barbosa dos Santos                         */
/*                                                   */
/*                   UNIFESP - SJC                   */
/* ************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// tamanho maximo da entrada
#define MAX_ENTRADA 1000 
// em quantas partes pode ser dividida
#define MAX_DIV 100
#define TRUE 1

// Codigos de Retorno
#define SAIR 1
#define SUCESSO 0
#define ERRO_ENTRADA -1
#define ERRO_PROCESSO -2
#define ERRO_COMANDO -3
#define ERRO_PIPE -4

// remover aspas de argumento para que seja identificado corretamente
void remover_aspas(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] != '"') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

// divide string com base em operador enviado por parametro
int divide_comandos(char *comandos, const char *operador, char **aux_comandos) {
    int qtde_comandos = 0;
    char *token;
    int entre_aspas = 0;

    token = strtok(comandos, operador);
    while (token != NULL && qtde_comandos < MAX_DIV - 1) {
        if (entre_aspas) { // nao separa por espaco se estiver entre aspas
            strcat(aux_comandos[qtde_comandos - 1], " ");
            strcat(aux_comandos[qtde_comandos - 1], token);
            if (token[strlen(token) - 1] == '"') {
                entre_aspas = 0;
            }
        } else {
            aux_comandos[qtde_comandos++] = token;
            if (token[0] == '"') { // verifica se o token esta entre aspas
                entre_aspas = 1;
                if (token[strlen(token) - 1] == '"') {
                    entre_aspas = 0;
                }
            }
        }
        token = strtok(NULL, operador);
    }

    aux_comandos[qtde_comandos] = NULL;
    
    return qtde_comandos;
}

int executa_comando(char *comando, int background) {
    char *argumentos[MAX_DIV];
    int qtde_argumentos = divide_comandos(comando, " ", argumentos);
    
    for (int i = 0; i < qtde_argumentos; i++) {
        remover_aspas(argumentos[i]);
    }
    
    pid_t pid = fork();
    if (pid == 0) { // processo filho executa comando
        if (background) {
            setsid(); // cria uma nova sessao para executar o comando em segundo plano
        }
        if (execvp(argumentos[0], argumentos) == -1) {
            fprintf(stderr, "Erro ao executar o comando\n");
            exit(ERRO_COMANDO);
        }
    } else if (pid > 0) { // processo pai espera termino do filho
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status); // extrai status de saida do filho
        }
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
        *operador = '\0';
        char *comando1 = comando; // comando antes do operador
        char *comando2 = operador + 2; // comando depois do operador
        
        resultado_anterior = executa_comando(comando1, 0);
        if (resultado_anterior == SUCESSO) { // executa o prox somente se o anterior finalizar ok
            executa_comando(comando2, 0);
        }
    } else {
        operador = strstr(comando, "||");
        if (operador != NULL) {
            *operador = '\0';
            char *comando1 = comando; // comando antes do operador
            char *comando2 = operador + 2; // comando depois do operador
            
            resultado_anterior = executa_comando(comando1, 0);
            if (resultado_anterior != SUCESSO) { // executa o prox somente se o anterior falhar
                executa_comando(comando2, 0);
            }
        } else { // verifica se o comando deve ser executado em background
            int background = 0;
            if (comando[strlen(comando) - 1] == '&') {
                background = 1;
                comando[strlen(comando) - 1] = '\0';
            }
            executa_comando(comando, background);
        }
    }
}

void executa_comandos_pipe(int qtde, char **comandos) {
    int fd[2]; // armazena arquivos descritores do pipe
    int fd_in = 0; // armazena arquivo descritor de entrada do pipe

    for (int j = 0; j < qtde; j++) {
        if (pipe(fd) < 0) {
            fprintf(stderr, "Erro ao criar pipe\n");
            exit(ERRO_PIPE);
        }

        pid_t pid = fork();
        if (pid == 0){ // processo filho executa comando
            dup2(fd_in, 0); // 0 = stdin
            if (j < qtde - 1) { // se for o ultimo comando, nao redireciona a saida
                dup2(fd[1], 1); // 1 = stdout
            }
            close(fd[0]);

            trata_operadores(comandos[j]);
            exit(SUCESSO);
        } else if (pid > 0) { // processo pai espera termino do filho
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

        // remove carct nova linha, garante termino correto da string, importante para strtok
        entrada[strcspn(entrada, "\n")] = 0;

        if (strlen(entrada) == 0 || entrada[0] == '\0') {
            fprintf(stderr, "Entrada vazia ou com caracteres inválidos.\n");
            exit(ERRO_ENTRADA);
        } else if (strcmp(entrada, "sair") == 0) {
            printf("Até logo...\n");
            exit(SAIR);
        }

        // separa entrada em comandos com base no pipe
        char *comandos[MAX_DIV];
        int qtde_comandos = divide_comandos(entrada, "|", comandos);

        executa_comandos_pipe(qtde_comandos, comandos);
    }
    return SUCESSO;
}
