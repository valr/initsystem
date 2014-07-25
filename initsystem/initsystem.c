/*
 * This file is part of initsystem
 *
 * Copyright (C) 2014 Valère Monseur (valere dot monseur at ymail dot com)
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
 * Both the initscript and the shutdown script have a timeout period of 5 minutes.
 * When the initscript reaches the timeout period, the rest of the process will
 * simply continue. When the shutdown script reaches the timeout period, the
 * status of the shutdown process is reset as if it was not executed. In such
 * a case, the system admin is the only one to know what has to be done
 * depending on where the shutdown script has blocked.
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <sys/prctl.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef MAX_ARG_NUM
#define MAX_ARG_NUM 64
#endif

#ifndef MAX_ARG_LEN
#define MAX_ARG_LEN 4096
#endif

volatile sig_atomic_t signal_number = 0;

void signal_handler (int signum)
{
    signal_number = signum;
}

char** parse_command (char *command)
{
    static char* argv[MAX_ARG_NUM];
    static char arg[MAX_ARG_LEN];
    int i = 0, j = 0;

    while (i < MAX_ARG_NUM-1 && *command != '\0')
    {
        while (*command == ' ' || *command == '\t' || *command == '\n')
        {
            command++;
        }

        argv[i++] = &arg[j];

        while (j < MAX_ARG_LEN-1 &&
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

pid_t spawn_command (char **argv)
{
    pid_t pid;

    pid = fork ();

    switch (pid)
    {
        case 0:
            setsid ();
            execvp (*argv, argv);
            sleep (3);
            _exit (1);
            break;

        case -1:
            pid = 0;
            break;
    }

    return pid;
}

time_t get_time ()
{
    struct timespec time;

    clock_gettime (CLOCK_MONOTONIC, &time);

    return time.tv_sec;
}

int main (int argc, char **argv)
{
    struct sigaction signal_action;
    pid_t init_pid = 0, shutdown_pid = 0, respawn_pid = 0, dead_pid = 0;
    time_t init_timeout = 0, shutdown_timeout = 0;

    if (getpid () != 1)
    {
        return 1;
    }

    reboot (RB_DISABLE_CAD);
    putenv ("PATH=/sbin:/bin:/usr/sbin:/usr/bin");
    umask (0);
    setsid ();
    chdir ("/");

    signal_action.sa_handler = &signal_handler;
    signal_action.sa_flags = SA_RESTART;
    sigfillset (&signal_action.sa_mask);

    sigaction (SIGTERM, &signal_action, 0);
    sigaction (SIGUSR1, &signal_action, 0);
    sigaction (SIGUSR2, &signal_action, 0);

    init_pid = spawn_command (parse_command ("initsystem-initscript.sh"));
    init_timeout = get_time () + 300;

    while (init_pid)
    {
        dead_pid = waitpid (-1, 0, WNOHANG);

        if (dead_pid == init_pid)
            break;

        if (dead_pid <= 0)
            sleep (1);

        if (init_timeout <= get_time ())
            break;
    }

    while (1)
    {
        dead_pid = waitpid (-1, 0, WNOHANG);

        if (dead_pid <= 0)
        {
            dead_pid = -1;
            sleep (1);
        }

        if (respawn_pid == 0 || respawn_pid == dead_pid)
        {
            if (signal_number == 0)
                respawn_pid = spawn_command (parse_command ("initsystem-respawn.sh"));
            else
                respawn_pid = 0;
        }

        if (signal_number != 0)
        {
            if (shutdown_pid == 0)
            {
                switch (signal_number)
                {
                    case SIGTERM:
                        shutdown_pid = spawn_command (parse_command ("initsystem-poweroff.sh"));
                        break;

                    case SIGUSR1:
                        shutdown_pid = spawn_command (parse_command ("initsystem-reboot.sh"));
                        break;

                    case SIGUSR2:
                        shutdown_pid = spawn_command (parse_command ("initsystem-halt.sh"));
                        break;
                }

                shutdown_timeout = get_time () + 300;
            }

            if (shutdown_pid == 0 || shutdown_pid == dead_pid || shutdown_timeout <= get_time ())
            {
                signal_number = 0;
                shutdown_pid = 0;
            }
        }
    }

    return 1;
}
