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

class PDBProcess
{
private:
    int read_pipe;
    int write_pipe;
    pid_t pid;

public:
    PDBProcess();
};

class PDBDebug
{
private:
    int pipe[2];
    pid_t pid;

public:
    PDBDebug();
    ~PDBDebug();
};