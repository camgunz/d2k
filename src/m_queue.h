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

#ifndef M_QUEUE_H__
#define M_QUEUE_H__

typedef struct mqueue_s {
  mqueueitem_t head;
  mqueueitem_t *tail;
  mqueueitem_t *rover;
  unsigned int size;
} mqueue_t;

void     M_QueueInit(mqueue_t *queue);
dboolean M_QueueIsEmpty(mqueue_t *queue);
void     M_QueuePush(mqueue_t *queue, void *obj);
void*    M_QueuePop(mqueue_t *queue);
void*    M_QueuePeek(mqueue_t *queue);
void     M_QueueFree(mqueue_t *queue);

#endif

