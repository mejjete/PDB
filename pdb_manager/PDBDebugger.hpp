#pragma once

#include <string>
#include <list>
#include <vector>
#include "PDBProcess.hpp"

namespace pdb
{
    class PDBDebugger : public PDBProcess
    {
    public:
        using PDBbr_list = std::vector<std::pair<std::string, std::vector<int>>>;
        using PDBbr = std::pair<int, std::string>;

    protected:
        const std::string exec_name;
        const std::string exec_opts;

        PDBbr_list breakpoints;

    public:
        PDBDebugger(std::string name, std::string opts) 
            : exec_name(name), exec_opts(opts) {};
        PDBDebugger(const PDBDebugger &) = delete;
        PDBDebugger(PDBDebugger &&) = default;
        virtual ~PDBDebugger() {};

        virtual std::vector<std::string> readInput() = 0;
        virtual void checkInput(const std::vector<std::string>&) const = 0;
        virtual std::string getOptions() const = 0;
        virtual std::string getExecutable() const = 0;
        virtual PDBbr_list getBreakpointList() = 0;


        /**
         *  @param args - additional arguments being passed to a debugger during runtime
         *  
         *  Starts the execution of a debugger just by commiting "run"
         *  On error, throws std::runtime_error
         */
        virtual void startDebug(std::string args) = 0; 


        /**
         *  Ends debugging session just by commiting "quit"
         */
        virtual void endDebug() = 0;


        /**
         *  On success, returns std::string containing full path to a source file.
         *  On error, throws std::runtime_error 
         * 
         *  Depending on the active context in a debugger, getSource() should return the 
         *  full path to a source file that is being debugged right now.
         *  
         *  Should only be called during active debugging process (after startDebug() call)
         */
        virtual std::string getSource() = 0;
        
        
        /**
         *  On success, returns vector of strings, each containing full path
         *  to every source file recorded in executable.
         * 
         *  On error, throws std::runtime_error
         */
        virtual std::vector<std::string> getSourceFiles() = 0;


        /**
         *  @param func_name - name of the function in question
         *  
         *  On success, returns pair describing function information
         *  first - location of a function in a source file (line)
         *  second - full path of a source file of a given function
         */
        virtual std::pair<int, std::string> getFunction(std::string func_name) = 0;


        /**
         *  @param brpoint - breakpoint to be set
         *  
         *  Sets breakpoint in executable described by PDBbr object.
         */
        virtual void setBreakpoint(PDBbr brpoint) = 0;
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

        std::string getOptions()    const { return exec_opts; };
        std::string getExecutable() const { return exec_name; };
 
        virtual std::vector<std::string> readInput();
        virtual void checkInput(const std::vector<std::string>&) const;

        virtual void setBreakpoint(PDBbr);
        virtual PDBbr_list getBreakpointList() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles();
        virtual std::pair<int, std::string> getFunction(std::string);
        virtual void startDebug(std::string) {};
        virtual void endDebug();
    };

    // LLVM lldb interface
    class LLDBDebugger : public PDBDebugger
    {
    private:
        using PDBDebugger::PDBbr_list;

        PDBProcess proc;
        const std::string exec_name;
        const std::string exec_opts;
        PDBbr_list breakpoints;

    public:
        LLDBDebugger(std::string name = "/usr/bin/lldb", std::string opts = "") 
            : PDBDebugger(name, opts) {};
        virtual ~LLDBDebugger() {};
        
        std::string getOptions()    const  { return exec_opts; };
        std::string getExecutable() const { return exec_name; };

        virtual std::vector<std::string> readInput() { return std::vector<std::string>(1, ""); };
        virtual void checkInput(const std::vector<std::string>&) const {};

        virtual void setBreakpoint(PDBbr) {};
        virtual PDBbr_list getBreakpointList() { return breakpoints; };
        virtual std::string getSource() { return "source"; };
        virtual std::vector<std::string> getSourceFiles() 
            { return std::vector<std::string>(1, "sourceFiles"); };
        virtual std::pair<int, std::string> getFunction(std::string) { return std::make_pair(0, ""); }; 
        virtual void startDebug(std::string) {};
        virtual void endDebug() {};
    };
}
