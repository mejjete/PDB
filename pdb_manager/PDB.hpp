#pragma once

#include <PDBDebugger.hpp>
#include <algorithm>
#include <boost/leaf.hpp>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#include <memory>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace pdb {
#define PDB_PIPE_LENGTH 20

/**
 *  Main Debug instance which communicates with UI
 */
template <typename DebuggerType> class PDBDebug {
protected:
  PDBDebug() {};

private:
  int temporal_file; // File used to pass arguments through PDB runtime
  std::string temporal_file_name; // Temporal pipe for arguments passing
  std::string executable;         // User-supplied executable name
  pid_t exec_pid;                 // Executable PID process

  // Debugger instances associated with each debugging process
  std::vector<std::unique_ptr<PDBDebugger>> pdb_proc;

  // Parse the input string args into tokens separated by delim
  static std::vector<std::string> parseArgs(std::string args,
                                            std::string delim);

public:
  using PDBbr = typename PDBDebugger::PDBbr;
  PDBDebug(const PDBDebug &) = delete;
  PDBDebug(PDBDebug &&) = default;
  ~PDBDebug();

  /**
   * @param start_routine - should specify the compiler instance and
   * target-specific flags [example] : "mpirun -oversubscribe -np 4"
   * @param debugger - path to debugger
   * @param exec - user-supplied executable
   * @param args - arguments to be passed directly to executable
   * @param dargs - arguments to be passed directly to external debugger
   *
   * PDBDebug<GDBDebugger>("mpirun -np 4 -oversubscribe", "/usr/bin/gdb",
   * "./mpi_test.out", "arg1, arg2, arg3", [following arguments being passed to
   * a DebuggerType during construction]);
   */
  static boost::leaf::result<PDBDebug> create(std::string start_rountine,
                                              std::string debugger,
                                              std::string exec,
                                              std::string args);

  /**
   *  @return On success, return vector of strings, each containing full path
   *  to every source file recorded in executable.
   *  On error, return an error string
   */
  boost::leaf::result<std::vector<std::string>> getSourceFiles() const {
    return boost::leaf::new_error<std::string>(
        "getSourceFiles(): not implemented");
  }

  /**
   *  @param func_name - name of the function in question
   *  @return On success, returns pair describing function information
   *  first - location of a function in a source file (line)
   *  second - full path of a source file of a given function
   */
  boost::leaf::result<std::pair<size_t, std::string>>
  getFunctionLocation(const std::string &func_name) const {
    return boost::leaf::new_error("getFunctionLocation(): not implemented");
  };

  boost::leaf::result<void> setBreakpointsAll(PDBbr brpoint);
  boost::leaf::result<void> setBreakpoint(size_t proc, PDBbr brpoints);

  void startDebug() {};
  void endDebug() {};

  /**
   * @param usec - miliseconds to wait to terminate all processes
   *
   * Must be called to properly terminate calling process. Returns pair of
   * values describing main process exit status. .first - holds 1 if child has
   * terminated and 0 otherwise .second - holds exit code of a child
   *
   * Because this function uses wait() in a non-blocking mode, it can be used
   * by high-level routines to wait until process termination or just kill it by
   * calling destructor
   */
  boost::leaf::result<std::pair<int, int>> join(useconds_t usec = 1000000);

  /**
   * Return number of active processes
   */
  size_t size() const { return pdb_proc.size(); };
};

