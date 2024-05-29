#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "PDBDebug.hpp"

bool is_signaled = false;

void signal_callback_handler(int signum) 
{
    if(signum == SIGINT)
        is_signaled = true;
}

int main()
{
    signal(SIGINT, signal_callback_handler);

    PDBProcess proc_1;
    PDBProcess proc_2;

    while(is_signaled == false)
    {
        // Do some manipulations with gdb output
        // Read and write to pipes
    }

    proc_1.setPid(0);
    proc_2.setPid(0);

    return 0;
}