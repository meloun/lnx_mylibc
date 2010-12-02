/* s2e_common.c - sub routine */

/* Copyright (c) 2008, Nissin Systems Co., Ltd. All rights reserved. */

/*
modification history
--------------------
01a,05aug08,nss  written.
*/

#include "s2e_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_FD_GUESS 1024

static void close_all_fd(int startingFd)
{
    int i;
    int maxFd = sysconf(_SC_OPEN_MAX);

    if (maxFd <= 0)
        maxFd = MAX_FD_GUESS;   
    
    for (i = startingFd; i <= maxFd; ++i)
    {
        close(i);
    }
}

int make_daemon(const char *path, char *argv[])
{
    int   rc = 0;	
    pid_t pid1;

    /*
        Two fork calls are required to fully disassociate
        from the parent process. It will also be necessary
        for the child to remap stdin/stdout/stderr to /dev/null,
        and close all remaining file descriptors.  
    */

	pid1 = vfork();
	if (pid1 == 0) { /* child */
        pid_t pid2;

        setsid();

        pid2 = vfork();
        if (pid2 == 0) /* child */
        {
            int fd;

            /*
                Remap stdin, stdout, and stderr to /dev/null
            */
            fd = open("/dev/null", O_RDWR);
            if (fd > 2)
            {
                dup2(fd, STDIN_FILENO); /* STDIN */
                dup2(fd, STDOUT_FILENO); /* STDOUT */
                dup2(fd, STDERR_FILENO); /* STDERR */
            }
            close(fd);
            close_all_fd(STDERR_FILENO+1);
            chdir("/");
            umask(0);                 

		    execv(path, (char * const *) argv);
		    /* We should never get here, unless execv() fails */
		    exit(1);
        }
        else if (pid2 != -1) /* parent */
        {
            /* Exit with successful status */
            _exit(0);    
        }
        else /* pid2 is -1, error calling vfork() */
        {
            /* Exit with error status */
            _exit(1);
        }
	}
    else if (pid1 > 0)  /* parent */
    {
        int lrc;
        
        /*
            Wait for child to exit. Note that the child
            will fork to create our grandchild and then exit.
        */

        lrc = waitpid(pid1, NULL, 0);
        if (lrc == -1 || WEXITSTATUS(lrc) != 0)
        {
            /* Error encountered by child */
            rc = -1;
        }
    }
    else /* Error calling vfork() */
    {
        rc = -1;
    }
    
    return rc;    
}

int execute (char *command)
{
	int argc;
    int rc;
	char *argv[16];
	char buffer[256];

	if (strstr (command, "${") != NULL) {
		char *ptr = command;
		int bufcnt = 0;

		while (*ptr != '\0') {

			if (strncmp (ptr, "${", 2) == 0) {
				char *curr = &buffer[bufcnt];
				int n = 0;
				char *env;

				ptr += 2;
				while (*ptr != '\0') {
					if (*ptr == '}') {
						ptr ++;
						break;
					}
					if (n < sizeof(buffer) - 1 - bufcnt)
						curr[n++] = *ptr;
					ptr ++;
				}
				curr[n] = '\0';

				if ((env = getenv (curr)) != NULL) {
					strncpy (&buffer[bufcnt], env, 
						sizeof(buffer) - 1 - bufcnt);
					bufcnt += strlen (&buffer[bufcnt]);
				}

			} else {
				if (bufcnt < sizeof(buffer) - 1)
					buffer[bufcnt ++] = *ptr;
				ptr ++;
			}
		}
		buffer[bufcnt] = '\0';

	} else
		strncpy (buffer, command, sizeof(buffer) - 1);

	argc = parge_argv (argv, sizeof(argv) / sizeof (char *), buffer);

    rc = make_daemon(argv[0], argv);

	return rc;
}

int child (char *buffer, int size, char *command)
{
	FILE *fd;
	char temp[256];
	int len;
	int total = 0;
	int count = 0;

	if ((fd = popen (command, "r")) < 0) {
		return -1;
	}

	if (buffer)
		buffer[0] = '\0';

	while (1) {
		if (fgets (temp, sizeof(temp) - 1, fd) == NULL) {
			if (count < 10 && errno == EINTR) {
				count ++;
				continue;
			}
	    	break;
		}

		len = strlen (temp);
		if (size && size <= len + total)
			len = size - total;

		if (buffer) {
			memcpy (&buffer[total], temp, len);
		} else
			printf ("%s", temp);

		total += len;

		if (size && size <= total)
			break;
	}

	if (buffer)
		buffer[total] = '\0';

	pclose (fd);
    return 0;
}

