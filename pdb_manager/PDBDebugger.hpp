#pragma once

#include <string>
#include <list>
#include <vector>

namespace pdb
{
    // Forward declaration, see full declaration in PDB.hpp
    class PDBProcess;

    class PDBDebugger
    {
    public:
        using pdb_br = std::list<std::pair<std::string, std::vector<int>>>;

        virtual std::string getOptions() = 0;
        virtual std::string getExecutable() = 0;
        virtual pdb_br getBreakpoints(PDBProcess &) = 0;
        virtual std::string getSource(PDBProcess &) = 0;
        virtual std::vector<std::string> getSourceFiles(PDBProcess &) = 0;
        virtual std::string submitComm(PDBProcess &, std::string com) = 0;
    };

    // GNU gdb interface
    class GDBDebugger : public PDBDebugger
    {
    private:
        using PDBDebugger::pdb_br;
        const std::string exec_name;
        const std::string exec_opts;

        pdb_br breakpoints;

    public:
        // By default, gdb will launch with Machine Interface enabled
        GDBDebugger(std::string name = "/usr/bin/gdb", std::string opts = "") 
            : exec_name(name), exec_opts(opts + " -q --interpreter=mi2") {};

        std::string getOptions() { return exec_opts; };
        std::string getExecutable() { return exec_name; };
 
        virtual pdb_br getBreakpoints(PDBProcess &) { return breakpoints; };
        virtual std::string getSource(PDBProcess &) { return "source"; };
        virtual std::vector<std::string> getSourceFiles(PDBProcess &);
        virtual std::string submitComm(PDBProcess &, std::string comm) { return comm; };
    };

    // LLVM lldb interface
    class LLDBDebugger : public PDBDebugger
    {
    private:
        using PDBDebugger::pdb_br;
        const std::string exec_name;
        const std::string exec_opts;

        pdb_br breakpoints;

    public:
        LLDBDebugger(std::string name = "/usr/bin/lldb", std::string opts = "") 
            : exec_name(name), exec_opts(opts) {};
        
        std::string getOptions() { return exec_opts; };
        std::string getExecutable() { return exec_name; };

        virtual pdb_br getBreakpoints(PDBProcess &) { return breakpoints; };
        virtual std::string getSource(PDBProcess &) { return "source"; };
        virtual std::vector<std::string> getSourceFiles(PDBProcess &) 
            { return std::vector<std::string>(1, "sourceFiles"); };
        virtual std::string submitComm(PDBProcess &, std::string comm) { return comm; };
    };
}
