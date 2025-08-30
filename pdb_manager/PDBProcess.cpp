#include <PDBProcess.hpp>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <system_error>
#include <vector>

namespace pdb {
PDBProcess::PDBProcess() : fd_read_desc(io_context), fd_write_desc(io_context) {
  char tmp_read_file[] = "/tmp/pdbpipeXXXXXXX";
  char tmp_write_file[] = "/tmp/pdbpipeXXXXXXX";

  fd_read = ::mkstemp(tmp_read_file);
  if (fd_read < 0)
    throw std::runtime_error("Error creating temporal read pipe");

  fd_write = ::mkstemp(tmp_write_file);
  if (fd_write < 0)
    throw std::runtime_error("Error creating temporal write pipe");

  ::close(fd_read);
  ::close(fd_write);

  ::unlink(tmp_read_file);
  ::unlink(tmp_write_file);

  if (::mkfifo(tmp_read_file, 0666) < 0)
    throw std::runtime_error("Error creating read pipe");

  if (::mkfifo(tmp_write_file, 0666) < 0)
    throw std::runtime_error("Error creating write pipe");

  // Set them to NULL and open lately, we don't want to block here upon call to
  // open()
  fd_read = fd_write = 0;

  fd_read_name = tmp_read_file;
  fd_write_name = tmp_write_file;
}

PDBProcess::~PDBProcess() {
  io_context.stop();
  reader.join();

  ::unlink(fd_read_name.c_str());
  ::unlink(fd_write_name.c_str());
}

void PDBProcess::openFIFO() {
  fd_read = ::open(fd_read_name.c_str(), O_RDONLY);
  if (fd_read < 0)
    throw std::runtime_error("Error opening read-end pipe");

  fd_write = ::open(fd_write_name.c_str(), O_WRONLY);
  if (fd_write < 0)
    throw std::runtime_error("Error opening write-end pipe");

  fd_read_desc.assign(fd_read);
  fd_write_desc.assign(fd_write);

  // Submit a work
  reader = std::thread([this]() {
    // Callback to be called whenever we pipe is available for read
    std::function<void(boost::system::error_code, std::size_t)>
        async_read_callback = [&](boost::system::error_code ec, std::size_t n) {
          if (!ec && n > 0) {
            std::string line(local_buffer.begin(), local_buffer.begin() + n);
            if (line.length() > 0)
              read_queue.push(line);
          } else if (ec == boost::asio::error::eof) {
            ; //
          }

          // Read data again or quit
          fd_read_desc.async_read_some(boost::asio::buffer(local_buffer),
                                       async_read_callback);
        };

    // Let it go
    fd_read_desc.async_read_some(boost::asio::buffer(local_buffer),
                                 async_read_callback);
    this->io_context.run();
  });
}

void PDBProcess::submitCommand(const std::string &msg) {
  fd_write_desc.write_some(boost::asio::buffer(msg));
}

std::vector<std::string> PDBProcess::fetchByLinesUntil(const std::string &tm) {
  std::vector<std::string> result;
  auto terminal_pos = std::string::npos;

  do {
    std::string temp;
    auto current_line = read_queue.pull();
    std::stringstream sstream(current_line);

    while (std::getline(sstream, temp, '\n') &&
           terminal_pos == std::string::npos) {
      result.push_back(temp);
      terminal_pos = temp.find(tm);
    }
  } while (terminal_pos == std::string::npos);

  return result;
}
} // namespace pdb