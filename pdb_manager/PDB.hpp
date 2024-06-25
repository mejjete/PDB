#pragma once

#include <cstdio>
#include <cstring>
#include <stdexcept>
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
#include <ranges>
#include "PDBDebugger.hpp"

namespace pdb
{
    #define PDB_PIPE_LENGTH 20

    enum class PDB_Debug_type
    {
        GDB, 
        LLDB
    };

    template <typename ...Args>
    std::unique_ptr<PDBDebugger> createDebugger(PDB_Debug_type type, Args... args)
    {
        if(type == PDB_Debug_type::GDB)
            return std::unique_ptr<PDBDebugger>(new GDBDebugger(args...));
        else if(type == PDB_Debug_type::LLDB)
            return std::unique_ptr<PDBDebugger>(new LLDBDebugger(args...));
            
        throw std::runtime_error("PDB does not support specified debugger");
    }

    /**
     *  Main Debug instance which communicates with UI
    */
    class PDBDebug
    {
    private:
        int temporal_file;  // File used to pass arguments through PDB runtime
        std::string temporal_file_name;
        pid_t exec_pid;     // Executable PID process

        // Debugger instances associated with each debugging process
        std::vector<std::unique_ptr<PDBDebugger>> pdb_proc;

        // Parse the input string args into tokens separated by delim
        std::vector<std::string> parseArgs(std::string args, std::string delim);

    public:
        PDBDebug() : temporal_file(0), exec_pid(0) {};
        ~PDBDebug();

        std::vector<std::string> getSourceFiles();
        std::pair<int, std::string> getFunction(std::string);

        template <typename ...DebuggerArgs>
        void launch(std::string start_rountine, std::string exec, std::string args, 
            PDB_Debug_type type, DebuggerArgs... dargs);

        /**
         *  Must be called to properly terminate calling process. Returns pair of values describing 
         *  main process exit status.
         *  .first - holds 1 if child has terminated and 0 otherwise
         *  .second - holds exit code of a child
         * 
         *  Because this function uses wait() in a non-blocking mode, it can be used by high-level 
         *  routines to wait until process termination or just kill it by calling destructor
         */
        std::pair<int, int> join();
        size_t size() const { return pdb_proc.size(); };
    };


