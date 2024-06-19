/**
 *  Auxiliary helper which set up the PDB runtime for each process 
 */
#include "PDB.hpp"
#include <cstdio>
#include <unistd.h>

std::pair<std::string, std::string> readPipeNames(const char *temp_file, int proc_num)
{
    int fd = open(temp_file, O_RDWR);
    if(fd < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening temporal file: ");

    if(flock(fd, LOCK_EX) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error locking temporal file: ");

    std::vector<char> file(4096);
    std::vector<char> in_pipe(PDB_PIPE_LENGTH);
    std::vector<char> out_pipe(PDB_PIPE_LENGTH);
 
    int nbytes;
    if((nbytes = read(fd, file.data(), 4096)) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error reading from temporal file: ");

    std::copy(file.begin(), file.begin() + PDB_PIPE_LENGTH, in_pipe.begin());
    std::copy(file.begin() + PDB_PIPE_LENGTH, file.begin() + PDB_PIPE_LENGTH * 2, out_pipe.begin());

    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    if(write(fd, file.data() + (PDB_PIPE_LENGTH * 2), nbytes - (PDB_PIPE_LENGTH * 2)) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error writing to temporal file: ");

    flock(fd, LOCK_UN);
    close(fd);

    std::string input_pipe(in_pipe.begin(), in_pipe.end());
    std::string output_pipe(out_pipe.begin(), out_pipe.end());

    return std::make_pair(input_pipe, output_pipe);
}

int main(int argc, char **argv)
{
    // The last 2 parameters in the current scheme would be process specific arguments
    int proc_num = atoi(argv[argc - 1]);
    auto pipes = readPipeNames(argv[argc - 2], proc_num);

    // The following calls to open should be synchronized with calls in PDBDebug constructor
    // For more information, see PDBDebug
    int pipe_out = open(pipes.first.c_str(), O_WRONLY);
    if(pipe_out < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening STDOUT pipe file: ");

    int pipe_in = open(pipes.second.c_str(), O_RDONLY);
    if(pipe_in < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening STDIN pipe file: ");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Redirect standard input to named pipe
    if(dup2(pipe_in, STDIN_FILENO) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error dupping STDIN to " + pipes.second + ": ");

    // Redirect standard output to named pipe
    if(dup2(pipe_out, STDOUT_FILENO) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error dupping STDOUT to " + pipes.first + ": ");
        
    if(dup2(pipe_out, STDERR_FILENO) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error dupping STDERR to " + pipes.first + ": ");

    // Toggle line bufferization
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stdin, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    argc -= 2;
    argv[argc] = NULL;
    if(execvp(argv[1], argv + 1) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "execvp error: ");
    return 0;
}