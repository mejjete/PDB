#include "PDB.hpp"

PDBDebug::PDBDebug(std::string compile_args, std::string pdb_args)
{
    std::vector<std::string> pdb_args_parced;

    // Tokenize argument string
    pdb_args_parced = parseArgs(pdb_args, " ;");

    // Fetch process count from command-line argument string
    int proc_count;
    auto iter = std::ranges::find(pdb_args_parced, "-np");

    if(iter == pdb_args_parced.end())
    {
        iter = std::ranges::find(pdb_args_parced, "-n");
        if(iter == pdb_args_parced.end())
            throw std::runtime_error("Missing -np/-n options\n");
    }
    
    iter = std::next(iter);
    if(iter == pdb_args_parced.end())
        throw std::runtime_error("Number of processes not provided\n");

    proc_count = std::atoi(iter->c_str());
    
    if(proc_count <= 0)
        throw std::runtime_error("Invalid number of MPI processes\n");

    // Create specified number of process handlers
    std::vector<std::string> proc_name_files;
    
    for(int i = 0; i < proc_count; i++)
    {
        pdb_proc.push_back(PDBProcess());
        auto proc_filenames = pdb_proc[i].getPipeNames();

        // Memorize pipe names
        proc_name_files.push_back(proc_filenames.first);
        proc_name_files.push_back(proc_filenames.second);
    }
    
    /**
     *  Modify command-line argument string and add extra arguments
     *  
     *  To spawn any process, execvp is used. It accepts char[] as an argument vector.
     *  The following code adds a few extra arguments which will accommodate names of
     *  named pipes for each process.
    */ 
    int old_argc = pdb_args_parced.size();
    int total_argc = old_argc + proc_count * 2;
    char **new_argv = new char*[total_argc + 1];
    
    // Copy old arguments
    int i;
    for(i = 0; i < old_argc; i++)
    {
        size_t len = pdb_args_parced[i].length();
        new_argv[i] = new char[len];
        memcpy(new_argv[i], pdb_args_parced[i].c_str(), len);
        new_argv[i][len] = 0;
    }

    // Copy new arguments
    for(int k = 0; i < total_argc; i++, k++)
    {
        size_t len = proc_name_files[k].length();
        new_argv[i] = new char[len];
        memcpy(new_argv[i], proc_name_files[k].c_str(), len);
        new_argv[i][len] = 0;
    }

    // Terminating NULL string for exec function
    new_argv[total_argc] = NULL;

    // Spawn process
    exec_pid = fork();
    if(exec_pid == 0)
    {
        if(execvp(pdb_args_parced[0].c_str(), new_argv) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "execvp error: " + std::string(strerror(errno)));
    }

    // After exec, we can finally free argument string
    for(int i = 0; i < total_argc; i++)
        delete[] new_argv[i];

    delete[] new_argv;
}

PDBDebug::~PDBDebug()
{
    int statlock;
    wait(&statlock);
    // waitpid(comp_pid, &statlock, WUNTRACED | WCONTINUED);
}

std::vector<std::string> PDBDebug::parseArgs(std::string pdb_args, std::string delim)
{
    char *args = new char[pdb_args.length()];
    memcpy(args, pdb_args.c_str(), pdb_args.length());
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

int PDBDebug::poll(int proc)
{
    if(proc >= static_cast<int>(pdb_proc.size()))
        throw std::runtime_error("PDB: Invalid process identifier\n");
    
    PDBProcess &proc_handler = pdb_proc[proc];
    return proc_handler.pollRead();
}
