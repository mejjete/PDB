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
  std::unique_ptr<llvm::DWARFContext>
      dwarf_context; // Object comprising of dwarf information on executable
  std::unique_ptr<llvm::MemoryBuffer>
      dwarf_mem_buf; // In-memory representation of an executable, needed for
                     // dwarf_context to operate on

  // Debugger instances associated with each debugging process
  std::vector<std::unique_ptr<PDBDebugger>> pdb_proc;

  // Parse the input string args into tokens separated by delim
  static std::vector<std::string> parseArgs(const std::string &args,
                                            const std::string &delim);

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
   *
   * PDBDebug<GDBDebugger>("mpirun -np 4 -oversubscribe", "/usr/bin/gdb",
   * "./mpi_test.out", "arg1, arg2, arg3");
   */
  static boost::leaf::result<PDBDebug> create(const std::string &start_rountine,
                                              const std::string &debugger,
                                              const std::string &exec);

  /**
   *  @return On success, return vector of strings, each containing full path
   *  to every source file recorded in executable.
   *  On error, return an error string
   */
  boost::leaf::result<std::vector<std::string>> getSourceFiles() const;

  /**
   *  @param func_name - name of the function in question
   *  @return On success, return pair describing function information
   *  first - location of a function in a source file (line)
   *  second - full path of a source file of a function in question
   */
  boost::leaf::result<std::pair<uint64_t, std::string>>
  getFunctionLocation(const std::string &func_name);

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
PDBDebug<DebuggerType>::create(const std::string &start_rountine,
                               const std::string &debugger,
                               const std::string &exec) {
  PDBDebug debugInstance;

  // Create and initialize in-memory representation of DWARF information
  // containing in executable
  auto expected_buffer = llvm::MemoryBuffer::getFile(exec);
  if (!expected_buffer)
    return boost::leaf::new_error<std::string>("Error reading executable");

  auto expected_obj_file = llvm::object::ObjectFile::createObjectFile(
      expected_buffer->get()->getMemBufferRef());
  if (!expected_obj_file)
    return boost::leaf::new_error<std::string>(
        "Error creating in-memory executable object");

  auto dwarf_context =
      std::move(llvm::DWARFContext::create(**expected_obj_file));
  if (!dwarf_context)
    return boost::leaf::new_error<std::string>(
        "Error initializing DWARF information");

  debugInstance.executable = exec;
  debugInstance.dwarf_mem_buf = std::move(*expected_buffer);
  debugInstance.dwarf_context = std::move(dwarf_context);

  // Tokenize command-line arguments
  auto pdb_routine_parsed =
      PDBDebug<DebuggerType>::parseArgs(start_rountine, " ;\n\r");
  auto pdb_default_debug_args = PDBDebug<DebuggerType>::parseArgs(
      DebuggerType::getDefaultOptions(), " ;\n\r");

  // Fetch process count from command-line argument string
  int proc_count;
  auto iter =
      std::find(pdb_routine_parsed.begin(), pdb_routine_parsed.end(), "-np");

  if (iter == pdb_routine_parsed.end()) {
    iter =
        std::find(pdb_routine_parsed.begin(), pdb_routine_parsed.end(), "-n");
    if (iter == pdb_routine_parsed.end())
      return boost::leaf::new_error<std::string>("Missing -np/-n option");
  }

  iter = std::next(iter);
  if (iter == pdb_routine_parsed.end())
    return boost::leaf::new_error<std::string>(
        "Number of processes not provided");

  proc_count = std::atoi(iter->c_str());
  if (proc_count <= 0)
    return boost::leaf::new_error<std::string>(
        "Invalid number of MPI processes");

  // Create temporal file to pass name of pipe
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
    debugInstance.pdb_proc.emplace_back(std::make_unique<DebuggerType>());
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
  int total_argc =
      pdb_routine_parsed.size() + pdb_default_debug_args.size() + 5;
  char **new_argv = new char *[total_argc];
  int new_arg_size = 0;

  // Step 1: copy routine call
  for (auto &token : pdb_routine_parsed) {
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

  // Step 3: copy debugger executable path
  new_argv[new_arg_size] = new char[debugger.length() + 1];
  memcpy(new_argv[new_arg_size], debugger.c_str(), debugger.length());
  new_argv[new_arg_size][debugger.length()] = 0;
  new_arg_size++;

  // Step 4: add default debugger arguments
  for (auto &token : pdb_default_debug_args) {
    new_argv[new_arg_size] = new char[token.length() + 1];
    memcpy(new_argv[new_arg_size], token.c_str(), token.length());
    new_argv[new_arg_size][token.length()] = 0;
    new_arg_size++;
  }

  // Step 5: add executable
  new_argv[new_arg_size] = new char[exec.length() + 1];
  memcpy(new_argv[new_arg_size], exec.c_str(), exec.length());
  new_argv[new_arg_size][exec.length()] = 0;
  new_arg_size++;

  // Step 6: add named pipe name
  new_argv[new_arg_size] = new char[strlen(temp_file) + 1];
  memcpy(new_argv[new_arg_size], temp_file, strlen(temp_file));
  new_argv[new_arg_size][strlen(temp_file)] = 0;
  new_arg_size++;

  // Step 7: add process count number
  std::string proc_count_char = std::to_string(proc_count);
  new_argv[new_arg_size] = new char[proc_count_char.length() + 1];
  memcpy(new_argv[new_arg_size], proc_count_char.c_str(),
         proc_count_char.length());
  new_argv[new_arg_size][proc_count_char.length()] = 0;
  new_arg_size++;

  // Terminating NULL string for exec function
  new_argv[new_arg_size] = NULL;

  // Spawn process
  debugInstance.exec_pid = fork();
  if (debugInstance.exec_pid == 0) {
    close(STDOUT_FILENO);
    close(STDIN_FILENO);
    close(STDERR_FILENO);

    std::string first_exec = pdb_routine_parsed[0];
    first_exec.push_back(0);

    if (execvp(first_exec.c_str(), new_argv) < 0)
      return boost::leaf::new_error<std::string>("execvp error");
  }

  // After exec, we can finally free argument string
  for (int i = 0; i < new_arg_size - 1; i++)
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
std::vector<std::string>
PDBDebug<DebuggerType>::parseArgs(const std::string &pdb_args,
                                  const std::string &delim) {
  char *args = new char[pdb_args.length()];
  memcpy(args, pdb_args.c_str(), pdb_args.length());
  args[pdb_args.length()] = 0;
  std::vector<std::string> pdb_args_parsed;

  char *token = strtok(args, delim.c_str());
  if (token == NULL)
    return pdb_args_parsed;

  do {
    pdb_args_parsed.push_back(token);
  } while ((token = strtok(NULL, delim.c_str())));

  return pdb_args_parsed;
}

template <typename DebuggerType>
boost::leaf::result<void>
PDBDebug<DebuggerType>::setBreakpointsAll(PDBbr brpoint) {
  if (pdb_proc.size() == 0)
    return boost::leaf::new_error<std::string>(
        "PDB: Invalid number of processes: 0");

  for (auto &iter : pdb_proc) {
    auto result = iter->setBreakpoint(brpoint);
    if (!result)
      return result;
  }
  return {};
}

template <typename DebuggerType>
boost::leaf::result<void>
PDBDebug<DebuggerType>::setBreakpoint(size_t proc, PDBbr brpoints) {
  if (proc >= pdb_proc.size())
    return boost::leaf::new_error<std::string>(
        "PDB: Invalid process identifier: " + std::to_string(proc));

  auto &ptr = pdb_proc[proc];
  auto result = ptr->setBreakpoint(brpoints);
  if (!result)
    return result;
  return {};
}

template <typename DebuggerType>
boost::leaf::result<std::vector<std::string>>
PDBDebug<DebuggerType>::getSourceFiles() const {
  std::vector<std::string> files;

  for (const auto &CU : dwarf_context->compile_units()) {
    if (!CU)
      continue;
    const auto *lt = dwarf_context->getLineTableForUnit(CU.get());
    if (!lt)
      continue;

    for (const auto &entry : lt->Prologue.FileNames) {
      std::string path;

      if (entry.DirIdx > 0 &&
          entry.DirIdx <= lt->Prologue.IncludeDirectories.size()) {
        if (auto dir = lt->Prologue.IncludeDirectories[entry.DirIdx - 1]
                           .getAsCString())
          path = *dir;
        path += "/";
      }

      if (auto name = entry.Name.getAsCString())
        path += *name;

      if (!path.empty())
        files.push_back(path);
    }
  }

  return files;
}

template <typename DebuggerType>
boost::leaf::result<std::pair<uint64_t, std::string>>
PDBDebug<DebuggerType>::getFunctionLocation(const std::string &func_name) {
  for (const auto &CU : dwarf_context->compile_units()) {
    for (const auto &entry : CU->dies()) {
      llvm::DWARFDie die(CU.get(), &entry);

      if (die.getTag() == llvm::dwarf::DW_TAG_subprogram) {
        if (const char *name = die.getName(llvm::DINameKind::ShortName)) {
          if (func_name == name) {
            // Obtain function source file
            std::string file_idx = die.getDeclFile(
                llvm::DILineInfoSpecifier::FileLineInfoKind::RawValue);
            // Obtain function line number
            uint64_t file_line = die.getDeclLine();
            return std::make_pair(file_line, file_idx);
          }
        }
      }
    }
  }

  return boost::leaf::new_error<std::string>("Unknown function: " + func_name);
}
} // namespace pdb
