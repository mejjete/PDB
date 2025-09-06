#include <DAP.hpp>
#include <PDBDebugger.hpp>
#include <boost/json.hpp>
#include <cstring>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace pdb {
const std::string LLDBDebugger::term = "\r\n\r\n";

void LLDBDebugger::openFIFO() {
  // OpenFIFO might block until its dual-opened
  PDBProcess::openFIFO();

  // Exchange a handshake with lldb-dap server to configure the environment
  Request initRequest(0, DAPCommand::Initialize);
  PDBProcess::submitCommand(initRequest.getFullMessage());

  // Receive it and check whether all is active and good upon opening
  auto contentHeader = PDBProcess::fetchByLinesUntil("\r\n\r\n");
  int contentLength = 0;

  if (contentHeader[0].find("Content-Length: ") == 0) {
    contentLength = std::stoi(contentHeader[0].substr(16));
  } else {
    throw std::runtime_error("Wrong DAP initialize request");
  }

  if (contentLength <= 0)
    throw std::runtime_error("Invalid DAP header length");

  std::string initResponse;
  initResponse.resize(contentLength);
};

std::vector<std::string> LLDBDebugger::readInput() {
  return std::vector<std::string>();
}
void LLDBDebugger::checkInput(const std::vector<std::string> &str) const {}
void LLDBDebugger::setBreakpoint(PDBbr brpoint) {}
void LLDBDebugger::startDebug(const std::string &) {}
void LLDBDebugger::endDebug() {}
} // namespace pdb
