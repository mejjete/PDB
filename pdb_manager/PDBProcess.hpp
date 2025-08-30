#pragma once

#include <boost/asio.hpp>
#include <boost/leaf.hpp>
#include <boost/thread/sync_queue.hpp>
#include <list>
#include <memory>
#include <string>
#include <unistd.h>
#include <utility>

namespace pdb {
/**
 * Process handler that creates connections to spawned processes.
 * Does not spawn any process by itself.
 */
class PDBProcess : std::enable_shared_from_this<PDBProcess> {
public:
  PDBProcess();
  PDBProcess(const PDBProcess &) = delete;
  PDBProcess(PDBProcess &&) = default;
  ~PDBProcess();

  std::pair<std::string, std::string> getPipeNames() const {
    return std::make_pair(fd_read_name, fd_write_name);
  };
  std::pair<int, int> getPipe() const {
    return std::make_pair(fd_read, fd_write);
  };
  void openFIFO();

protected:
  // Read a read-end pipe until tm
  std::vector<std::string> fetchByLinesUntil(const std::string &tm);

  // Issues a write to a process write-end pipe
  void submitCommand(const std::string &);

  boost::sync_queue<std::string> read_queue;

private:
  int fd_read;
  int fd_write;

  boost::asio::io_context io_context;

  boost::asio::posix::stream_descriptor fd_read_desc;
  boost::asio::posix::stream_descriptor fd_write_desc;
  std::array<char, 4096> local_buffer;

  // File names for named pipes
  std::string fd_read_name;
  std::string fd_write_name;

  std::thread reader;
};
} // namespace pdb