template <typename DebuggerType>
boost::leaf::result<PDBDebug<DebuggerType>>
PDBDebug<DebuggerType>::create(std::string start_rountine, std::string debugger,
                               std::string exec, std::string args) {
  PDBDebug debugInstance;
  debugInstance.executable = exec;

  std::vector<std::string> pdb_args_parced;
  std::vector<std::string> pdb_routine_parced;

  // Tokenize command-line arguments
  pdb_args_parced = PDBDebug<DebuggerType>::parseArgs(args, " ;\n\r");
  pdb_routine_parced =
      PDBDebug<DebuggerType>::parseArgs(start_rountine, " ;\n\r");

  // Fetch process count from command-line argument string
  int proc_count;
  auto iter =
      std::find(pdb_routine_parced.begin(), pdb_routine_parced.end(), "-np");

  if (iter == pdb_routine_parced.end()) {
    iter =
        std::find(pdb_routine_parced.begin(), pdb_routine_parced.end(), "-n");
    if (iter == pdb_routine_parced.end())
      return boost::leaf::new_error<std::string>("Missing -np/-n option");
  }

  iter = std::next(iter);
  if (iter == pdb_args_parced.end())
    return boost::leaf::new_error<std::string>(
        "Number of processes not provided");

  proc_count = std::atoi(iter->c_str());
  if (proc_count <= 0)
    return boost::leaf::new_error<std::string>(
        "Invalid number of MPI processes");

  // Create temporal file to pass name of pipes
  char temp_file[] = "/tmp/pdbpipeXXXXXXX";
  debugInstance.temporal_file = mkstemp(temp_file);
  if (debugInstance.temporal_file < 0)
    return boost::leaf::new_error<std::string>(
        std::string("Error opening temporal file: ") + temp_file);

  close(debugInstance.temporal_file);
  unlink(temp_file);
  debugInstance.temporal_file =
      open(temp_file, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

  if (debugInstance.temporal_file < 0)
    return boost::leaf::new_error<std::string>(
        std::string("Error opening temporal file: ") + temp_file);
  debugInstance.temporal_file_name = temp_file;

  // Create specified number of process handlers
  std::vector<std::string> proc_name_files;
  debugInstance.pdb_proc.reserve(proc_count);

  for (int i = 0; i < proc_count; i++) {
    debugInstance.pdb_proc.emplace_back(
        std::make_unique<DebuggerType>(debugger, exec));
    auto proc_filenames = debugInstance.pdb_proc[i]->getPipeNames();

    // Memorize pipe names
    proc_name_files.push_back(proc_filenames.first);
    proc_name_files.push_back(proc_filenames.second);

    write(debugInstance.temporal_file, proc_filenames.first.c_str(),
          PDB_PIPE_LENGTH);
    write(debugInstance.temporal_file, proc_filenames.second.c_str(),
          PDB_PIPE_LENGTH);
  }

  /**
   * Modify command-line argument string and add extra arguments
   *
   * To spawn any process, execvp is used. It accepts char[] as an argument
   * vector. The following code adds a few extra arguments. This behavior is
   * implementation-defined and must synchronize in the receving process
   */
  int old_argc = pdb_routine_parced.size() + 2;
  int total_argc = old_argc + 6;
  char **new_argv = new char *[total_argc];
  int new_arg_size = 0;

  // Step 1: copy routine call
  for (auto &token : pdb_routine_parced) {
    new_argv[new_arg_size] = new char[token.length() + 1];
    memcpy(new_argv[new_arg_size], token.c_str(), token.length());
    new_argv[new_arg_size][token.length()] = 0;
    new_arg_size++;
  }

  // Step 2: copy PDB launch application to prepare PDB runtime
  char pdb_launch[] = "./pdb_launch";
  new_argv[new_arg_size] = new char[strlen(pdb_launch) + 1];
  memcpy(new_argv[new_arg_size], pdb_launch, sizeof(pdb_launch));
  new_argv[new_arg_size][strlen(pdb_launch)] = 0;
  new_arg_size++;

  // Step 3: copy debugger executable and
  new_argv[new_arg_size] = new char[debugger.length() + 1];
  memcpy(new_argv[new_arg_size], debugger.c_str(), debugger.length());
  new_argv[new_arg_size][debugger.length()] = 0;
  new_arg_size++;

  // Add executable
  new_argv[new_arg_size] = new char[exec.length() + 1];
  memcpy(new_argv[new_arg_size], exec.c_str(), exec.length());
  new_argv[new_arg_size][exec.length()] = 0;
  new_arg_size++;

  // Step 4: add extra arguments, namely temporal file and process number
  new_argv[new_arg_size] = new char[strlen(temp_file) + 1];
  memcpy(new_argv[new_arg_size], temp_file, strlen(temp_file));
  new_argv[new_arg_size][strlen(temp_file)] = 0;
  new_arg_size++;

  std::string proc_count_char = std::to_string(proc_count);
  new_argv[new_arg_size] = new char[proc_count_char.length() + 1];
  memcpy(new_argv[new_arg_size], proc_count_char.c_str(),
         proc_count_char.length());
  new_argv[new_arg_size][proc_count_char.length()] = 0;
  new_arg_size++;

  // Terminating NULL string for exec function
  new_argv[new_arg_size] = NULL;

  // std::cout << "Forking process\n";

  // Spawn process
  debugInstance.exec_pid = fork();
  if (debugInstance.exec_pid == 0) {
    close(STDOUT_FILENO);
    close(STDIN_FILENO);
    close(STDERR_FILENO);

    std::string first_exec = pdb_routine_parced[0];
    first_exec.push_back(0);

    if (execvp(first_exec.c_str(), new_argv) < 0)
      return boost::leaf::new_error<std::string>("execvp error");
  }

  // After exec, we can finally free argument string
  for (int i = 0; i < new_arg_size; i++)
    delete[] new_argv[i];

  delete[] new_argv;

  /**
   * At this point, child process which is now PDB launch will try to open FIFO
   * and block because FIFO is blocked until it is dual-opened.
   * The following open() calls should be synchronized with the same open() in
   * a child.
   */
  for (auto &proc : debugInstance.pdb_proc) {
    if (proc->openFIFO() < 0)
      return boost::leaf::new_error<std::string>("error opening FIFO");
  }

  // Read out initial print from gdb to clear input for subsequent commands
  for (auto &proc : debugInstance.pdb_proc) {
    auto check = proc->checkInput(proc->readInput());
    if (!check)
      boost::leaf::new_error<std::string>("Error happens");
  }

  return debugInstance;
}

template <typename DebuggerType> PDBDebug<DebuggerType>::~PDBDebug() {
  if (temporal_file >= 0)
    close(temporal_file);

  if (temporal_file_name.length() > 0)
    unlink(temporal_file_name.c_str());

  if (exec_pid > 0) {
    // Second chance to terminate process
    int statlock;
    pid_t pid = waitpid(exec_pid, &statlock, WNOHANG);
    if (pid == 0) {
      printf("PDB: Killing main process\n");
      kill(exec_pid, SIGKILL);
    }
  }
}

template <typename DebuggerType>
boost::leaf::result<std::pair<int, int>>
PDBDebug<DebuggerType>::join(useconds_t usec) {
  for (auto &iter : pdb_proc)
    iter->endDebug();

  // Try to wait a while to let the processes terminate
  usleep(usec);

  int statlock;
  pid_t pid = waitpid(exec_pid, &statlock, WNOHANG);
  if (pid < 0) {
    return boost::leaf::new_error<std::string>(
        "Error waiting for main process");
  } else if (pid == 0)
    return std::make_pair(0, 0); // If process has not terminated yet, do
                                 // nothing, destructor will kill it
  else {
    exec_pid = 0;

    if (WIFEXITED(statlock))
      return std::make_pair(1, WEXITSTATUS(statlock));
    else if (WIFSIGNALED(statlock))
      return std::make_pair(1, WTERMSIG(statlock));
    else if (WIFSTOPPED(statlock))
      return std::make_pair(1, WSTOPSIG(statlock));
    else {
      exec_pid = pid;
      return std::make_pair(0, 0);
    }
  }

  return std::make_pair(0, 0);
}

template <typename DebuggerType>
std::vector<std::string> PDBDebug<DebuggerType>::parseArgs(std::string pdb_args,
                                                           std::string delim) {
  char *args = new char[pdb_args.length()];
  memcpy(args, pdb_args.c_str(), pdb_args.length());
  args[pdb_args.length()] = 0;
  std::vector<std::string> pdb_args_parced;

  char *token = strtok(args, delim.c_str());
  if (token == NULL)
    return pdb_args_parced;

  do {
    pdb_args_parced.push_back(token);
  } while ((token = strtok(NULL, delim.c_str())));

  return pdb_args_parced;
}

template <typename DebuggerType>
boost::leaf::result<void>
PDBDebug<DebuggerType>::setBreakpointsAll(PDBbr brpoint) {
  if (pdb_proc.size() == 0)
    return boost::leaf::new_error<std::string>(
        "PDB: Invalid number of processes: 0");

  for (auto &iter : pdb_proc)
    iter->setBreakpoint(brpoint);
}

template <typename DebuggerType>
boost::leaf::result<void>
PDBDebug<DebuggerType>::setBreakpoint(size_t proc, PDBbr brpoints) {
  if (proc >= pdb_proc.size())
    throw std::runtime_error("PDB: Invalid process identifier: " +
                             std::to_string(proc));

  auto &ptr = pdb_proc[proc];
  ptr->setBreakpoint(brpoints);
}
} // namespace pdb
