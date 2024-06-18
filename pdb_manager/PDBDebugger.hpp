#pragma once

#include <string>
#include <utility>
#include <vector>

namespace pdb
{
    class PDBDebugger
    {
    protected: 
        using pdb_br = std::vector<std::pair<std::string, std::vector<int>>>;

        std::string exec_name;
        std::string exec_opts;

        pdb_br breakpoints; 
    public:
        /**
         *  Initializes new debugger
         * 
         *  @param name - debugger executable name
         *  @param opts - debugger specific options
         */
        PDBDebugger(std::string name, std::string opts) 
            : exec_name(name), exec_opts(opts) {};

        virtual pdb_br getBreakpoints() = 0;
        virtual std::string getSource() = 0;
        virtual std::vector<std::string> getSourceFiles() = 0;
        virtual std::string submitComm(std::string com) = 0;
    };

    // GNU gdb interface
    class GDBDebugger : public PDBDebugger
    {
    public:
        GDBDebugger(std::string name = "/usr/bin/gdb", std::string opts = "") 
            : PDBDebugger(name, opts) {};

        virtual pdb_br getBreakpoints() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles();
        virtual std::string submitComm(std::string comm) { return comm; };
    };

    // LLVM lldb interface
    class LLDBDebugger : public PDBDebugger
    {
    private:
    using PDBDebugger::pdb_br;

    public:
        LLDBDebugger(std::string name = "/usr/bin/lldb", std::string opts = "") 
            : PDBDebugger(name, opts) {};
        
        virtual pdb_br getBreakpoints() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles() { return std::vector<std::string>(1, "sourceFiles"); };
        virtual std::string submitComm(std::string comm) { return comm; };
    };
}

