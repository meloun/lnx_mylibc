/* s2e_common.h - s2e common header */

/* Copyright (c) 2009, Lantronix Inc. All rights reserved. */


#ifndef _S2E_COMMON_H
#define _S2E_COMMON_H

#define S2E_CMD_NAME "s2e"

int execute (char *command);
int child (char *buffer, int size, char *command);
void msleep (long msec);
int getpid_by_file (char *pid_file);
int terminate_process (char *pid_file, int timeout, char* cmdName);
long hatol (const char *s);
char *skip_brank (char *ptr);
char *get_token (char *ptr, char *token, int len);
int parge_argv (char *argv[], int args, char *command);
int fileExists(char* filePath);
int is_active_proc(int pid, char* cmdName);

#endif	/* _SUBR_H */
