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

  virtual std::pair<std::string, std::string> getPipeNames() const {
    return std::make_pair(fd_read_name, fd_write_name);
  };
  virtual std::pair<int, int> getPipe() const {
    return std::make_pair(fd_read, fd_write);
  };
  virtual void openFIFO();

protected:
  // Blocking read to monitor the input stream until the line tm occurs
  std::vector<std::string> fetchByLinesUntil(const std::string &tm);

  // Issues a write to a process write-end pipe
  void submitCommand(const std::string &);

private:
  int fd_read;
  int fd_write;

  boost::asio::io_context io_context;

  boost::asio::posix::stream_descriptor fd_read_desc;
  boost::asio::posix::stream_descriptor fd_write_desc;

  boost::asio::streambuf local_buffer;
  boost::asio::streambuf global_buffer;
  std::mutex buffer_mutex;

  // File names for named pipes
  std::string fd_read_name;
  std::string fd_write_name;

  std::thread reader;
};
} // namespace pdb