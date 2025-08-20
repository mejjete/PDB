#pragma once

#include <PDBProcess.hpp>
#include <boost/leaf.hpp>
#include <list>
#include <string>
#include <vector>

namespace pdb {
class PDBDebugger : public PDBProcess {
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

  virtual std::string getOptions() const { return exec_opts; };
  virtual PDBbr_list getBreakpointList() = 0;

  /**
   * @brief Starts the execution of a debugger just by commiting "run"
   * @param args - additional arguments being passed to a debugger during
   * runtime
   * @return On error, throws std::runtime_error
   */
  virtual boost::leaf::result<void> startDebug(std::string args) = 0;
  virtual boost::leaf::result<void> endDebug() = 0;
  virtual boost::leaf::result<void> setBreakpoint(PDBbr brpoint) = 0;

  virtual std::vector<std::string> readInput() = 0;

  virtual boost::leaf::result<void>
  checkInput(const std::vector<std::string> &) const = 0;
};

// GNU gdb interface
class GDBDebugger : public PDBDebugger {
private:
  /**
   * GDB/MI has special meaning of "(gdb) " literal. The sequence of output
   * records is terminated by "(gdb) ". It can be used as a separator for
   * internal parser.
   */
  static std::string term;

  // Leading \n is essential for gdb, it indicates end of input
  std::string makeCommand(std::string comm) { return comm += "\n"; };
  std::vector<std::string> stringifyInput(std::string);

public:
  // By default, gdb will launch with Machine Interface enabled
  GDBDebugger(std::string name, std::string opts = "")
      : PDBDebugger(name, opts + " -q --interpreter=mi2") {};
  virtual ~GDBDebugger() {};

  std::string getOptions() const { return exec_opts; };
  virtual boost::leaf::result<void> setBreakpoint(PDBbr);
  virtual PDBbr_list getBreakpointList() { return breakpoints; };
  virtual boost::leaf::result<void> startDebug(std::string) { return {}; };
  virtual boost::leaf::result<void> endDebug();

  virtual std::vector<std::string> readInput();

  virtual boost::leaf::result<void>
  checkInput(const std::vector<std::string> &) const;
};

// LLVM lldb interface
// class LLDBDebugger : public PDBDebugger {
// private:
//   /**
//    * Terminal for detecting command end, defined in LLDBDebugger.cpp
//    */
//   static std::string term;

//   // Leading \n is essential for lldb, it indicates end of input
//   std::string makeCommand(std::string comm) { return comm += "\n"; };
//   std::vector<std::string> stringifyInput(std::string);

// public:
//   using PDBDebugger::PDBbr_list;
//   LLDBDebugger(std::string name, std::string opts = "")
//       : PDBDebugger(name, opts) {};
//   virtual ~LLDBDebugger() {};

//   std::string getOptions() const { return exec_opts; };
//   virtual boost::leaf::result<void> setBreakpoint(PDBbr);
//   virtual PDBbr_list getBreakpointList() { return breakpoints; };
//   virtual boost::leaf::result<void> startDebug(std::string) override;
//   virtual boost::leaf::result<void> endDebug() override;

//   virtual std::vector<std::string> readInput();

//   virtual boost::leaf::result<void>
//   checkInput(const std::vector<std::string> &) const;
// };
} // namespace pdb
