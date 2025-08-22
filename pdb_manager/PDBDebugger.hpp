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
  PDBbr_list breakpoints;

public:
  PDBDebugger() {};
  PDBDebugger(const PDBDebugger &) = delete;
  PDBDebugger(PDBDebugger &&) = default;
  virtual ~PDBDebugger() {};

  virtual PDBbr_list getBreakpointList() = 0;

  /**
   * @brief Starts the execution of a debugger just by commiting "run"
   * @param args - additional arguments being passed to a debugger during
   * runtime
   * @return On error, throws std::runtime_error
   */
  virtual boost::leaf::result<void> startDebug(const std::string &args) = 0;
  virtual boost::leaf::result<void> endDebug() = 0;
  virtual boost::leaf::result<void> setBreakpoint(PDBbr brpoint) = 0;

  virtual std::vector<std::string> readInput() = 0;

  virtual boost::leaf::result<void>
  checkInput(const std::vector<std::string> &) const = 0;

  // Default set of options being passed to a debugger
  static std::string getDefaultOptions() { return ""; };
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
  std::vector<std::string> stringifyInput(const std::string &);

public:
  // By default, gdb will launch with Machine Interface enabled
  GDBDebugger() {};
  virtual ~GDBDebugger() {};

  virtual boost::leaf::result<void> setBreakpoint(PDBbr);
  virtual PDBbr_list getBreakpointList() { return breakpoints; };
  virtual boost::leaf::result<void> startDebug(const std::string &) {
    return {};
  };
  virtual boost::leaf::result<void> endDebug();
  virtual std::vector<std::string> readInput();

  virtual boost::leaf::result<void>
  checkInput(const std::vector<std::string> &) const;

  static std::string getDefaultOptions() { return "-q --interpreter=mi2"; };
};
} // namespace pdb
