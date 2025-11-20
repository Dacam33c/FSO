# Shell para Escalonamento Preemptivo-Interrupt (FSO 02/2025)

## 1. Objetivo

Implementar um shell (`shell_sched`) que gerencia um processo escalonador (`user_scheduler`) capaz de executar um **escalonamento preemptivo-interrupt com prioridades estáticas**. O escalonador utiliza múltiplas filas **Round-Robin** e mecanismos de Comunicação Interprocessos (IPC) Unix, como pipes e sinais.

## 2. Arquitetura e Comunicação

O projeto é composto por três executáveis principais:

* **`shell_sched` (Processo Pai):** Interface com o usuário. Cria o processo `user_scheduler` e envia comandos via **pipe**.
* **`user_scheduler` (Processo Filho/Escalonador):** Implementa a lógica de escalonamento. Recebe comandos via pipe, gerencia as filas de prioridade e controla a execução dos processos filhos usando **sinais (`SIGSTOP`, `SIGCONT`, `SIGALRM`, `SIGCHLD`)**.
* **`cpu_bound` (Processos Escalados):** Programa de teste que simula uma carga de trabalho intensa (CPU-bound) com duração de cerca de 20 segundos de tempo de CPU.

### Regras de Escalonamento

* **Filas:** O escalonador suporta 2 ou 3 filas Round-Robin.
* **Prioridade:** A prioridade **1 é a mais alta**.
* **Preempção:** Caso um novo processo com prioridade maior seja requisitado, o processo em execução é interrompido e colocado no **início de sua fila** com o quantum restante, e o processo de maior prioridade é executado.
* **IPC:** É obrigatório o uso de IPCs Unix (pipes, sinais, etc.), sendo vedado o uso de `semaphore` da biblioteca `pthread`.

## 3. Comandos do Shell

O `shell_sched` aceita os seguintes comandos:

| Comando | Descrição |
| :--- | :--- |
| **`create_user_scheduler <N>`** | Cria o processo escalonador e inicializa $N$ filas de prioridade (N=2 ou N=3). |
| **`execute_process <cmd> <prio>`** | Cria um novo processo (`cmd`) e o insere no final da fila de prioridade `<prio>`. |
| **`list_scheduler`** | Apresenta o processo em execução e todos os processos nas filas de prioridade. |
| **`exit_scheduler`** | Termina o escalonador, exibindo o **tempo de *turnaround*** de cada processo finalizado e listando os que não terminaram. |

---

## 4. Como Executar

### Pré-requisitos

* GCC (Compilador C)
* Ambiente Unix-like (Linux, WSL ou MacOS).

### Instruções

1.  **Clone o repositório:**
    ```bash
    git clone https://github.com/Dacam33c/FSO.git
    cd FSO
    ```

2.  **Compile o projeto:**
    ```bash
    make
    ```

3.  **Inicie o shell:**
    ```bash
    ./shell_sched
    ```

4.  **Exemplo de uso:**
    ```bash
    >shell_sched: create_user_scheduler 3
    >shell_sched: execute_process ./cpu_bound 3
    >shell_sched: execute_process ./cpu_bound 1
    >shell_sched: list_scheduler
    >shell_sched: exit_scheduler
    ```

## 5. Documentação

O código-fonte (`shell_sched.c` e `user_scheduler.c`) contém no cabeçalho o nome e matrícula dos alunos, além das versões do compilador e do sistema operacional.