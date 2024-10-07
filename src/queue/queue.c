#include "./queue.h"

#include <stdbool.h>

// Ve si el proceso a es menor que el proceso b
// (a, pa), (b, pb)
// si a < b            -> (a, pa), (b, pb)
// si a > b            -> (b, pb), (a, pa)
// si a == b y pa < pb -> (a, pa), (b, pb)
// si a == b y pa > pb -> (b, pb), (a, pa)
bool compare_with_priority_double(double a, void *pa, double b, void *pb) {
  if (a == b) return pa < pb;
  return a < b;
}

bool compare_with_priority_int(int a, void *pa, int b, void *pb) {
  if (a == b) return pa < pb;
  return a < b;
}

void queue_push_by_booking_time(Queue *queue, Process *process) {
  queue->size++;

  if (queue->head == NULL || compare_with_priority_double(process->booked_time, process, queue->head->booked_time, queue->head)) {
    process->next = queue->head;
    queue->head = process;
    return;
  }

  Process *current = queue->head;
  while (current->next != NULL && !compare_with_priority_double(process->booked_time, process, current->next->booked_time, current->next)) {
    current = current->next;
  }

  process->next = current->next;
  current->next = process;
}

void queue_push_by_cpu_burst(Queue *queue, Process *process) {
  queue->size++;

  if (queue->head == NULL || compare_with_priority_int(process->cpu_burst, process, queue->head->cpu_burst, queue->head)) {
    process->next = queue->head;
    queue->head = process;
    return;
  }

  Process *current = queue->head;
  while (current->next != NULL && !compare_with_priority_int(process->cpu_burst, process, current->next->cpu_burst, current->next)) {
    current = current->next;
  }

  process->next = current->next;
  current->next = process;
}

Process *queue_pop(Queue *queue) {
  if (queue->head == NULL) return NULL;

  Process *process = queue->head;
  queue->head = process->next;
  process->next = NULL;

  queue->size--;
  return process;
}

Process *queue_get_first(Queue *queue) {
  return queue->head;
}

void queue_remove_by_pointer(Queue *queue, Process *process) {
  if (queue->head == process) {
    queue->head = process->next;
    queue->size--;
    return;
  }

  Process *current = queue->head;
  while (current->next != NULL && current->next != process) {
    current = current->next;
  }
  current->next = process->next;
  process->next = NULL;
  queue->size--;
}
