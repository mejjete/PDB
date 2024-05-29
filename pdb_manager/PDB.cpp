#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "PDBDebug.hpp"

#define PROC_NUM 4
bool is_signaled = false;

void signal_callback_handler(int signum) 
{
    if(signum == SIGINT)
        is_signaled = true;
}

int main()
{
    signal(SIGINT, signal_callback_handler);

    int pipe_fd[PROC_NUM][2];
    pid_t pid_ids[PROC_NUM];

    for(int i = 0; i < PROC_NUM; i++)
    {
        if(pipe(pipe_fd[i]) < -1)
            throw std::runtime_error("Pipe error: " + std::string(strerror(errno)));
    }

    for(int i = 0; i < PROC_NUM; i++)
    {
        int *private_pipe = pipe_fd[i];

        pid_t pid = fork();
        pid_ids[i] = pid;
        if(pid == 0)
        {
            // Redirect child process's stdin to our write end pipe
            dup2(private_pipe[1], STDIN_FILENO);

            // Redirect child process's stdout to our read end pipe
            dup2(private_pipe[0], STDOUT_FILENO);

            // Start gdb as a child process
            if(execlp("gdb", "gdb", "-q", "./a.out", NULL) < 0)
                throw std::runtime_error("Execlp error: " + std::string(strerror(errno)));
        }
        else 
            printf("Process %d spawned\n", pid);
    }

    while(is_signaled == false)
    {
        // Do some manipulations with gdb output
        // Read and write to pipes
    }

    if(is_signaled)
    {
        for(int i = 0; i < PROC_NUM; i++)
        {
            kill(pid_ids[i], SIGKILL);

            // Close pipe
            close(pipe_fd[i][0]);
            close(pipe_fd[i][1]);
        }
    }

    return 0;
}