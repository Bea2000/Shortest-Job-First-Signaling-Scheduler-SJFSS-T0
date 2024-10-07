#pragma once
#include <stdbool.h>

#include "../queue/queue.h"

typedef struct SJFSS {
  int total_processes;
  int max_cpu_process;
  Process *processes;

  Queue waiting_queue;
  Queue cooldown_queue;
  Queue runing_processes_queue;
} SJFSS;

void sjfss_init(SJFSS *sjfss, int size, int cores);
void sjfss_add_process(SJFSS *sjfss, int index, char *name, int start_time, int cpu_burst, int io_burst, char *path, int argc, char **argv);
bool sjfss_has_work(SJFSS *sjfss);
bool sjfss_has_running_process(SJFSS *sjfss);
void sjfss_from_running_to_cooldown(SJFSS *sjfss);
void sjfss_from_cooldown_to_waiting(SJFSS *sjfss);
void sjfss_from_waiting_to_running(SJFSS *sjfss);
void sjfss_from_running_to_finished(SJFSS *sjfss);
void stfss_destroy(SJFSS *sjfss);
