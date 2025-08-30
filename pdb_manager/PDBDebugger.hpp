#pragma once

#include <PDBProcess.hpp>
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
  bool isRunning;
  std::size_t currentLine;
  std::string currentFile;
  std::string currentFunction;

public:
  PDBDebugger() : isRunning(false) {};
  PDBDebugger(const PDBDebugger &) = delete;
  PDBDebugger(PDBDebugger &&) = default;
  virtual ~PDBDebugger() {};

  virtual PDBbr_list getBreakpointList() = 0;

  /**
   * @brief Start the execution of debugger just by commiting "run"
   * @param args - additional arguments being passed to a debugger during
   * runtime
   * @return On error, throws std::runtime_error
   */
  virtual void startDebug(const std::string &args) = 0;
  virtual void endDebug() = 0;
  virtual void setBreakpoint(PDBbr brpoint) = 0;
  virtual std::vector<std::string> readInput() = 0;

  virtual void checkInput(const std::vector<std::string> &) const = 0;

  // Default set of options being passed to a debugger
  static std::string getDefaultOptions() { return ""; };

  virtual bool getCurrentStatus() const { return isRunning; };

  virtual std::pair<std::size_t, std::string> getCurrentPosition() const {
    if (!isRunning) {
      throw std::logic_error("Cannot get the current source file position. The "
                             "debugging is not started");
    }

    return std::make_pair(currentLine, currentFile);
  }
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

public:
  // By default, gdb will launch with Machine Interface enabled
  GDBDebugger() {};
  virtual ~GDBDebugger() {};

  virtual void setBreakpoint(PDBbr);
  virtual PDBbr_list getBreakpointList() { return breakpoints; };
  virtual void startDebug(const std::string &);
  virtual void endDebug();
  virtual std::vector<std::string> readInput();

  virtual void checkInput(const std::vector<std::string> &) const;

  static std::string getDefaultOptions() { return "-q --interpreter=mi2"; };
};
} // namespace pdb
