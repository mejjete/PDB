#include "PDBDebugger.hpp"
#include <cstring>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace pdb {
std::string GDBDebugger::term = "(gdb) ";

std::vector<std::string> GDBDebugger::readInput() {
  std::vector<std::string> main_buffer;

  while (true) {
    std::string iter = read();
    if (iter == term)
      break;

    if (iter.length() == 0)
      continue;

    main_buffer.push_back(iter);
  }

  return main_buffer;
}

void GDBDebugger::checkInput(const std::vector<std::string> &str) const {
  for (auto &iter : str) {
    if (iter.find("No debugging symbols found") != std::string::npos)
      throw std::runtime_error("PDB: No debugging symbols found");
  }
}

std::vector<std::string> GDBDebugger::stringifyInput(std::string input) {
  char *list = new char[input.length() + 1];
  memcpy(list, input.c_str(), input.length());
  list[input.length()] = 0;

  char *token = strtok(list, "\n");
  if (token == NULL)
    return std::vector(1, input);

  std::vector<std::string> string_array;
  do {
    string_array.push_back(token);
  } while ((token = strtok(NULL, "\n")));

  return string_array;
}

std::vector<std::string> GDBDebugger::getSourceFiles() {
  std::string command = makeCommand("info sources");
  write(command);

  auto result = readInput();
  checkInput(result);

  // Find first occurence of ^done
  auto done_expr = std::ranges::find(result, "^done");
  if (done_expr == result.end())
    throw std::runtime_error("PDB: Failed parsing <info sources>");

  // If no errors occured so far, previous string relative to "^done" is list of
  // source files
  auto source_list = done_expr - 1;
  std::vector<std::string> sources;

  char *list = new char[source_list->length() + 1];
  memcpy(list, source_list->c_str(), source_list->length());
  list[source_list->length()] = 0;

  char *token = strtok(list, " ,");
  if (token == NULL)
    throw std::runtime_error("PDB: Failed parsing <info sources>");

  do {
    sources.push_back(token);
  } while ((token = strtok(NULL, " ,")));
  delete[] list;

  // GDB usually annotates its output with special characters, following line of
  // code removes them

  // Remove command header from first file
  {
    auto &first_file = sources[0];
    std::string new_string(first_file.begin() + 2, first_file.end());
    first_file = new_string;
  }

  // Remove 2 newline characters from last file
  {
    auto &last_file = sources[0];
    last_file.pop_back();
    last_file.pop_back();
  }

  return sources;
}

std::pair<int, std::string> GDBDebugger::getFunction(std::string func_name) {
  if (func_name.length() == 0)
    throw std::runtime_error("PDB: Empty function name");

  std::string command = makeCommand("info func " + func_name);
  write(command);

  auto result = readInput();
  checkInput(result);

  // Pointer to a vector position containing function information
  size_t func_source = 0; // Path to a source file containing function
  size_t func_decl = 0;   // Function declaration

  for (size_t i = 0; i < result.size(); i++) {
    auto &iter = result[i];

    if (iter == "^done") {
      // If string prior to "^done" is as follow, that means no function is
      // found
      auto &prev = result.at(i);

      if (strstr(prev.c_str(), "All functions matching regular expression") !=
          NULL)
        throw std::runtime_error("PDB: No such function is found: " +
                                 func_name);
      else {
        func_source = i - 2;
        func_decl = i - 1;
      }

      break;
    }
  }

  if (func_decl == func_source)
    throw std::runtime_error("PDB: Failed parsing <info func " + func_name +
                             ">");

  if (strstr(result[func_source].c_str(), "Non-debugging symbol") != NULL)
    throw std::runtime_error(
        "PDB: Cannot obtain information about non-debugging symbols");

  // Fetch path to source file in which function is located
  auto &string_func_source = result[func_source];
  std::string function_declaration(string_func_source.begin() + 2,
                                   string_func_source.end() - 4);

  // Fetch line at which function is declared
  std::string accum;
  auto &string_func_decl = result[func_decl];
  size_t i = 2;

  while (isdigit(string_func_decl[i])) {
    accum += string_func_decl[i];
    i++;
  }

  return std::make_pair(std::atoi(accum.c_str()), function_declaration);
}

void GDBDebugger::endDebug() {
  std::string command = makeCommand("quit");
  write(command);

  // Skip whatever gdb has left on stdout just to let it exit peacefully
  read();
}

void GDBDebugger::setBreakpoint(PDBbr brpoint) {
  if (brpoint.second.length() == 0)
    throw std::runtime_error("PDB: Error setting breakpoint in unknown file");

  bool br_found = false;

  // Check if we have already set up this breakpoint
  for (auto &source_files : breakpoints) {
    if (source_files.first == brpoint.second) {
      auto br = std::ranges::find(source_files.second, brpoint.first);
      if (br != source_files.second.end()) {
        br_found = true;
        break;
      }
    }
  }

  // If breakpoint is already set on given position, return
  if (br_found == true)
    return;

  // If breakpoint has not been set yet, add it to breakpoint list
  std::string command =
      makeCommand("b " + brpoint.second + ":" + std::to_string(brpoint.first));
  write(command);

  auto result = readInput();
  checkInput(result);

  /**
   *  At this moment, we may want to check output from "br" command
   *  to understand if breakpoint has been set. We have to check second
   *  string if it has substring "No source file" and the line right
   *  before "^done" on more detailed information if so exist
   */
  if (strstr(result[1].c_str(), "No source file") != NULL) {
    throw std::runtime_error(
        "PDB: Cannot set breakpoint at specified location: " + brpoint.second +
        ":" + std::to_string(brpoint.first));
  }

  // Find string that starts with "=breakpoint-created"
  auto br_created = std::ranges::find(result, "^done") - 1;

  // Tokenize string
  char *list = new char[br_created->length() + 1];
  memcpy(list, br_created->c_str(), br_created->length());
  list[br_created->length()] = 0;

  char *token = strtok(list, ",");

  do {
    if (strstr(token, "addr") != NULL)
      break;
  } while ((token = strtok(NULL, ",")));

  // Thow an exception if we failed to parse command
  if (strstr(token, "addr") == NULL) {
    throw std::runtime_error("PDB: Failed parsing <br " + brpoint.second + ":" +
                             std::to_string(brpoint.first) + ">");
  }

  // If breakpoint has status "PENDING", it means we cannot obtain its
  // information from executable
  if (strstr(token, "<PENDING>") != NULL) {
    throw std::runtime_error(
        "PDB: Cannot set breakpoint at specified location: " + brpoint.second +
        ":" + std::to_string(brpoint.first));
  }

  // If status contains address, we successfully set up a breakpoint, now add it
  // to list
  br_found = false;

  for (auto &source_files : breakpoints) {
    if (source_files.first == brpoint.second) {
      source_files.second.push_back(brpoint.first);
      br_found = true;
    }
  }

  // Add it as new pair
  if (br_found != true)
    breakpoints.push_back(
        std::make_pair(brpoint.second, std::vector<int>(1, brpoint.first)));
}
} // namespace pdb