    /**
     *  Opens a connection to a spawned processes via PDBRuntime.
     * 
     *  @param start_routine - should specify the compiler instance and target-specific flags
     *  [example] : "mpirun -oversubscribe -np 4"
     *  
     *  @param exec - should specify the executable
     *  @param args - should contain a list of arguments to be passed to executable
     *  @param type - external debugger type, should be member of PDB_Debug_type
     *  @param dargs - arguments to be passed to an external debugger
     *  
     *  PDBDebug("mpirun -np 4 -oversubscribe", "./mpi_test.out", "arg1, arg2, arg3", PDB_Debug_type::GDB, 
     *      [following arguments being passed to a specified debugger type during construction]);
     */  
    template <typename ...DebuggerArgs>
    void PDBDebug::launch(std::string start_rountine, std::string exec, std::string args, 
        PDB_Debug_type type, DebuggerArgs... dargs)
    {
        std::vector<std::string> pdb_args_parced;
        std::vector<std::string> pdb_routine_parced;
        std::vector<std::string> pdb_debugger_parced;

        // Temporary object
        auto debug = createDebugger(type, dargs...);

        // Tokenize command-line arguments
        pdb_args_parced = parseArgs(args, " ;\n\r");
        pdb_routine_parced = parseArgs(start_rountine, " ;\n\r");
        pdb_debugger_parced = parseArgs(debug->getOptions(), " ;\n\r");

        // Fetch process count from command-line argument string
        int proc_count;
        auto iter = std::ranges::find(pdb_routine_parced, "-np");

        if(iter == pdb_routine_parced.end())
        {
            iter = std::ranges::find(pdb_routine_parced, "-n");
            if(iter == pdb_routine_parced.end())
                throw std::runtime_error("Missing -np/-n options\n");
        }
        
        iter = std::next(iter);
        if(iter == pdb_args_parced.end())
            throw std::runtime_error("Number of processes not provided\n");

        proc_count = std::atoi(iter->c_str());
        
        if(proc_count <= 0)
            throw std::runtime_error("Invalid number of MPI processes\n");

        // Create temporal file to pass name of pipes
        char temp_file[] = "/tmp/pdbpipeXXXXXXX";
        temporal_file = mkstemp(temp_file);
        if(temporal_file < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error opening temporal file: ");

        close(temporal_file);
        unlink(temp_file);

        temporal_file = open(temp_file, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if(temporal_file < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error opening temporal file: ");
        temporal_file_name = temp_file;

        // Create specified number of process handlers
        std::vector<std::string> proc_name_files;
        pdb_proc.reserve(proc_count);
        
        for(int i = 0; i < proc_count; i++)
        {
            pdb_proc.emplace_back(createDebugger(type, dargs...));
            auto proc_filenames = pdb_proc[i]->getPipeNames();

            // Memorize pipe names
            proc_name_files.push_back(proc_filenames.first);
            proc_name_files.push_back(proc_filenames.second);

            write(temporal_file, proc_filenames.first.c_str(), PDB_PIPE_LENGTH);
            write(temporal_file, proc_filenames.second.c_str(), PDB_PIPE_LENGTH);
        }
        
        /**
        *  Modify command-line argument string and add extra arguments
        *  
        *  To spawn any process, execvp is used. It accepts char[] as an argument vector.
        *  The following code adds a few extra arguments. This behavior is implementation-defined
        *  and must synchronize in the receving process 
        */ 
        int old_argc = pdb_routine_parced.size() + pdb_debugger_parced.size() + 2;
        int total_argc = old_argc + 6;
        char **new_argv = new char*[total_argc];
        int new_arg_size = 0;

        // Step 1: copy routine call
        for(auto &token : pdb_routine_parced)
        {
            new_argv[new_arg_size] = new char[token.length() + 1];
            memcpy(new_argv[new_arg_size], token.c_str(), token.length());
            new_argv[new_arg_size][token.length()] = 0;
            new_arg_size++;
        }

        // Step 2: copy PDB launch application to prepare PDB runtime
        char pdb_launch[] = "./pdb_launch";
        new_argv[new_arg_size] = new char[strlen(pdb_launch) + 1];
        memcpy(new_argv[new_arg_size], pdb_launch, sizeof(pdb_launch));
        new_argv[new_arg_size][strlen(pdb_launch)] = 0;
        new_arg_size++;

        // Step 3: copy debugger executable and arguments
        std::string debug_name = debug->getExecutable();
        new_argv[new_arg_size] = new char[debug_name.length() + 1];
        memcpy(new_argv[new_arg_size], debug_name.c_str(), debug_name.length());
        new_argv[new_arg_size][debug_name.length()] = 0;
        new_arg_size++;

        for(auto &token : pdb_debugger_parced)
        {
            new_argv[new_arg_size] = new char[token.length() + 1];
            memcpy(new_argv[new_arg_size], token.c_str(), token.length());
            new_argv[new_arg_size][token.length()] = 0;
            new_arg_size++;
        }

        // Add executable
        new_argv[new_arg_size] = new char[exec.length() + 1];
        memcpy(new_argv[new_arg_size], exec.c_str(), exec.length());
        new_argv[new_arg_size][exec.length()] = 0;
        new_arg_size++;

        // Step 4: add extra arguments, namely temporal file and process number
        new_argv[new_arg_size] = new char[strlen(temp_file) + 1];
        memcpy(new_argv[new_arg_size], temp_file, strlen(temp_file));
        new_argv[new_arg_size][strlen(temp_file)] = 0;
        new_arg_size++;

        std::string proc_count_char = std::to_string(proc_count);
        new_argv[new_arg_size] = new char[proc_count_char.length() + 1];
        memcpy(new_argv[new_arg_size], proc_count_char.c_str(), proc_count_char.length());
        new_argv[new_arg_size][proc_count_char.length()] = 0;
        new_arg_size++;

        // Terminating NULL string for exec function
        new_argv[new_arg_size] = NULL;

        // Spawn process
        exec_pid = fork();
        if(exec_pid == 0)
        {
            std::string first_exec = pdb_routine_parced[0]; 
            first_exec.push_back(0);

            if(execvp(first_exec.c_str(), new_argv) < 0)
                throw std::system_error(std::error_code(errno, std::generic_category()), 
                    "execvp error:");
        }

        // After exec, we can finally free argument string
        for(int i = 0; i < new_arg_size; i++)
            delete[] new_argv[i];

        delete[] new_argv;

        /**
        *  At this point, child process which is now PDB launch will try to open FIFO 
        *  and block because FIFO is blocked until it is dual-opened. 
        *  The following open() calls should be synchronized with the same open() in a child.
        */
        for(auto &proc : pdb_proc)
        {
            if(proc->openFIFO() < 0)
                throw std::system_error(std::error_code(errno, std::generic_category()), 
                    "error opening FIFO");
        }

        // Make sure that each process has started successfully by checking its output
        while(true)
        {
            bool flag = true;

            for(auto &proc : pdb_proc)
            {
                int nbytes = proc->pollRead();
                if(nbytes <= 0) 
                    flag = false;
            }

            if(flag = true)
                break;
        }

        // Read out initial print from gdb to clear input for subsequent commands
        for(auto &proc : pdb_proc)
            proc->checkInput(proc->readInput());
    }
}
