#include <math.h>
#include <signal.h>
#include <stdio.h>   // FILE, fopen, fclose, etc.
#include <stdlib.h>  // malloc, calloc, free, etc
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "../file_manager/manager.h"
#include "../process/process.h"
#include "../queue/queue.h"
#include "./debug.h"
#include "./sjfss.h"

SJFSS sjfss;

void handle_interruption(int sig) {
  printf("Voy a terminar el programa\n");

  double start_time = clock();
  while (sjfss_has_running_process(&sjfss) && (clock() - start_time) / CLOCKS_PER_SEC < 5.0) {
    sjfss_from_running_to_finished(&sjfss);
  };

  stfss_destroy(&sjfss);
  exit(1);  // Tenemos que terminar el programa
}

int main(int argc, char const *argv[]) {
  char *file_name = (char *)argv[1];
  char *output_file_name = (char *)argv[2];

  InputFile *input_file = read_file(file_name);

  bool is_multicore = argc == 4 && strcmp(argv[3], "--multicore") == 0;
  printf("Algoritmo: SJFSS con %s\n", is_multicore ? "multicore" : "monocore");

  printf("Nombre archivo: %s\n", file_name);
  printf("Cantidad de procesos: %d\n", input_file->len);

  sjfss_init(&sjfss, input_file->len, is_multicore ? 2 : 1);
  signal(SIGINT, handle_interruption);

  for (int i = 0; i < sjfss.total_processes; ++i) {
    char *name = input_file->lines[i][0];
    int start_time = atoi(input_file->lines[i][1]);
    int cpu_burst = atoi(input_file->lines[i][2]);
    int io_burst = atoi(input_file->lines[i][3]);
    char *path = input_file->lines[i][4];
    int argc = atoi(input_file->lines[i][5]);
    sjfss_add_process(&sjfss, i, name, start_time, cpu_burst, io_burst, path, argc, &input_file->lines[i][6]);
  }
  input_file_destroy(input_file);

  debug_log_processes(&sjfss);

  // // DEBUG
  process_run(&sjfss.processes[0]);
  wait(&sjfss.processes[0].pid);

  while (sjfss_has_work(&sjfss)) {
    debug_log_processes(&sjfss);
    // [WAITING -> READY]
    sjfss_from_cooldown_to_waiting(&sjfss);
    // [READY   -> RUNNING]
    sjfss_from_waiting_to_running(&sjfss);
    // [RUNNING -> FINISHED]
    sjfss_from_running_to_finished(&sjfss);
    // [RUNNING -> WAITING]
    sjfss_from_running_to_cooldown(&sjfss);
  }

  FILE *output_file = fopen(output_file_name, "w");
  if (output_file == NULL) {
    printf("Error al abrir el archivo de salida\n");
    exit(1);
  }
  for (int i = 0; i < sjfss.total_processes; ++i) {
    Process p = sjfss.processes[i];
    fprintf(output_file,
            "%s,%d,%d,%d,%d,%d,%d\n",
            p.name,
            p.pid,
            p.cpu_turns,
            (int)round(p.turnaround_time / CLOCKS_PER_SEC),
            (int)round(p.response_time / CLOCKS_PER_SEC),
            (int)round(p.waiting_time / CLOCKS_PER_SEC),
            p.exit_code);
  }
  fclose(output_file);
}