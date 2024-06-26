#include <unistd.h>
#include <vector>
#include <system_error>
#include "PDB.hpp"
#include "PDBDebugger.hpp"

namespace pdb
{
    PDBDebug::~PDBDebug()
    {
        if(temporal_file >= 0)
            close(temporal_file);
        
        if(temporal_file_name.length() > 0)
            unlink(temporal_file_name.c_str());
        
        if(exec_pid > 0)
        {   
            // Second chance to terminate process
            int statlock;
            pid_t pid = waitpid(exec_pid, &statlock, WNOHANG);
            if(pid == 0)
            {
                printf("PDB: Killing main process\n");
                kill(exec_pid, SIGKILL);
            }
        }
    }

    std::pair<int, int> PDBDebug::join()
    {
        for(auto &iter : pdb_proc)
            iter->endDebug();
        
        // Try to wait a while to let the processes terminate
        usleep(10000);

        int statlock;
        pid_t pid = waitpid(exec_pid, &statlock, WNOHANG);
        if(pid < 0)
        {
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error waiting for main process");
        }
        else if(pid == 0)
            return std::make_pair(0, 0); // If process has not terminated yet, do nothing, destructor will kill this process
        else
        {
            exec_pid = 0;

            if (WIFEXITED(statlock))
                return std::make_pair(1, WEXITSTATUS(statlock));
            else if (WIFSIGNALED(statlock))
                return std::make_pair(1, WTERMSIG(statlock));
            else if (WIFSTOPPED(statlock))
                return std::make_pair(1, WSTOPSIG(statlock));
            else
            {
                exec_pid = pid;
                return std::make_pair(0, 0);
            }
        }

        return std::make_pair(0, 0);
    }

    std::vector<std::string> PDBDebug::parseArgs(std::string pdb_args, std::string delim)
    {
        char *args = new char[pdb_args.length()];
        memcpy(args, pdb_args.c_str(), pdb_args.length());
        args[pdb_args.length()] = 0;
        std::vector<std::string> pdb_args_parced;

        char *token = strtok(args, delim.c_str());
        if(token == NULL)
            return pdb_args_parced;

        do
        {
            pdb_args_parced.push_back(token);
        } while ((token = strtok(NULL, delim.c_str())));

        return pdb_args_parced;
    }

    std::vector<std::string> PDBDebug::getSourceFiles()
    {
        if(pdb_proc.size() == 0)
            throw std::runtime_error("PDB: Invalid number of processes: ");
        
        auto &ptr = pdb_proc[0];
        return ptr->getSourceFiles();
    }

    std::pair<int, std::string> PDBDebug::getFunction(std::string func_name)
    {
        if(pdb_proc.size() == 0)
            throw std::runtime_error("PDB: Invalid number of processes: ");
        
        auto &ptr = pdb_proc[0];
        return ptr->getFunction(func_name);
    }
}
