#include <PDBDebugger.hpp>
#include <algorithm>
#include <cstring>
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

boost::leaf::result<void>
GDBDebugger::checkInput(const std::vector<std::string> &str) const {
  for (auto &iter : str) {
    if (iter.find("No debugging symbols found") != std::string::npos)
      return boost::leaf::new_error<std::string>("No debugging symbols found");
  }
  return {};
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

boost::leaf::result<void> GDBDebugger::endDebug() {
  std::string command = makeCommand("quit");
  write(command);

  // Skip whatever gdb has left on stdout just to let it exit peacefully
  read();
  return {};
}

boost::leaf::result<void> GDBDebugger::setBreakpoint(PDBbr brpoint) {
  if (brpoint.second.length() == 0)
    return boost::leaf::new_error<std::string>(
        "Error setting breakpoint in unknown file");

  bool br_found = false;

  // Check if we have already set up this breakpoint
  for (auto &source_files : breakpoints) {
    if (source_files.first == brpoint.second) {
      auto br = std::find(source_files.second.begin(),
                          source_files.second.end(), brpoint.first);
      if (br != source_files.second.end()) {
        br_found = true;
        break;
      }
    }
  }

  // If breakpoint is already set on given position, return
  if (br_found == true)
    return {};

  // If breakpoint has not been set yet, add it to breakpoint list
  std::string command =
      makeCommand("b " + brpoint.second + ":" + std::to_string(brpoint.first));
  write(command);

  auto result = readInput();
  auto check = checkInput(result);
  if (!check)
    return check;

  /**
   * At this moment, we may want to check output from "br" command
   * to understand if breakpoint has been set. We have to check second
   * string if it has substring "No source file" and the line right
   * before "^done" on more detailed information if so exist
   */
  if (strstr(result[1].c_str(), "No source file") != NULL) {
    return boost::leaf::new_error<std::string>(
        "PDB: Cannot set breakpoint at specified location: " + brpoint.second +
        ":" + std::to_string(brpoint.first));
  }

  // Find string that starts with "=breakpoint-created"
  auto br_created = std::find(result.begin(), result.end(), "^done") - 1;

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
    return boost::leaf::new_error<std::string>(
        "PDB: Failed parsing <br " + brpoint.second + ":" +
        std::to_string(brpoint.first) + ">");
  }

  // If breakpoint has status "PENDING", it means we cannot obtain its
  // information from executable
  if (strstr(token, "<PENDING>") != NULL) {
    return boost::leaf::new_error<std::string>(
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

  return {};
}
} // namespace pdb
