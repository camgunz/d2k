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

#include "doom.h"

#include "m_queue.h"

void M_QueueInit(mqueue_t *queue) {
  queue->head.next = NULL;
  queue->tail = &queue->head;
  queue->size = 0;
}

dboolean M_QueueIsEmpty(mqueue_t *queue) {
  if (queue->head.next == NULL)
    return true;

  return false;
}

void M_QueuePush(mqueue_t *queue, void *obj) {
  mqueueitem_t *item = calloc(1, sizeof(mqueueitem_t));
  item->obj = obj;

  queue->tail = queue->tail->next = item;
  queue->tail->next = NULL;
  queue->size++;
}

void* M_QueuePop(mqueue_t *queue) {
  mqueueitem_t *item = NULL;
  void *obj = NULL;

  if (M_QueueIsEmpty(queue))
    return NULL;

  item = queue->head.next;
  queue->head.next = item->next;

  if (queue->tail == item)
    queue->tail = &queue->head;

  queue->size--;

  obj = item->obj;

  free(item);
  return obj;
}

void* M_QueuePeek(mqueue_t *queue) {
  if (M_QueueIsEmpty(queue))
    return NULL;

  return queue->head.next->obj;
}

void M_QueueFree(mqueue_t *queue) {
  while (!M_QueueIsEmpty(queue))
    free(M_QueuePop(queue));
}

/* vi: set et sw=2 ts=2: */

