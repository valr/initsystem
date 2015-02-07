/*
 * This file is part of initsystem
 *
 * Copyright (C) 2014-2015 Valère Monseur (valere dot monseur at ymail dot com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Both the initscript and the shutdown script have a timeout period of
 * 5 minutes.
 *
 * When the initscript reaches the timeout period, the rest of the process will
 * simply continue i.e. respawn processes are started.
 *
 * When the shutdown script reaches the timeout period, the status of the
 * shutdown process is reset as if it was not executed i.e. respawn processes
 * are (re)started.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#ifndef RESPAWN_CNT
#define RESPAWN_CNT 6
#endif

#ifndef MAX_ARG_NUM
#define MAX_ARG_NUM 8 /* strictly > RESPAWN_CNT */
#endif

#ifndef MAX_STR_LEN
#define MAX_STR_LEN 256
#endif

static sigset_t signal_set;

static char** split_command (char *command)
{
    static char* argv[MAX_ARG_NUM];
    static char arg[MAX_STR_LEN];
    int i = 0, j = 0;

    while (i < MAX_ARG_NUM-1 && *command != '\0')
    {
        while (*command == ' ' || *command == '\t' || *command == '\n')
        {
            command++;
        }

        argv[i++] = &arg[j];

        while (j < MAX_STR_LEN-1 &&
               *command != '\0' && *command != ' ' &&
               *command != '\t' && *command != '\n')
        {
            arg[j++] = *command++;
        }

        arg[j++] = '\0';
    }

    argv[i] = 0;

    return argv;
}

static pid_t spawn_command (char **argv)
{
    pid_t pid = fork ();

    switch (pid)
    {
        case 0:
            sigprocmask (SIG_UNBLOCK, &signal_set, NULL);
            setsid ();
            execvp (*argv, argv);
            sleep (3);
            _exit (EXIT_FAILURE);
            break;

        case -1:
            sleep (3);
            pid = 0;
            break;
    }

    return pid;
}

static void exec_command (char **argv)
{
    execvp (*argv, argv);
}

static time_t get_time ()
{
    struct timespec time;

    clock_gettime (CLOCK_MONOTONIC, &time);

    return time.tv_sec;
}

static void write_wtmp (short type, pid_t pid, char *line, char *id, char *user)
{
    struct utmp utmp = {};
    struct utsname uts = {};
    struct timeval tv = {};

    utmp.ut_type = type;
    utmp.ut_pid = pid;

    strncpy (utmp.ut_line, line, sizeof (utmp.ut_line));
    strncpy (utmp.ut_id, id, sizeof (utmp.ut_id));
    strncpy (utmp.ut_name, user, sizeof (utmp.ut_name));

    if (uname (&uts) == 0)
        strncpy (utmp.ut_host, uts.release, sizeof (utmp.ut_host));

	gettimeofday (&tv, NULL);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;

    updwtmp (WTMP_FILE, &utmp);
}

int main (int argc, char **argv)
{
    pid_t spawn_pid = 0, dead_pid = 0;
    pid_t init_pid = -1;

    int respawn_idx;
    pid_t respawn_pid[RESPAWN_CNT] = {};
    time_t respawn_tim[RESPAWN_CNT] = {};
    char respawn_cmd[MAX_STR_LEN];

    char reexec_cmd[MAX_STR_LEN];
    char reexec_arg[MAX_STR_LEN];

    if (getpid () != 1)
        return EXIT_FAILURE;

    reboot (RB_DISABLE_CAD);
    putenv ("PATH=/sbin:/bin:/usr/sbin:/usr/bin");
    setsid ();
    umask (0);
    chdir ("/");

    sigfillset (&signal_set);
    sigprocmask (SIG_BLOCK, &signal_set, NULL);

    if (argc == 1)
    {
        init_pid = spawn_pid = spawn_command (split_command ("/etc/rc"));
        alarm (300);
    }
    else
    {
        for (respawn_idx = 0; respawn_idx < RESPAWN_CNT; respawn_idx++)
        {
            if (respawn_idx + 1 < argc)
            {
                sscanf (argv[respawn_idx + 1], "%d", &respawn_pid[respawn_idx]);
                memset (argv[respawn_idx + 1], '\0', strlen (argv[respawn_idx + 1]));
            }
        }
    }

    while (1)
    {
        switch (sigwaitinfo (&signal_set, NULL))
        {
            case SIGCHLD:
                while ((dead_pid = waitpid (-1, NULL, WNOHANG)) > 0)
                {
                    if (spawn_pid == dead_pid)
                    {
                        if (spawn_pid == init_pid)
                        {
                            write_wtmp (BOOT_TIME, 0, "~", "~~", "reboot");
                            init_pid = -1;
                        }

                        spawn_pid = 0;
                        alarm (0);
                    }

                    for (respawn_idx = 0; respawn_idx < RESPAWN_CNT; respawn_idx++)
                    {
                        if (respawn_pid[respawn_idx] == 0 || respawn_pid[respawn_idx] == dead_pid)
                        {
                            if (respawn_tim[respawn_idx] <= get_time () && spawn_pid == 0)
                            {
                                sprintf (respawn_cmd, "/etc/rc.respawn %d", respawn_idx + 1);
                                respawn_pid[respawn_idx] = spawn_command (split_command (respawn_cmd));
                                respawn_tim[respawn_idx] = get_time () + 7;
                            }
                            else
                                respawn_pid[respawn_idx] = 0;
                        }
                    }
                }
                break;

            case SIGALRM:
                if (spawn_pid == init_pid)
                {
                    write_wtmp (BOOT_TIME, 0, "~", "~~", "reboot");
                    init_pid = -1;
                }

                spawn_pid = 0;

                for (respawn_idx = 0; respawn_idx < RESPAWN_CNT; respawn_idx++)
                {
                    if (respawn_pid[respawn_idx] == 0)
                    {
                        sprintf (respawn_cmd, "/etc/rc.respawn %d", respawn_idx + 1);
                        respawn_pid[respawn_idx] = spawn_command (split_command (respawn_cmd));
                        respawn_tim[respawn_idx] = get_time () + 7;
                    }
                }
                break;

            case SIGTERM:
                if (spawn_pid == 0)
                {
                    write_wtmp (RUN_LVL, 0, "~~", "~~", "shutdown");
                    spawn_pid = spawn_command (split_command ("/etc/rc.shutdown poweroff"));
                    alarm (300);
                }
                break;

            case SIGUSR1:
                if (spawn_pid == 0)
                {
                    write_wtmp (RUN_LVL, 0, "~~", "~~", "shutdown");
                    spawn_pid = spawn_command (split_command ("/etc/rc.shutdown reboot"));
                    alarm (300);
                }
                break;

            case SIGUSR2:
                if (spawn_pid == 0)
                {
                    write_wtmp (RUN_LVL, 0, "~~", "~~", "shutdown");
                    spawn_pid = spawn_command (split_command ("/etc/rc.shutdown halt"));
                    alarm (300);
                }
                break;

            case SIGQUIT:
                if (spawn_pid == 0)
                {
                    strcpy (reexec_cmd, argv[0]);

                    for (respawn_idx = 0; respawn_idx < RESPAWN_CNT; respawn_idx++)
                    {
                        sprintf (reexec_arg, " %d", respawn_pid[respawn_idx]);
                        strcat (reexec_cmd, reexec_arg);
                    }

                    exec_command (split_command (reexec_cmd));
                }
                break;
        }
    }

    return EXIT_FAILURE;
}
