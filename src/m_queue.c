// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2003 James Haley
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//--------------------------------------------------------------------------
//
// General queue code
//
// By James Haley
//
//-----------------------------------------------------------------------------

#include "z_zone.h"

#include "m_queue.h"

typedef struct mqueueitem_s {
  struct mqueueitem_s *next;
  void *obj;
} mqueueitem_t;

//
// M_QueueInit
//
// Sets up a queue. Can be called again to reset a used queue
// structure.
//
void M_QueueInit(mqueue_t *queue) {
  queue->head.next = NULL;
  queue->tail = &(queue->head);
  queue->rover = &(queue->head);
  queue->size = 0;
}

//
// M_QueueIsEmpty
//
// Returns true if the queue is empty, false otherwise.
//
dboolean M_QueueIsEmpty(mqueue_t *queue) {
  if (queue->head.next == NULL)
    return true;

  return false;
}

//
// M_QueuePush
//
// Inserts the given item into the queue.
//
void M_QueuePush(mqueue_t *queue, void *obj) {
  mqueueitem_t *item = calloc(1, sizeof(mqueueitem_t));
  item->obj = obj;

  queue->tail = queue->tail->next = item;
  queue->tail->next = NULL;
  queue->size++;
}

//
// M_QueuePop
//
// Removes the oldest element in the queue and returns it.
//
void* M_QueuePop(mqueue_t *queue) {
  mqueueitem_t *item = NULL;
  void *obj = NULL;

  if (M_QueueIsEmpty(queue))
    return NULL;

  item = queue->head.next;
  queue->head.next = item->next;
  queue->rover = &(queue->head);

  if (queue->tail == item)
    queue->tail = &(queue->head);

  queue->size--;

  obj = item->obj;

  free(item);
  return obj;
}

//
// M_QueuePeek
//
// Returns the first element of the queue.
//
void* M_QueuePeek(mqueue_t *queue) {
  if (M_QueueIsEmpty(queue))
    return NULL;

  return queue->head.next->obj;
}

//
// M_QueueFree
//
// Frees all the elements in the queue
//
void M_QueueFree(mqueue_t *queue) {
  while (!M_QueueIsEmpty(queue))
    free(M_QueuePop(queue));
}

