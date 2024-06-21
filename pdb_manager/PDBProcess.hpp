#pragma once 

#include <string>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace pdb
{
    /**
     *  Process handler that creates connections to spawned processes.
     *  Does not spawn any process by itself.
    */
    class PDBProcess
    {
    private:
        int fd_read;
        int fd_write;

        // File names for named pipes
        std::string fd_read_name;
        std::string fd_write_name;

    public:
        PDBProcess();

        PDBProcess(const PDBProcess&) = delete;
        PDBProcess(PDBProcess&&) = default;
        ~PDBProcess();

        int pollRead() const;
        int openFIFO();

        // Issues a read from a process read-end pipe
        std::string read();

        // Issues a write to a process write-end pipe and wait for responce
        void write(std::string);

        std::pair<std::string, std::string> getPipeNames() const { return std::make_pair(fd_read_name, fd_write_name); };
        std::pair<int, int> getPipe() const { return std::make_pair(fd_read, fd_write); };
    };
}
