/*
 ============================================================================
 Name        : hev-task-system-schedule.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2017 everyone.
 Description :
 ============================================================================
 */

#include <stdlib.h>
#include <setjmp.h>

#include "hev-task-system.h"
#include "hev-task-system-private.h"
#include "hev-task-private.h"

#define PRIORITY_COUNT	(HEV_TASK_PRIORITY_MAX - HEV_TASK_PRIORITY_MIN + 1)

static HevTask *running_lists[PRIORITY_COUNT];
static HevTask *waitting_lists[PRIORITY_COUNT][HEV_TASK_YIELD_COUNT];
static HevTask *current_task;
static HevTask *new_task;
static unsigned int task_count;
static jmp_buf kernel_context;

void
hev_task_system_schedule (HevTaskYieldType type, HevTask *_new_task)
{
	int i, count, timeout, priority;
	struct epoll_event events[128];

	new_task = _new_task;

	if (current_task) {
		/* save current task context */
		if (setjmp (current_task->context))
			return; /* resume to task context */

		/* insert current task to waitting list by type */
		priority = hev_task_get_priority (current_task);
		current_task->prev = NULL;
		current_task->next = waitting_lists[priority][type];
		if (waitting_lists[priority][type])
			waitting_lists[priority][type]->prev = current_task;
		waitting_lists[priority][type] = current_task;

		/* resume to kernel context */
		longjmp (kernel_context, 1);
	}

	if (type == HEV_TASK_YIELD_COUNT)
		setjmp (kernel_context);

	/* NOTE: in kernel context */
	if (new_task) {
		priority = hev_task_get_priority (new_task);

		/* insert new task to running list */
		new_task->prev = NULL;
		new_task->next = running_lists[priority];
		if (running_lists[priority])
			running_lists[priority]->prev = new_task;
		running_lists[priority] = new_task;

		/* save context in new task */
		if (setjmp (new_task->context)) {
			/* NOTE: the new task will run at next schedule */

			/* execute new task entry */
			hev_task_execute (current_task);

			hev_task_unref (current_task);
			task_count --;

			longjmp (kernel_context, 1);
		}

		task_count ++;
		new_task = NULL;

		/* for task_run before task_system_run */
		if (!current_task)
			return;
	}

	/* All tasks exited, Bye! */
	if (!task_count)
		return;

	/* schedule */
	timeout = 0;
retry:
	/* io poll */
	count = epoll_wait (hev_task_system_get_epoll_fd (), events, 128, timeout);
	for (i=0; i<count; i++) {
		HevTask *task = events[i].data.ptr;

		hev_task_system_wakeup_task (task);
	}

	/* get a ready task */
	current_task = NULL;
	for (i=HEV_TASK_PRIORITY_MIN; i<=HEV_TASK_PRIORITY_MAX; i++) {
		if (!running_lists[i])
			continue;

		current_task = running_lists[i];
		break;
	}

	/* no task ready */
	if (!current_task) {
		/* move tasks from yield watting list to running list */
		for (i=HEV_TASK_PRIORITY_MIN; i<=HEV_TASK_PRIORITY_MAX; i++) {
			running_lists[i] = waitting_lists[i][HEV_TASK_YIELD];
			waitting_lists[i][HEV_TASK_YIELD] = NULL;
		}

		/* get a ready task again */
		for (i=HEV_TASK_PRIORITY_MIN; i<=HEV_TASK_PRIORITY_MAX; i++) {
			if (!running_lists[i])
				continue;

			current_task = running_lists[i];
			break;
		}
	}

	/* no task ready again, retry */
	if (!current_task) {
		timeout = -1;
		goto retry;
	}

	/* remove task from running list */
	if (current_task->prev) {
		current_task->prev->next = current_task->next;
	} else {
		priority = hev_task_get_priority (current_task);
		running_lists[priority] = current_task->next;
	}
	if (current_task->next) {
		current_task->next->prev = current_task->prev;
	}

	/* apply task's next_priority */
	current_task->priority = current_task->next_priority;

	/* switch to task */
	longjmp (current_task->context, 1);
}

void
hev_task_system_wakeup_task (HevTask *task)
{
	int priority = hev_task_get_priority (task);

	/* remove task from running/waitting list */
	if (task->prev) {
		task->prev->next = task->next;
	} else {
		int j;

		if(running_lists[priority] == task)
			running_lists[priority] = task->next;
		for (j=HEV_TASK_YIELD; j<HEV_TASK_YIELD_COUNT; j++)
			if (waitting_lists[priority][j] == task) {
				waitting_lists[priority][j] = task->next;
				break;
			}
	}
	if (task->next) {
		task->next->prev = task->prev;
	}

	/* insert task to running list */
	task->prev = NULL;
	task->next = running_lists[priority];
	if (running_lists[priority])
		running_lists[priority]->prev = task;
	running_lists[priority] = task;
}

HevTask *
hev_task_system_get_current_task (void)
{
	return current_task;
}

