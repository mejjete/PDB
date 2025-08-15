#include "PDB.hpp"
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool is_signaled = false;
void PDBcommand(pdb::PDBDebug &pdb_instance);

int main() {
  using namespace pdb;

  PDBDebug pdb_instance;
  pdb_instance.launch("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3",
                      PDB_Debug_type::GDB);

  PDBcommand(pdb_instance);

  pdb_instance.join(100000);
  return 0;
}

void PDBcommand(pdb::PDBDebug &pdb_instance) {
  std::cout << "All Source Files: \n";

  // Print out the full information about source files
  auto source_array = pdb_instance.getSourceFiles();
  for (auto &iter : source_array)
    printf("\033[92m%s\033[0m,", iter.c_str());
  std::cout << std::endl;
  std::cout << "---------------------------------------------------------------"
               "------------\n";

  // Print out information about given function
  std::string func = "main";
  auto function = pdb_instance.getFunction("main");
  std::cout << "Function: " << func << std::endl;
  std::cout << "\033[92mSource File: " << function.second << "\033[0m"
            << std::endl;
  std::cout << "\033[92mLine: " << function.first << "\033[0m" << std::endl;

  bool caught = false;
  try {
    // Set up a breakpoint
    pdb::PDBDebug::PDBbr br(1, function.second);
    pdb_instance.setBreakpointsAll(br);
  } catch (std::runtime_error &err) {
    caught = true;
  }

  std::cout << "---------------------------------------------------------------"
               "------------\n";
  std::cout << "Breakpoint: " << function.second << ":" << 1 << std::endl;
  if (caught == false)
    std::cout << "\033[92mBreakpoint set at: " << function.second << ":" << 1
              << "\033[0m\n";
  else
    std::cout << "\033[92mError setting breakpoint at: " << function.second
              << ":" << 1 << "\033[0m\n";
}
