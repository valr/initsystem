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

#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
            sleep (1);
            _exit (1);
            break;

        case -1:
            pid = 0;
            break;
    }

    return pid;
}

int main (int argc, char **argv)
{
    struct sigaction signal_action;
    pid_t init_pid = 0, shutdown_pid = 0, dead_pid = 0;
    pid_t respawn_pid[argc];
    int i;

    if (getpid () != 1)
    {
        return 1;
    }

    signal_action.sa_handler = &signal_handler;
    signal_action.sa_flags = SA_RESTART;
    sigfillset (&signal_action.sa_mask);

    sigaction (SIGTERM, &signal_action, 0);
    sigaction (SIGUSR1, &signal_action, 0);
    sigaction (SIGUSR2, &signal_action, 0);

    init_pid = spawn_command (parse_command ("initsystem-initscript.sh"));

    if (init_pid > 0)
    {
        for (i = 0; i < 60; i++)
        {
            if (init_pid == waitpid (init_pid, 0, WNOHANG))
            {
                break;
            }

            sleep (1);
        }
    }

    memset (respawn_pid, 0, sizeof (respawn_pid));

    for (;;)
    {
        for (i = 1; i < argc; i++)
        {
            if (respawn_pid[i] == 0 || respawn_pid[i] == dead_pid)
            {
                if (signal_number == 0)
                {
                    /* normal processing: respawn commands */
                    respawn_pid[i] = spawn_command (parse_command (argv[i]));
                }
                else
                {
                    /* termination ongoing: don't respawn commands */
                    respawn_pid[i] = 0;
                }
            }
        }

        if (signal_number != 0)
        {
            if (shutdown_pid == 0)
            {
                switch (signal_number)
                {
                    case SIGUSR1:
                        shutdown_pid = spawn_command (parse_command ("initsystem-reboot.sh"));
                        break;

                    case SIGUSR2:
                        shutdown_pid = spawn_command (parse_command ("initsystem-halt.sh"));
                        break;

                    default:
                        shutdown_pid = spawn_command (parse_command ("initsystem-poweroff.sh"));
                        break;
                }
            }

            /* termination failed: back to normal processing */
            if (shutdown_pid == 0 || shutdown_pid == dead_pid)
            {
                shutdown_pid = 0;
                signal_number = 0;
            }
        }

        dead_pid = waitpid (-1, 0, WNOHANG);

        if (dead_pid <= 0)
        {
            dead_pid = -1;
            sleep (1);
        }
    }

    return 1;
}
