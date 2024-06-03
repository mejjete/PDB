#include "PDB.hpp"

#include <sys/ioctl.h>

PDBProcess::PDBProcess()
{
    char tmp_read_file[] = "/tmp/pdbpipeXXXXXXX";
    char tmp_write_file[] = "/tmp/pdbpipeXXXXXXX";
    
    fd_read = mkstemp(tmp_read_file);
    if(fd_read < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening read pipe: " + std::string(strerror(errno)));

    fd_write = mkstemp(tmp_write_file);
    if(fd_write < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening write pipe: " + std::string(strerror(errno)));
        
    fd_read_name = tmp_read_file;
    fd_write_name = tmp_write_file;
}

PDBProcess::~PDBProcess()
{
    unlink(fd_read_name.c_str());
    unlink(fd_write_name.c_str());
}

int PDBProcess::pollRead() const
{
    struct pollfd fds[1];
    fds[0].fd = fd_read;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 0);

    if(ret == 0)
        return ret;
    else if(ret > 0)
    {
        if (fds[0].revents & POLLIN)
        {
            int bytes_available;
            if (ioctl(fd_read, FIONREAD, &bytes_available) == -1) 
                throw std::system_error(std::error_code(errno, std::generic_category()), 
                    "PDB: error ioctl process handler: " + std::string(strerror(errno)));
            
            ret = bytes_available;
        }
    }
    else 
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "PDB: poll error: " + std::string(strerror(errno)));
        
    return ret;
}
