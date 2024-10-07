#pragma once

#include <unistd.h>

typedef enum ProcessState {
  READY,
  RUNNING,
  WAITING,
  FINISHED
} ProcessState;

typedef struct Process {
  /* Valores constantes */

  // Prioridad
  int priority;
  // Nombre
  char *name;
  // Cuando parte el proceso
  int start_time;
  // Cuanto tiempo se mantendrá en la CPU
  int cpu_burst;
  // Cuanto tiempo tendrá que esperar en la cola
  int io_burst;
  // Path del ejecutable
  char *path;
  // Número de argumentos
  int argc;
  // Argumentos
  char **argv;

  /* Valores dinámicos */

  // Estado del proceso
  ProcessState state;
  // ID del proceso, parte en 0
  pid_t pid;
  // Cuando un proceso tiene su tiempo agendado
  double booked_time;
  // Siguiente proceso en la cola
  struct Process *next;

  /* Estadisticas */
  int cpu_turns;
  double turnaround_time;
  double response_time;
  double waiting_time;
  // Cuando el proceso terminó de ejecutarse (-1 si no ha terminado)
  double finished_time;
  // Código de salida del proceso
  int exit_code;
} Process;

void process_init(Process *process, int priority, char *name, int start_time, int cpu_burst, int io_burst, char *path, int argc, char **argv);
void process_run(Process *process);
void process_stop(Process *process);
void process_destroy(Process *process);
