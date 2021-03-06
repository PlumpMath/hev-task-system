/*
 ============================================================================
 Name        : hev-task-system.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2017 everyone.
 Description :
 ============================================================================
 */

#ifndef __HEV_TASK_SYSTEM_H__
#define __HEV_TASK_SYSTEM_H__

/**
 * hev_task_system_init:
 *
 * Initialize the task system.
 *
 * Returns: When successful, returns zero. When an error occurs, returns -1.
 *
 * Since: 1.0
 */
int hev_task_system_init (void);

/**
 * hev_task_system_fini:
 *
 * Finalize the task system.
 *
 * Since: 1.0
 */
void hev_task_system_fini (void);

/**
 * hev_task_system_run:
 *
 * Run the task system.
 *
 * Since: 1.0
 */
void hev_task_system_run (void);

#endif /* __HEV_TASK_SYSTEM_H__ */

