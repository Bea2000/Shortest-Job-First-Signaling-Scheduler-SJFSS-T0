#include "./debug.h"

#include <stdio.h>
#include <time.h>

char *debug_process_state(ProcessState state) {
  switch (state) {
    case READY:
      return "READY";
    case RUNNING:
      return "RUNNING";
    case WAITING:
      return "WAITING";
    case FINISHED:
      return "FINISHED";
    default:
      return "UNKNOWN";
  }
}

clock_t last_log = 0;
#define SECONDS_TO_LOG 0.5
void debug_log_processes(SJFSS *sjfss) {
  if (clock() < last_log + SECONDS_TO_LOG * CLOCKS_PER_SEC) return;
  last_log = clock();

  printf("(%ld):", 10 * clock() / CLOCKS_PER_SEC);
  for (int i = 0; i < sjfss->total_processes; i++) {
    Process process = sjfss->processes[i];
    printf(" %s(%d)=%s", process.name, process.pid, debug_process_state(process.state));
  }
  printf("  [");

  // Cola running
  printf("R:");
  Process *current = sjfss->runing_processes_queue.head;
  while (current != NULL) {
    printf(" P(%d)=%d", current->pid, (int)(10 * current->booked_time / CLOCKS_PER_SEC));
    current = current->next;
  }
  printf(" || ");

  // Cola waiting (ready to run)
  printf("W:");
  current = sjfss->waiting_queue.head;
  while (current != NULL) {
    printf(" P(%d)=%d", current->pid, current->cpu_burst);
    current = current->next;
  }
  printf(" || ");

  // Cola cooldown (waiting IO)
  printf("C:");
  current = sjfss->cooldown_queue.head;
  while (current != NULL) {
    printf(" P(%d)=%d", current->pid, (int)(10 * current->booked_time / CLOCKS_PER_SEC));
    current = current->next;
  }
  printf("]\n");
}
