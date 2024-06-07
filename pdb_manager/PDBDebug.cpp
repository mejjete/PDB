#include "PDB.hpp"

PDBDebug::PDBDebug(std::string start_rountine, std::string exec, std::string args)
{
    std::vector<std::string> pdb_args_parced;
    std::vector<std::string> pdb_routine_parced;

    // Tokenize command-line arguments
    pdb_args_parced = parseArgs(args, " ;");
    pdb_routine_parced = parseArgs(start_rountine, " ;");

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
            "Error opening temporal file: " + std::string(strerror(errno)));

    // Create specified number of process handlers
    std::vector<std::string> proc_name_files;
    
    for(int i = 0; i < proc_count; i++)
    {
        pdb_proc.push_back(PDBProcess());
        auto proc_filenames = pdb_proc[i].getPipeNames();

        // Memorize pipe names
        proc_name_files.push_back(proc_filenames.first);
        proc_name_files.push_back(proc_filenames.second);

        write(temporal_file, proc_filenames.first.c_str(), proc_filenames.first.length());
        write(temporal_file, proc_filenames.second.c_str(), proc_filenames.second.length());
    }
    
    /**
     *  Modify command-line argument string and add extra arguments
     *  
     *  To spawn any process, execvp is used. It accepts char[] as an argument vector.
     *  The following code adds a few extra arguments. This behavior is implementation-defined
     *  and must synchronize in the receving process 
    */ 
    int old_argc = pdb_routine_parced.size() + pdb_args_parced.size() + 1;
    int total_argc = old_argc + 3;
    char **new_argv = new char*[total_argc];
    int new_arg_size = 0;

    // Step 1: copy routine call
    for(auto &token : pdb_routine_parced)
    {
        new_argv[new_arg_size] = new char[token.length()];
        token.copy(new_argv[new_arg_size], token.length(), 0);
        new_argv[new_arg_size][token.length()] = 0;
        new_arg_size++;
    }

    // Step 2: copy executable name
    // First launch pdb_launch process to prepare PDB runtime
    char pdb_launch[] = "./pdb_launch";
    new_argv[new_arg_size] = new char[strlen(pdb_launch)];
    std::ranges::copy(pdb_launch, new_argv[new_arg_size]);
    new_arg_size++;

    new_argv[new_arg_size] = new char[exec.length()];
    std::ranges::copy(exec, new_argv[new_arg_size]);
    new_arg_size++;

    // Step 3: copy program arguments
    for(auto &token : pdb_args_parced)
    {
        new_argv[new_arg_size] = new char[token.length()];
        token.copy(new_argv[new_arg_size], token.length(), 0);
        new_argv[new_arg_size][token.length()] = 0;
        new_arg_size++;
    }

    // Step 4: add extra arguments, namely temporal file and process number
    new_argv[new_arg_size] = new char[strlen(temp_file)];
    std::ranges::copy(temp_file, new_argv[new_arg_size]);
    new_arg_size++;

    std::string proc_count_char = std::to_string(proc_count);
    new_argv[new_arg_size] = new char[proc_count_char.length()];
    std::ranges::copy(proc_count_char, new_argv[new_arg_size]);

    // Terminating NULL string for exec function
    new_argv[total_argc] = NULL;

    for(int i = 0; i < total_argc; i++)
        std::cout << new_argv[i] << std::endl;

    // Spawn process
    exec_pid = fork();
    if(exec_pid == 0)
    {
        std::string first_exec = pdb_routine_parced[0]; 
        first_exec.push_back(0);

        if(execvp(first_exec.c_str(), new_argv) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "execvp error:" + std::string(strerror(errno)));
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
