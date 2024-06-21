#pragma once

#include <string>
#include <list>
#include <vector>
#include <memory>
#include <system_error>
#include <algorithm>
#include "PDBProcess.hpp"

namespace pdb
{
    class PDBDebugger : public PDBProcess
    {
    protected:
        using pdb_br = std::list<std::pair<std::string, std::vector<int>>>;

        const std::string exec_name;
        const std::string exec_opts;

        pdb_br breakpoints;

    public:
        PDBDebugger(std::string name, std::string opts) 
            : exec_name(name), exec_opts(opts) {};
        PDBDebugger(const PDBDebugger &) = default;
        PDBDebugger(PDBDebugger &&) = default;
        virtual ~PDBDebugger() {};

        virtual std::string getOptions() const = 0;
        virtual std::string getExecutable() const = 0;
        virtual pdb_br getBreakpoints() = 0;
        virtual std::string getSource() = 0;
        virtual std::vector<std::string> getSourceFiles() = 0;
    };

    // GNU gdb interface
    class GDBDebugger : public PDBDebugger
    {
    public:
        // By default, gdb will launch with Machine Interface enabled
        GDBDebugger(std::string name = "/usr/bin/gdb", std::string opts = "") 
            : PDBDebugger(name, opts + " -q --interpreter=mi2") {};
        virtual ~GDBDebugger() {};

        std::string getOptions() const { return exec_opts; };
        std::string getExecutable() const { return exec_name; };
 
        virtual pdb_br getBreakpoints() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles();
    };

    // LLVM lldb interface
    class LLDBDebugger : public PDBDebugger
    {
    private:
        using PDBDebugger::pdb_br;

        PDBProcess proc;
        const std::string exec_name;
        const std::string exec_opts;
        pdb_br breakpoints;

    public:
        LLDBDebugger(std::string name = "/usr/bin/lldb", std::string opts = "") 
            : PDBDebugger(name, opts) {};
        virtual ~LLDBDebugger() {};
        
        std::string getOptions() const { return exec_opts; };
        std::string getExecutable() const { return exec_name; };

        virtual pdb_br getBreakpoints() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles() 
            { return std::vector<std::string>(1, "sourceFiles"); };
        virtual std::string submitComm(std::string comm) { return comm; };
    };
}
