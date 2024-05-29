#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>
#include <array>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <vector>

class PDBProcess
{
private:
    int read_pipe;
    int write_pipe;
    pid_t pid;

public:
    PDBProcess();
    PDBProcess(const PDBProcess&) = delete;
    PDBProcess(PDBProcess&&) = default;
    ~PDBProcess();

    void sendSignal(int signum);

    std::pair<int, int> getPipe() const { return std::make_pair(read_pipe, write_pipe); };
    pid_t getPid() const { return pid; };
    void setPid(pid_t p) { pid = p; };
};

class PDBDebug
{
private:
    // Open connection to a master-child process
    PDBProcess master;

    // Open connections to all sub-child processes
    std::vector<PDBProcess> pdb_proc;

public:
    PDBDebug(int proc_num);
    ~PDBDebug();
};