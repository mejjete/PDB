#pragma once

#include <string>
#include <list>
#include <vector>
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
        PDBDebugger(const PDBDebugger &) = delete;
        PDBDebugger(PDBDebugger &&) = default;
        virtual ~PDBDebugger() {};

        virtual std::string readInput() = 0;
        virtual void checkInput(std::string) = 0;
        virtual std::string getOptions() const = 0;
        virtual std::string getExecutable() const = 0;
        virtual pdb_br getBreakpoints() = 0;
        virtual std::string getSource() = 0;
        virtual std::vector<std::string> getSourceFiles() = 0;
        virtual std::pair<int, std::string> getFunction(std::string) = 0; 
    };

    // GNU gdb interface
    class GDBDebugger : public PDBDebugger
    {
    private:
        /**
         *  GDB/MI has special meaning of "(gdb) " literal. The sequence of output records is terminated 
         *  by "(gdb) ". It can be used as a separator for internal parser.
         */
        static std::string term;

        // Leading \n is essential for gdb, it indicates end of input
        std::string makeCommand(std::string comm) 
        { return comm += "\n"; };

        std::vector<std::string> stringifyInput(std::string);

    public:
        // By default, gdb will launch with Machine Interface enabled
        GDBDebugger(std::string name = "/usr/bin/gdb", std::string opts = "") 
            : PDBDebugger(name, opts + " -q --interpreter=mi2") {};
        virtual ~GDBDebugger() {};

        std::string getOptions() const { return exec_opts; };
        std::string getExecutable() const { return exec_name; };
 
        virtual std::string readInput();
        virtual void checkInput(std::string);
        virtual pdb_br getBreakpoints() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles();
        virtual std::pair<int, std::string> getFunction(std::string);
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

        virtual std::string readInput() { return "NULL"; };
        virtual void checkInput(std::string) {};
        virtual pdb_br getBreakpoints() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles() 
            { return std::vector<std::string>(1, "sourceFiles"); };
        virtual std::pair<int, std::string> getFunction(std::string) { return std::make_pair(0, ""); }; 
    };
}
