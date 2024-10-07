#include "./sjfss.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

void sjfss_init(SJFSS *sjfss, int size, int cores) {
  sjfss->total_processes = size;
  sjfss->processes = malloc(sizeof(Process) * size);

  // Valores iniciales de las colas
  sjfss->max_cpu_process = cores;
  sjfss->cooldown_queue.size = 0;
  sjfss->cooldown_queue.head = NULL;
  sjfss->waiting_queue.size = 0;
  sjfss->waiting_queue.head = NULL;
  sjfss->runing_processes_queue.size = 0;
  sjfss->runing_processes_queue.head = NULL;
}

void stfss_destroy(SJFSS *sjfss) {
  for (int i = 0; i < sjfss->total_processes; ++i) process_destroy(&sjfss->processes[i]);
  free(sjfss->processes);
}

void sjfss_add_process(SJFSS *sjfss, int index, char *name, int start_time, int cpu_burst, int io_burst, char *path, int argc, char **argv) {
  process_init(&sjfss->processes[index], index, name, start_time, cpu_burst, io_burst, path, argc, argv);
  queue_push_by_booking_time(&sjfss->cooldown_queue, &sjfss->processes[index]);
}

bool sjfss_has_running_process(SJFSS *sjfss) {
  return sjfss->runing_processes_queue.size > 0;
}

bool sjfss_has_work(SJFSS *sjfss) {
  return sjfss->waiting_queue.size > 0 || sjfss->cooldown_queue.size > 0 || sjfss->runing_processes_queue.size > 0;
}

// RUNNING -> WAITING
void sjfss_from_running_to_cooldown(SJFSS *sjfss) {
  Process *process;
  while ((process = queue_get_first(&sjfss->runing_processes_queue)) != NULL && process->booked_time <= clock()) {
    process->waiting_time += process->io_burst * CLOCKS_PER_SEC;

    queue_pop(&sjfss->runing_processes_queue);
    process->state = WAITING;
    process->booked_time = clock() + process->io_burst * CLOCKS_PER_SEC;
    process_stop(process);
    queue_push_by_booking_time(&sjfss->cooldown_queue, process);
  }
}

// WAITING -> READY
void sjfss_from_cooldown_to_waiting(SJFSS *sjfss) {
  Process *process;
  while ((process = queue_get_first(&sjfss->cooldown_queue)) != NULL && process->booked_time <= clock()) {
    queue_pop(&sjfss->cooldown_queue);
    process->state = READY;
    queue_push_by_cpu_burst(&sjfss->waiting_queue, process);
  }
}

// READY -> RUNNING
void sjfss_from_waiting_to_running(SJFSS *sjfss) {
  Process *process;
  while ((process = queue_get_first(&sjfss->waiting_queue)) != NULL && sjfss->runing_processes_queue.size < sjfss->max_cpu_process) {
    // Antes de pasarlo a run
    double time_since_added = clock() - process->booked_time;
    process->waiting_time += time_since_added;
    if (process->response_time == -1) process->response_time = time_since_added;
    process->cpu_turns++;

    // Se pasa a run
    queue_pop(&sjfss->waiting_queue);
    process_run(process);
    process->state = RUNNING;
    process->booked_time = clock() + process->cpu_burst * CLOCKS_PER_SEC;
    queue_push_by_booking_time(&sjfss->runing_processes_queue, process);
  }
}

// RUNNING -> FINISHED
// https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-waitpid-wait-specific-child-process-end
void sjfss_from_running_to_finished(SJFSS *sjfss) {
  int status;
  for (int i = 0; i < sjfss->total_processes; i++) {
    Process *p = &sjfss->processes[i];
    if (p->state != RUNNING) continue;

    int pid = waitpid(p->pid, &status, WNOHANG);
    if (pid == -1) continue;  // error
    if (pid == 0) continue;   // sigue corriendo

    if (WIFEXITED(status)) {  // Este if en algunas partes causa que el cógido no termine
      queue_remove_by_pointer(&sjfss->runing_processes_queue, p);
      p->state = FINISHED;
      p->finished_time = clock();
      p->exit_code = WEXITSTATUS(status);
      p->turnaround_time = p->finished_time - p->start_time * CLOCKS_PER_SEC;
    }
  }
}
