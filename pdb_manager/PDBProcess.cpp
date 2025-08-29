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
    std::terminate();

  fd_write = ::mkstemp(tmp_write_file);
  if (fd_write < 0)
    std::terminate();

  if ((fd_read < 0) || (fd_write < 0))
    std::terminate();

  ::close(fd_read);
  ::close(fd_write);

  ::unlink(tmp_read_file);
  ::unlink(tmp_write_file);

  if (::mkfifo(tmp_read_file, 0666) < 0)
    std::terminate();

  if (::mkfifo(tmp_write_file, 0666) < 0)
    std::terminate();

  // Set them to NULL and open lately, we don't want to block here upon call to
  // open()
  fd_read = fd_write = 0;

  fd_read_name = tmp_read_file;
  fd_write_name = tmp_write_file;
}

PDBProcess::~PDBProcess() {
  if (fd_read > 0)
    ::close(fd_read);
  if (fd_write > 0)
    ::close(fd_write);

  ::unlink(fd_read_name.c_str());
  ::unlink(fd_write_name.c_str());
}

int PDBProcess::openFIFO() {
  fd_read = ::open(fd_read_name.c_str(), O_RDONLY);
  if (fd_read < 0)
    return fd_read;

  fd_write = ::open(fd_write_name.c_str(), O_WRONLY);
  if (fd_write < 0)
    return fd_write;

  fd_read_desc.assign(fd_read);
  fd_write_desc.assign(fd_write);

  // Submit a work
  reader = std::thread([this]() {
    this->fd_read_desc.async_read_some(
        boost::asio::buffer(this->local_buffer),
        [this](boost::system::error_code ec, std::size_t n) {
          if (!ec && n > 0) {
            std::string line(local_buffer.begin(), local_buffer.end());
            read_queue.push(line);
          } else if (ec == boost::asio::error::eof) {
            std::cout << "Read complete" << std::endl;
          } else if (ec) {
            std::cerr << "Read error: " << ec.message() << std::endl;
          }
        });
  });

  child = std::thread([this]() { this->io_context.run(); });
  return 0;
}

void PDBProcess::write(const std::string &msg) { return; }

std::string PDBProcess::read() { std::string(""); };
} // namespace pdb