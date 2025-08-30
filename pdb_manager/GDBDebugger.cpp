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
  // Fetch all lines from input until we get the terminating symbol in gdb
  return fetchByLinesUntil(term);
}

void GDBDebugger::checkInput(const std::vector<std::string> &str) const {
  for (auto &iter : str) {
    if (iter.find("No debugging symbols found") != std::string::npos)
      throw std::runtime_error("No debugging symbols found");
  }
}

void GDBDebugger::endDebug() {
  std::string command = makeCommand("quit");
  submitCommand(command);

  // Skip whatever gdb has left on stdout just to let it exit peacefully
  fetchByLinesUntil(term);
}

void GDBDebugger::setBreakpoint(PDBbr brpoint) {
  if (brpoint.second.length() == 0)
    throw std::logic_error("Error setting breakpoint in unknown file");

  // If breakpoint has not been set yet, add it to breakpoint list
  std::string command =
      makeCommand("b " + brpoint.second + ":" + std::to_string(brpoint.first));
  submitCommand(command);

  auto result = readInput();
  checkInput(result);

  /**
   * At this moment, we may want to check output from "br" command
   * to understand if breakpoint has been set. We have to check second
   * string if it has substring "No source file" and the line right
   * before "^done" on more detailed information if so exist
   */
  if (strstr(result[1].c_str(), "No source file") != NULL) {
    throw std::logic_error(
        "Cannot set breakpoint at specified location: " + brpoint.second + ":" +
        std::to_string(brpoint.first));
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

  // Return an error if we failed to parse command
  if (strstr(token, "addr") == NULL) {
    throw std::logic_error("Failed parsing <br " + brpoint.second + ":" +
                           std::to_string(brpoint.first) + ">");
  }

  // If breakpoint has status "PENDING", it means we cannot obtain its
  // information from executable
  if (strstr(token, "<PENDING>") != NULL) {
    throw std::logic_error("Breakpoint is already set at: " + brpoint.second +
                           ":" + std::to_string(brpoint.first));
  }

  // If status contains address, we successfully set up a breakpoint, now add it
  // to list
  bool br_found = false;

  for (auto &source_files : breakpoints) {
    if (source_files.first == brpoint.second) {
      source_files.second.push_back(brpoint.first);
      br_found = true;
    }
  }

  // Add it as new pair
  if (br_found != true) {
    breakpoints.push_back(
        std::make_pair(brpoint.second, std::vector<int>(1, brpoint.first)));
  }
}
} // namespace pdb
