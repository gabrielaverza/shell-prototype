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
    // ascii 34 = " e 39 = '
    int i, j;
    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if ((int) str[i] != 34 && (int) str[i] != 39) { // verifica por aspas duplas e simples
            str[j++] = str[i];
        } 
    }
    str[j] = '\0';
}

int divide_comandos_pipe(const char *comandos, char **aux_comandos) {
    int qtde_comandos = 0;

    aux_comandos[qtde_comandos] = (char *)malloc(MAX_ENTRADA); // aloca espaco para o primeiro token
    aux_comandos[qtde_comandos][0] = '\0'; // inicializa com string vazia

    for (int i = 0; comandos[i] != '\0'; i++) {
        if (comandos[i] == '|') {
            if (comandos[i + 1] == '|') {
                strcat(aux_comandos[qtde_comandos], "||");
                i++; // pula o proximo '|' pois ja foi tratado
            } else {
                qtde_comandos++;
                aux_comandos[qtde_comandos] = (char *)malloc(MAX_ENTRADA); // aloca espaco para o proximo token
                aux_comandos[qtde_comandos][0] = '\0'; // inicializa com string vazia
            }
        } else {
            char temp[2] = {comandos[i], '\0'};
            strcat(aux_comandos[qtde_comandos], temp);
        }
    }

    aux_comandos[qtde_comandos + 1] = NULL; // indica fim da lista
    return qtde_comandos + 1;
}

int divide_comandos(char *comandos, char **aux_comandos) {
    char *token = strtok(comandos, " ");
    int qtde_comandos = 0;

    while (token != NULL && qtde_comandos < MAX_DIV - 1) {
        // verifica se o token começa com aspas duplas
        if ((int) token[0] == 34 || (int) token[0] == 39) {
            char temp[MAX_ENTRADA] = "";
            strcat(temp, token);
            
            // continua concatenando ate encontrar as aspas finais
            while ((int) token[strlen(token) - 1] != 34 && (int) token[strlen(token) - 1] != 39) {
                token = strtok(NULL, " ");
                if (token == NULL) {
                    break;
                }
                strcat(temp, " "); // mantém o delimitador " " quando houver aspas
                strcat(temp, token);
            }
            
            aux_comandos[qtde_comandos] = (char *)malloc(strlen(temp) + 1);
            strcpy(aux_comandos[qtde_comandos], temp);
        } else {
            aux_comandos[qtde_comandos] = (char *)malloc(strlen(token) + 1);
            strcpy(aux_comandos[qtde_comandos], token);
        }
        
        qtde_comandos++;
        token = strtok(NULL, " "); // move para o proximo token
    }

    aux_comandos[qtde_comandos] = NULL; // indica fim da lista
    return qtde_comandos;
}

int executa_comando(char *comando, int background) {
    char *argumentos[MAX_DIV];
    int qtde_argumentos = divide_comandos(comando, argumentos);
    
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

int trata_operadores(char *comando) {
    char *operador_ou = strstr(comando, "||");
    char *operador_e = strstr(comando, "&&");
    int resultado_comando;

    if (operador_e && operador_ou) { // ambos os operadores encontrados
        if (operador_e < operador_ou) { // && aparece antes de ||
            *operador_e = '\0';
            
            char *comando1 = comando;
            char *comando2 = operador_e + 2;

            resultado_comando = executa_comando(comando1, 0);
            if (resultado_comando == SUCESSO) {
                trata_operadores(comando2);
            } else {
                *operador_ou = '\0';
                comando2 = operador_ou + 2;
                executa_comando(comando2, 0);
            }
        } else { // || aparece antes de &&
            *operador_ou = '\0';
            
            char *comando1 = comando;
            char *comando2 = operador_ou + 2;

            resultado_comando = executa_comando(comando1, 0);
            if (resultado_comando != SUCESSO) {
                trata_operadores(comando2);
            }
        }
    } else if (operador_e) { // apenas operador &&
        *operador_e = '\0';
        char *comando1 = comando;
        char *comando2 = operador_e + 2;

        resultado_comando = executa_comando(comando1, 0);
        if (resultado_comando == SUCESSO) {
            trata_operadores(comando2);
        }
    } else if (operador_ou) { // apenas operador ||
        *operador_ou = '\0';
        char *comando1 = comando;
        char *comando2 = operador_ou + 2;

        resultado_comando = executa_comando(comando1, 0);
        if (resultado_comando != SUCESSO) {
            trata_operadores(comando2);
        }
    } else { // executa o comando
        int background = 0;
        if (comando[strlen(comando) - 1] == '&') { // verifica se deve ser executado em bg
            background = 1;
            comando[strlen(comando) - 1] = '\0';
        }
        return executa_comando(comando, background);
    }
    return SUCESSO;
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
        int qtde_comandos = divide_comandos_pipe(entrada, comandos);

        executa_comandos_pipe(qtde_comandos, comandos);
    }
    return SUCESSO;
}
