#include <unistd.h>
#include <vector>
#include "PDB.hpp"
#include "PDBDebugger.hpp"

namespace pdb
{
    PDBDebug::~PDBDebug()
    {
        int statlock;
        wait(&statlock);
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
}
