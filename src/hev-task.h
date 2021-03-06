/*
 ============================================================================
 Name        : hev-task.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2017 everyone.
 Description :
 ============================================================================
 */

#ifndef __HEV_TASK_H__
#define __HEV_TASK_H__

#include <sys/epoll.h>

#define HEV_TASK_PRIORITY_MIN	(0)
#define HEV_TASK_PRIORITY_MAX	(15)

#define HEV_TASK_PRIORITY_HIGH	HEV_TASK_PRIORITY_MIN
#define HEV_TASK_PRIORITY_LOW	HEV_TASK_PRIORITY_MAX

typedef struct _HevTask HevTask;
typedef enum _HevTaskState HevTaskState;
typedef enum _HevTaskYieldType HevTaskYieldType;
typedef void (*HevTaskEntry) (void *data);

/**
 * HevTaskState:
 * @HEV_TASK_STOPPED: The task is not in any task system.
 * @HEV_TASK_RUNNING: The task is in a task system's running list.
 * @HEV_TASK_WAITING: The task is in a task system's waiting list.
 *
 * Since: 1.0
 */
enum _HevTaskState
{
	HEV_TASK_STOPPED,
	HEV_TASK_RUNNING,
	HEV_TASK_WAITING,
};

/**
 * HevTaskYieldType:
 * @HEV_TASK_YIELD: Move task to yield waiting list.
 * @HEV_TASK_WAITIO: Move task to I/O waiting list.
 * @HEV_TASK_YIELD_COUNT: Maximum yield type count.
 *
 * Since: 1.0
 */
enum _HevTaskYieldType
{
	HEV_TASK_YIELD,
	HEV_TASK_WAITIO,
	HEV_TASK_YIELD_COUNT,
};

/**
 * hev_task_new:
 * @stack_size: stack size for task
 *
 * Creates a new task. If @stack_size = -1, the default stack size
 * will be used.
 *
 * Returns: a new #HevTask.
 *
 * Since: 1.0
 */
HevTask * hev_task_new (int stack_size);

/**
 * hev_task_ref:
 * @self: a #HevTask
 *
 * Increases the reference count of the @self by one.
 *
 * Returns: a #HevTask
 *
 * Since: 1.0
 */
HevTask * hev_task_ref (HevTask *self);

/**
 * hev_task_unref:
 * @self: a #HevTask
 *
 * Decreases the reference count of @self. When its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 *
 * Since: 1.0
 */
void hev_task_unref (HevTask *self);

/**
 * hev_task_self:
 *
 * Get the current task.
 *
 * Returns: a #HevTask
 *
 * Since: 1.0
 */
HevTask * hev_task_self (void);

/**
 * hev_task_get_state:
 * @self: a #HevTask
 *
 * Get the state of a task.
 *
 * Returns: a #HevTaskState
 *
 * Since: 1.0
 */
HevTaskState hev_task_get_state (HevTask *self);

/**
 * hev_task_set_priority:
 * @self: a #HevTask
 * @priority: priority
 *
 * Set the priority of a task. The value range of priority are [0-15],
 * with smaller values representing higher priorities.
 *
 * Since: 1.0
 */
void hev_task_set_priority (HevTask *self, int priority);

/**
 * hev_task_get_priority:
 * @self: a #HevTask
 *
 * Get the priority of a task.
 *
 * Returns: current priority of task
 *
 * Since: 1.0
 */
int hev_task_get_priority (HevTask *self);

/**
 * hev_task_add_fd:
 * @self: a #HevTask
 * @fd: a file descriptor
 * @events: a epoll events. (e.g. EPOLLIN, EPOLLOUT)
 *
 * Add a file descriptor to I/O poll queue of task system. The task system
 * will wake up the task when I/O events ready.
 *
 * Returns: When successful, returns zero. When an error occurs, returns -1.
 *
 * Since: 1.0
 */
int hev_task_add_fd (HevTask *self, int fd, unsigned int events);

/**
 * hev_task_mod_fd:
 * @self: a #HevTask
 * @fd: a file descriptor
 * @events: a epoll events.
 *
 * Modify events of a file descriptor that added into I/O poll queue of
 * task system.
 *
 * Returns: When successful, returns zero. When an error occurs, returns -1.
 *
 * Since: 1.0
 */
int hev_task_mod_fd (HevTask *self, int fd, unsigned int events);

/**
 * hev_task_mod_fd:
 * @self: a #HevTask
 * @fd: a file descriptor
 *
 * Remove a file descriptor from I/O poll queue of task system.
 *
 * Returns: When successful, returns zero. When an error occurs, returns -1.
 *
 * Since: 1.0
 */
int hev_task_del_fd (HevTask *self, int fd);

/**
 * hev_task_wakeup:
 * @self: a #HevTask
 *
 * Wake up a task. Don't switch tasks immediately.
 *
 * Since: 1.0
 */
void hev_task_wakeup (HevTask *task);

/**
 * hev_task_yield:
 * @type: type of #HevTaskYieldType
 *
 * Save current task context, pick a new task and switch to.
 *
 * Since: 1.0
 */
void hev_task_yield (HevTaskYieldType type);

/**
 * hev_task_sleep:
 * @milliseconds: time to sleep
 *
 * Like yield. The task will be waked up by two condition:
 * 1. Timer. time has elapsed.
 * 2. I/O events. The task will be waked up by file descriptors events.
 *
 * Returns: Zero if the requested time has elapsed, or
 * the number of milliseconds left to sleep.
 *
 * Since: 1.0
 */
unsigned int hev_task_sleep (unsigned int milliseconds);

/**
 * hev_task_run:
 * @self (transfer full): a #HevTask
 * @entry: A #HevTaskEntry
 * @data (nullable): a user data to passed to @entry
 *
 * Set the entry and data to a task, and added to running list of task system.
 * The task will be run unitl task system run.
 *
 * Since: 1.0
 */
void hev_task_run (HevTask *self, HevTaskEntry entry, void *data);

#endif /* __HEV_TASK_H__ */