void msleep (long msec)
    {
    struct timespec req;
    struct timespec rem;

    req.tv_sec  = msec / 1000;
    req.tv_nsec = (msec % 1000) * 1000 * 1000;
    rem.tv_sec  = 0;
    rem.tv_nsec = 0;

    nanosleep(&req, &rem);
    }

int getpid_by_file (char *pid_file)
{
	int pid = -1;
	FILE*fp;
	char buffer[16];

	if ((fp = fopen (pid_file, "r")) != NULL) {
		if (fgets(buffer, sizeof (buffer), fp)!=NULL)
			pid = atoi (buffer);
		fclose (fp);
	}

	return pid;
}

int terminate_process (char *pid_file, int timeout, char* cmdName)
{
	int pid;

	if (0 < (pid = getpid_by_file (pid_file))) {
        if (!is_active_proc(pid, cmdName))
        {
            unlink(pid_file);
		    return 0;
		}

		kill (pid, SIGTERM);

		if (timeout) {
			while (0 < (pid = getpid_by_file (pid_file)) && 0 < timeout) {
				msleep (100);
				timeout -= 100;
			}
			if (timeout <= 0)
				return -1;
		}
	}

	return 0;
}

long hatol (const char *s)
{
	unsigned long total = 0;
	unsigned long value;
	char c;

	if (s == (char *)0)
		return -1;

	while ((c = *s++) != '\0') {

		if (total & 0x80000000)
			return -1;

		if ('0' <= c && c <= '9')
			c -= '0';
		else if ('a' <= c && c <= 'f')
			c -= ('a' - 10);
		else if ('A' <= c && c <= 'F')
		c -= ('A' - 10);
		else
			return -1;

	    value = total * 16 + ((unsigned long)c & 0xf);

		if (value < total)
			return -1;

		total = value;
	}

	return total;
}

char *
skip_brank (char *ptr)
{
	if (!ptr)
		return NULL;
	while (*ptr == ' ' || *ptr == '\t')
		ptr ++;
	return ptr;
}

char *
get_token (char *ptr, char *token, int len)
{
	int i = 0;
	if (!ptr || !token)
		return NULL;
	ptr = skip_brank (ptr);
	while (' ' < *ptr && i < (len - 1))
		token[i++] = *ptr ++;
	while (' ' < *ptr)
		ptr++;
	token[i] = '\0';

	return ptr;
}

int 
parge_argv (char *argv[], int args, char *command)
{
	char *curr;
	char *next;
	int argc = 0;
	int flag = 0;

	curr = command;
	curr = skip_brank (curr);

	while (*curr) {
		next = curr;
		curr = skip_brank (curr);

		while (*next != '\0') {
			if (*next == '"')
				flag = 1 - flag;
			if ((*next == ' ' || *next == '\t') && flag == 0){
				*next++ = '\0';
				break;
			}
			next ++;
		}

		if (argc < (args - 1))
			argv[argc++] = curr;
		curr = next;
	}
	argv[argc] = NULL;

	return argc;
}

int fileExists(char* filePath)
{
    struct stat statBuf;
    int         fileExists = 0;

    if (stat(filePath, &statBuf) == 0)
    {
        fileExists = 1;    
    }

    return fileExists;
}

int is_active_proc(int pid, char* cmdName)
{
    int activeProc = 0;

    /*
        Make sure we have a valid pid.
    */      

    if (pid > 0)
    {
        struct stat statBuf;
        char procEntry[64];

        /*
            See if a process with a matching pid is running.
        */

        snprintf(procEntry, sizeof(procEntry), "/proc/%d", pid);
        if (stat(procEntry, &statBuf) == 0 && 
            S_ISDIR(statBuf.st_mode))
        {
            FILE* fp;

            /* Process is running, but is it really s2e? */  
    
            strncat(procEntry, "/cmdline", sizeof(procEntry));
            fp = fopen(procEntry, "r");
            if (fp)
            {
                char lineBuf[128];
                
                if (fgets(lineBuf, sizeof(lineBuf), fp) != NULL &&
                    strstr(lineBuf, cmdName) != NULL)
                {
                    /* 
                        This appears to be an active s2e process. Note that we
                        may get a false positive if the process is not running
                        s2e, but has "s2e" in its command line string. This seems
                        very unlikely.
                    */
                    activeProc = 1;    
                } /* if (fgets( ... */
                
                fclose(fp);  
            } /* if (fp ... */  
        }  /* if (stat( ... */
    } /* if (pid > 0 ... */
    
    return activeProc;    
}
