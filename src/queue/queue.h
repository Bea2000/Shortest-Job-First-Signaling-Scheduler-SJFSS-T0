#pragma once

#include "../process/process.h"

typedef struct Queue {
  int size;
  Process *head;
} Queue;

void queue_push_by_booking_time(Queue *queue, Process *process);
void queue_push_by_cpu_burst(Queue *queue, Process *process);
Process *queue_pop(Queue *queue);
Process *queue_get_first(Queue *queue);
void queue_remove_by_pointer(Queue *queue, Process *process);
