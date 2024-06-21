#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <vector>
#include <memory>
#include <wait.h>
#include <poll.h>
#include "PDBDebugger.hpp"
#include "PDBProcess.hpp"

namespace pdb
{
    #define PDB_PIPE_LENGTH 20

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

        // PDBDebugger handles all requests to and from actual debugger
        std::unique_ptr<PDBDebugger> debugger;

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
        PDBDebug(std::string start_rountine, std::string exec, std::string args, std::unique_ptr<PDBDebugger> debug);
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
}
