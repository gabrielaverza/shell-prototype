Projeto desenvolvido durante a disciplina Sistemas Operacionais, na UNIFESP - SJC.

## Implementação de um protótipo de shell

### Para executar em terminal Linux
```
gcc -o shell shell.c
./shell
```

### Funcionalidades
**1. Executar comandos unitários com múltiplos parâmetros** <br/>
Exemplo:
```
ls -la
```
<br/><br/>
**2. Executar comandos encadeados, utilizando operador pipe "|" para combinar saída e entrada entre n comandos** <br/>
Exemplo 1: 
```
ls -la | grep "teste"
```
<br/>
Exemplo 2:
```
cat /proc/cpuinfo | grep "model name" | wc -l
```
<br/><br/>
**3. Executar comandos condicionados com operadores OR (||) e AND (&&)** <br/>
Exemplo 1:
```
cat 12345 || echo "aquivo inexistente"
```
<br/>
Exemplo 2: 
```
ping -c1 www.unifesp.br.br && echo "SERVIDOR DISPONIVEL" || echo "SERVIDOR INDISPONIVEL"
```
<br/><br/>
**4. Executar comandos em background, liberando o shell para receber novos comandos do usuário** <br/>
Exemplo 1:
```
ping -c5 www.unifesp.br &
```

---

Project developed during the Operating Systems course at UNIFESP - SJC. 

## Shell Prototype Implementation  

### How to run on a Linux terminal  
```
gcc -o shell shell.c  
./shell  
```  

### Features  
**1. Execute single commands with multiple parameters**  
Example:  
```
ls -la  
```  
<br/><br/>
**2. Execute chained commands using the pipe "|" operator to combine output and input between n commands**  
Example 1:  
```
ls -la | grep "test"  
```
<br/>
Example 2:  
```
cat /proc/cpuinfo | grep "model name" | wc -l  
```  
<br/><br/>
**3. Execute conditional commands with OR (||) and AND (&&) operators**  
Example 1:  
```
cat 12345 || echo "file does not exist"  
```
<br/>
Example 2:  
```
ping -c1 www.unifesp.br.br && echo "SERVER AVAILABLE" || echo "SERVER UNAVAILABLE"  
```  
<br/><br/>
**4. Execute commands in the background, allowing the shell to receive new user commands**  
Example 1:  
```
ping -c5 www.unifesp.br &  
```  
