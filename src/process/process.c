#include "process.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @param priority Prioridad de salir de ready
 * @param name
 * @param start_time Tiempo de partida en segundos
 * @param cpu_burst Tiempo que se mantendrá en la CPU
 * @param io_burst Tiempo que tendrá que esperar en la cola luego de salir de la CPU
 * @param path Path al ejecutable
 * @param argc Número de argumentos
 * @param argv Argumentos
 */
void process_init(Process *process, int priority, char *name, int start_time, int cpu_burst, int io_burst, char *path, int argc, char **argv) {
  process->cpu_burst = cpu_burst;
  process->io_burst = io_burst;
  process->start_time = start_time;
  process->priority = priority;

  process->path = malloc(sizeof(char) * (strlen(path) + 1));
  strcpy(process->path, path);

  process->argc = argc;

  process->state = WAITING;

  process->name = malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(process->name, name);

  process->argv = malloc(sizeof(char *) * (argc + 1));  // +1 por que termina en NULL

  for (int i = 0; i < argc; i++) {
    process->argv[i] = malloc(sizeof(char) * (strlen(argv[i]) + 1));  // +1 por que termina en NULL
    strcpy(process->argv[i], argv[i]);
    process->argv[i][strlen(argv[i])] = '\0';
  }
  process->argv[argc] = NULL;

  process->pid = 0;

  process->booked_time = clock() + CLOCKS_PER_SEC * (double)start_time;
  process->finished_time = -1;
  process->cpu_turns = 0;
  process->turnaround_time = 0;
  process->waiting_time = 0;
  process->response_time = -1;
}

void process_run(Process *process) {
  if (process->pid == 0) {
    // Tenemos que crear un proceso
    int pid = fork();
    if (pid == 0) {
      execvp(process->path, process->argv);
      printf("El proceso hijo %d no se pudo ejecutar\n", pid);
      exit(1);
    } else {
      process->pid = pid;
    }
  } else {
    // Tenemos que continuar un proceso
    kill(process->pid, SIGCONT);
  }
}

void process_stop(Process *process) {
  if (process->pid != 0) {
    kill(process->pid, SIGSTOP);
  }
}

void process_destroy(Process *process) {
  if (process->pid != 0) kill(process->pid, SIGKILL);

  for (int i = 0; i < process->argc; i++) free(process->argv[i]);
  free(process->argv);
  free(process->name);
  free(process->path);
}
