#include "PDBDebug.hpp"

PDBProcess::PDBProcess()
{
    int pipe_fd[2];

    if(pipe(pipe_fd) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening pipe: " + std::string(strerror(errno)));
    
    read_pipe = pipe_fd[0];
    write_pipe = pipe_fd[1];

    pid = fork();
    if(pid == 0)
    {
        // Redirect child process's stdin to our write end pipe
        if(dup2(write_pipe, STDIN_FILENO) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "dup2 error: could not substituted STDIN_FILENO with opened pipe");

        // Redirect child process's stdout to our read end pipe
        if(dup2(read_pipe, STDOUT_FILENO) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "dup2 error: could not substituted STDOUT_FILENO with opened pipe");

        // Start gdb as a child process
        if(execlp("gdb", "gdb", "-q", NULL) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "execlp error: " + std::string(strerror(errno)));
    }
    else if(pid < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
                "fork error: " + std::string(strerror(errno)));
}

PDBProcess::~PDBProcess()
{
    int wstatus;
    if(pid != 0)
        waitpid(pid, &wstatus, 0);
    else
        sendSignal(SIGKILL);

    close(read_pipe);
    close(write_pipe);
}

void PDBProcess::sendSignal(int signum)
{
    switch (signum)
    {
    case SIGINT:
        break;
    
    case SIGTERM:
        break;

    case SIGKILL:
        break;
    default:
        throw std::system_error(std::error_code(errno, std::generic_category()), 
                "PDB does not support signal with code: " + std::to_string(signum));
    }

    if(kill(pid, signum) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error sending sidnal " + std::to_string(signum) + " to a process " + std::to_string(pid) + "(pid)\n");
}

