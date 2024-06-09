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
        
    close(fd_read);
    close(fd_write);

    unlink(tmp_read_file);
    unlink(tmp_write_file);

    if(mkfifo(tmp_read_file, 0666) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening input fifo: " + std::string(strerror(errno)));

    if(mkfifo(tmp_write_file, 0666) < 0)
        throw std::system_error(std::error_code(errno, std::generic_category()), 
            "Error opening output fifo: " + std::string(strerror(errno)));

    // Set them to NULL and open lately, we don't want to block here upon call to open()
    fd_read = fd_write = 0;

    if(fd_read < 0 | fd_write < 0)
        throw std::runtime_error("Can't open named pipes");
    
    fd_read_name = tmp_read_file;
    fd_write_name = tmp_write_file;
}

PDBProcess::~PDBProcess()
{
    if(fd_read > 0)
        close(fd_read);
    if(fd_write > 0)
        close(fd_write);

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

int PDBProcess::openFIFO()
{
    fd_read = open(fd_read_name.c_str(), O_RDONLY);
    if(fd_read < 0)
        return fd_read;

    fd_write = open(fd_write_name.c_str(), O_WRONLY);
    if(fd_write < 0)
        return fd_write;
    
    return 0;
}

