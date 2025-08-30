#include <PDB.hpp>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// bool is_signaled = false;
using Debugger = pdb::PDBDebug<pdb::GDBDebugger>;

void PDBcommand(Debugger &pdb_instance) {
  std::cout << "All Source Files: \n";

  // Print out the full information about source files
  auto source_array = pdb_instance.getSourceFiles();
  for (auto &iter : *source_array) {
    printf("\033[92m%s\033[0m,", iter.c_str());
    std::cout << std::endl;
  }
  std::cout << "---------------------------------------------------------------"
               "------------\n";

  // Print out information about given function std::string func = "main";
  std::string func_in_question = "main";
  auto function = pdb_instance.getFunctionLocation(func_in_question);
  std::cout << "Function: " << func_in_question << std::endl;
  std::cout << "\033[92mSource File: " << function->second << "\033[0m"
            << std::endl;
  std::cout << "\033[92mLine: " << function->first << "\033[0m" << std::endl;
  std::cout << "---------------------------------------------------------------"
               "------------\n";

  // Set up a breakpoint
  std::cout << "Setting breakpoint at: " << function->second << ":"
            << function->first << std::endl;
  Debugger::PDBbr br(function->first, function->second);
  auto br_result = pdb_instance.setBreakpointsAll(br);

  if (br_result) {
    std::cout << "\033[92mBreakpoints set at: " << function->second << ":"
              << function->first << "\033[0m\n";
  } else {
    std::cout << "\033[92mError setting breakpoints at: " << function->second
              << ":" << function->first << "\033[0m\n";
  }
}

int main() {
  using namespace pdb;
  auto debug =
      Debugger::create("mpirun -np 1", "/usr/bin/gdb", "./mpi_test.out");
  PDBcommand(*debug);
  return 0;
}
