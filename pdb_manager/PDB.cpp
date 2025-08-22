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

  //   std::cout <<
  //   "---------------------------------------------------------------"
  //                "------------\n";
  //   bool caught = false;
  //   try {
  //     // Set up a breakpoint
  //     Debugger::PDBbr br(1, function.second);
  //     pdb_instance.setBreakpointsAll(br);
  //   } catch (std::runtime_error &err) {
  //     caught = true;
  //   }

  //   std::cout << "Breakpoint: " << function.second << ":" << 1 << std::endl;
  //   if (caught == false)
  //     std::cout << "\033[92mBreakpoint set at: " << function.second << ":" <<
  //     1
  //               << "\033[0m\n";
  //   else
  //     std::cout << "\033[92mError setting breakpoint at: " << function.second
  //               << ":" << 1 << "\033[0m\n";
}

int main() {
  using namespace pdb;
  auto debug = Debugger::create("mpirun -np 4", "/usr/bin/gdb",
                                "./mpi_test.out", "arg1 arg2 arg3");
  PDBcommand(*debug);
  return 0;
}
