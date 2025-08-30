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
  // Fetch all lines from input until we get the terminating symbol
  auto result = fetchByLinesUntil(term);

  // Update status and internal state
  for (auto &i : result) {
    // Check whether we started an application
    if (i.find("^running") != std::string::npos)
      isRunning = true;

    if (i.find("*stopped") != std::string::npos) {
      // Get exact current breakpoint file
      auto fullNameStart = i.find("fullname=\"");
      if (fullNameStart == std::string::npos)
        throw std::runtime_error("Invalid debugger output");

      auto fullNameEnd =
          std::find(i.begin() + fullNameStart + 11, i.end(), '\"');
      if (fullNameEnd == i.end())
        throw std::runtime_error("Invalid debugger output");

      std::string fullPath(i.begin() + fullNameStart + 11, fullNameEnd);

      // Get exact current breakpoint line number
      auto lineStart = i.find("line=\"");
      if (lineStart == std::string::npos)
        throw std::runtime_error("Invalid debugger output");

      auto lineEnd = std::find(i.begin() + lineStart + 7, i.end(), '\"');
      if (lineEnd == i.end())
        throw std::runtime_error("Invalid debugger output");

      std::string lineNumberStr(i.begin() + lineStart + 7, lineEnd);
      std::size_t lineNumber =
          static_cast<std::size_t>(std::atoi(lineNumberStr.c_str()));

      // Validate source file position
      currentFile = fullPath;
      currentLine = lineNumber;
    };
  }

  return result;
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
  readInput();
  isRunning = false;
}

void GDBDebugger::startDebug(const std::string &args) {
  auto command = makeCommand("r " + args);
  submitCommand(command);

  auto result = readInput();
  checkInput(result);

  for (auto &iter : result) {
    if (iter[0] == '^') {
      if (iter.find("error") != std::string::npos)
        throw std::logic_error("Error starting debugging");
    }

    if (iter.find("^done") != std::string::npos)
      return;
  }

  // To get to the breakpoint
  result = readInput();
  for (auto &iter : result) {
    if (iter.find("error") != std::string::npos)
      throw std::runtime_error("Debugging error");

    if (iter.find("*stopped") != std::string::npos &&
        iter.find("breakpoint-hit") == std::string::npos)
      throw std::runtime_error("Unable to start debugging");
  }

  isRunning = true;
}

void GDBDebugger::setBreakpoint(PDBbr brpoint) {
  if (brpoint.second.length() == 0)
    throw std::logic_error("Error setting breakpoint in unknown file");

  // Check if breakpoint is already set
  bool br_found = false;

  for (auto &brs : breakpoints) {
    if (brs.first == brpoint.second) {
      brs.second.push_back(brpoint.first);
      br_found = true;
    }
  }

  if (br_found) {
    std::string brLocation =
        brpoint.second + ":" + std::to_string(brpoint.first);
    throw std::logic_error("Breakpoint is already set at: " + brLocation);
  }

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

  // Add a new breakpoint to the list
  if (br_found != true) {
    breakpoints.push_back(
        std::make_pair(brpoint.second, std::vector<int>(1, brpoint.first)));
  }
}
} // namespace pdb
