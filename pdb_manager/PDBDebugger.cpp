#include "PDBDebugger.hpp"
#include <stdexcept>
#include <cstring>
#include <string>
#include <vector>

namespace pdb
{
    std::string GDBDebugger::term = "(gdb) ";

    std::string GDBDebugger::readInput()
    {
        std::string main_buffer;

        while(true)
        {
            std::string buff = read();
            main_buffer += buff;

            if(buff.find(term) != std::string::npos)
                break;
        }

        return main_buffer;
    }

    void GDBDebugger::checkInput(std::string input)
    {
        if(input.find("No debugging symbols found") != std::string::npos)
            throw std::runtime_error("PDB: No debugging symbols found");
    }

    std::vector<std::string> GDBDebugger::getSourceFiles()
    {
        std::string command = makeCommand("info sources");
        write(command);
        
        std::string result = readInput();
        checkInput(result);

        // Find first occurence of ^done
        auto done_expr = result.find("^done");
        if(done_expr == std::string::npos)
            throw std::runtime_error("PDB: error parsing <info sources>");
        
        auto source_end = done_expr - 2;
        auto source_start = result.rfind("\"", done_expr - 3) + 1;

        std::string source_list(result.begin() + source_start, result.begin() + source_end);
        std::vector<std::string> sources;

        char *list = new char[source_list.length() + 1];
        memcpy(list, source_list.c_str(), source_list.length());
        list[source_list.length()] = 0;

        char *token = strtok(list, " ,");
        if(token == NULL)
            throw std::runtime_error("PDB: error parsing <info sources>");

        do
        {
            sources.push_back(token);
        } while ((token = strtok(NULL, " ,")));
        delete[] list;

        // gdb usually adds 2 additional \n at the end of the last source file
        sources[sources.size() - 1].resize(sources[sources.size() - 1].length() - 4);
        return sources;
    }
}
