#pragma once

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>
#include <array>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <system_error>
#include <vector>
#include <algorithm>
#include <ranges>
#include <wait.h>
#include <poll.h>

#define PDB_PIPE_LENGTH 20

/**
 *  Process handler that creates connections to spawned processes.
 *  Does not spawn any process by itself.
*/
class PDBProcess
{
private:
    int fd_read;
    int fd_write;

    // File names for named pipes
    std::string fd_read_name;
    std::string fd_write_name;

public:
    PDBProcess();

    PDBProcess(const PDBProcess&) = delete;
    PDBProcess(PDBProcess&&) = default;
    ~PDBProcess();

    int pollRead() const;

    std::pair<std::string, std::string> getPipeNames() const { return std::make_pair(fd_read_name, fd_write_name); };
    std::pair<int, int> getPipe() const { return std::make_pair(fd_read, fd_write); };
};


/**
 *  Main Debug instance which communicates with UI
*/
class PDBDebug
{
private:
    int temporal_file;  // File used to pass arguments through PDB runtime
    pid_t exec_pid;     // Executable PID process

    // Open connections to all sub-child processes
    std::vector<PDBProcess> pdb_proc;

    // Parse the input string args into tokens separated by delim
    std::vector<std::string> parseArgs(std::string args, std::string delim);

public:
    /**
     *  Opens a connection to a spawned processes via PDBRuntime.
     * 
     *  @param start_routine - should specify the compiler instance and target-specific flags
     *  [example] : "mpirun -oversubscribe -np 4"
     *  
     *  @param exec - should specify the executable
     *  @param args - should contain a list of arguments to be passed to executable
     *  
     *  PDBDebug("mpirun -np 4 -oversubscribe", "./mpi_test.out", "arg1, arg2, arg3");
     */  
    PDBDebug(std::string start_rountine, std::string exec, std::string args);
    ~PDBDebug();

    /**
     *  Checks if process proc has something to read.
     *  Returns number of bytes ready to read for process proc
    */
    int poll(int proc);
    size_t size() const { return pdb_proc.size(); };
    PDBProcess& getProc(int proc) { return pdb_proc.at(proc); };

    // Blocking read call from process proc
    std::string readProc(int proc);

    // Blocking write call to process proc with message msg
    std::string writeProc(int proc, std::string msg);
};
