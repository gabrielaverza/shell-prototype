# Sistemas Operacionais
Repositório de um projeto desenvolvido durante a disciplina Sistemas Operacionais, na UNIFESP - SJC.

## Implementação de um protótipo de shell

### Para executar em terminal Linux
gcc -o shell shell.c <br/>
./shell

### Funcionalidades
**1. Executar comandos unitários com múltiplos parâmetros** <br/>
Exemplo: ls -la <br/><br/>
**2. Executar comandos encadeados, utilizando operador pipe "|" para combinar saída e entrada entre n comandos** <br/>
Exemplo 1: ls -la | grep "teste" <br/>
Exemplo 2: cat /proc/cpuinfo | grep "model name" | wc -l <br/><br/>
**3. Executar comandos condicionados com operadores OR (||) e AND (&&)** <br/>
Exemplo 1: cat 12345 || echo "aquivo inexistente" <br/>
Exemplo 2: ping -c1 www.unifesp.br.br && echo "SERVIDOR DISPONIVEL" || echo "SERVIDOR INDISPONIVEL"<br/><br/>
**4. Executar comandos em background, liberando o shell para receber novos comandos do usuário** <br/>
Exemplo 1: ping -c5 www.unifesp.br &



