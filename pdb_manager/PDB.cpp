#include <PDB.hpp>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using Debugger = pdb::PDBDebug<pdb::GDBDebugger>;

void brCommand(const std::vector<std::string> &command, Debugger &pdb_instance);
void infoCommand(const std::vector<std::string> &command,
                 Debugger &pdb_instance);

void PDBcommand(Debugger &pdb_instance) {
  std::string command;
  do {
    try {
      std::cout << "(pdb) ";
      std::getline(std::cin, command);

      std::stringstream sstream(command);
      std::vector<std::string> comm_parsed;
      std::string temp;
      while (sstream >> temp)
        comm_parsed.push_back(temp);

      if (comm_parsed[0] == "b") {
        brCommand(comm_parsed, pdb_instance);
      } else if (comm_parsed[0] == "info" && comm_parsed.size() > 1) {
        infoCommand(comm_parsed, pdb_instance);
      } else if (command == "q") {
        break;
      } else {
        throw std::logic_error("Invalid command: " + comm_parsed[0]);
      }
    } catch (std::runtime_error &re) { // Fatal error
      std::cout << "Fatal error: " << re.what() << std::endl;
      std::cout << "Terminating..." << std::endl;
      break;
    } catch (std::logic_error &le) { // Logic error
      std::cout << le.what() << std::endl;
    }
  } while (true);
}

void brCommand(const std::vector<std::string> &commands,
               Debugger &pdb_instance) {
  if (commands.size() != 2) {
    throw std::logic_error("Invalid number of arguments: " +
                           std::to_string(commands.size()));
  }

  auto delim = commands[1].find(':');
  if (delim == std::string::npos) {
    throw std::logic_error("Invalid breakpoint location: " + commands[1]);
  }

  std::string fileName = commands[1].substr(0, delim);
  std::string pos = commands[1].substr(delim + 1, commands[1].length() - delim);
  int filePos = std::atoi(pos.c_str());
  if (filePos < 0) {
    throw std::logic_error("Invalid line number: " + pos);
  }

  Debugger::PDBbr br(static_cast<std::size_t>(filePos), fileName);
  pdb_instance.setBreakpointsAll(br);
  std::cout << "\033[92mBreakpoints set at: " << fileName << ":" << pos
            << "\033[0m\n";
}

void infoCommand(const std::vector<std::string> &command,
                 Debugger &pdb_instance) {
  if (command[1] == "sources") {
    auto source_list = pdb_instance.getSourceFiles();

    for (auto &iter : source_list) {
      printf("\033[92m%s\033[0m,", iter.c_str());
      std::cout << std::endl;
    }
  } else if (command[1] == "func") {
    std::string func_in_question = command[2];
    auto function = pdb_instance.getFunctionLocation(func_in_question);
    std::cout << "\033[92m" << function.second << "\033[0m" << ":";
    std::cout << "\033[92m" << function.first << "\033[0m" << std::endl;
  }
};

int main() {
  using namespace pdb;
  auto debug = Debugger("mpirun -np 1", "/usr/bin/gdb", "./mpi_test.out");
  PDBcommand(debug);
  return 0;
}
