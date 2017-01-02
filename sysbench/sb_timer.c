/* Copyright (C) 2004 MySQL AB
   Copyright (C) 2004-2017 Alexey Kopytov <akopytov@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "sb_logger.h"
#include "sb_timer.h"

/* Some functions for simple time operations */

/* initialize timer */

void sb_timer_init(sb_timer_t *t)
{
  memset(&t->time_start, 0, sizeof(struct timespec));
  memset(&t->time_end, 0, sizeof(struct timespec));
  sb_timer_reset(t);
}

/* Reset timer counters, but leave the current state intact */

void sb_timer_reset(sb_timer_t *t)
{
  t->min_time = 0xffffffffffffffffULL;
  t->max_time = 0;
  t->sum_time = 0;
  t->events = 0;
  t->queue_time = 0;
}

/* Clone a timer */

void sb_timer_copy(sb_timer_t *to, sb_timer_t *from)
{
  memcpy(to, from, sizeof(sb_timer_t));
}

/* check whether the timer is running */

bool sb_timer_running(sb_timer_t *t)
{
  return TIMESPEC_DIFF(t->time_start, t->time_end) > 0;
}

/*
  get time elapsed since the previous call to sb_timer_checkpoint() for the
  specified timer without stopping it.  The first call returns time elapsed
  since the timer was started.
*/

uint64_t sb_timer_checkpoint(sb_timer_t *t)
{
  struct timespec tmp;
  uint64_t        res;

  SB_GETTIME(&tmp);
  res = TIMESPEC_DIFF(tmp, t->time_start);
  t->time_start = tmp;

  return res;
}

/* get average time per event */


unsigned long long get_avg_time(sb_timer_t *t)
{
  if(t->events == 0)
    return 0; /* return zero if there were no events */
  return (t->sum_time / t->events);
}


/* get total time for all events */


unsigned long long get_sum_time(sb_timer_t *t)
{
  return t->sum_time;
}


/* get minimum time */


unsigned long long  get_min_time(sb_timer_t *t)
{
  return t->min_time;
}


/* get maximum time */


unsigned long long  get_max_time(sb_timer_t *t)
{
  return t->max_time;
}


/* sum data from several timers. used in summing data from multiple threads */


sb_timer_t merge_timers(sb_timer_t *t1, sb_timer_t *t2)
{
  sb_timer_t t;

  /* Initialize to avoid warnings */
  memset(&t, 0, sizeof(sb_timer_t));

  t.sum_time = t1->sum_time+t2->sum_time;
  t.events = t1->events+t2->events;

  if (t1->max_time > t2->max_time)
    t.max_time = t1->max_time;
  else 
    t.max_time = t2->max_time;

  if (t1->min_time<t2->min_time)
    t.min_time = t1->min_time;
  else 
    t.min_time = t2->min_time;
     
  return t;       
}
