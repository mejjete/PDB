#include "PDBProcess.hpp"
#include <stdexcept>
#include <system_error>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <poll.h>

namespace pdb 
{
    PDBProcess::PDBProcess()
    {
        char tmp_read_file[] = "/tmp/pdbpipeXXXXXXX";
        char tmp_write_file[] = "/tmp/pdbpipeXXXXXXX";
        
        fd_read = mkstemp(tmp_read_file);
        if(fd_read < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error opening read pipe: ");

        fd_write = mkstemp(tmp_write_file);
        if(fd_write < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error opening write pipe: ");
            
        close(fd_read);
        close(fd_write);

        unlink(tmp_read_file);
        unlink(tmp_write_file);

        if(mkfifo(tmp_read_file, 0666) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error opening input fifo: ");

        if(mkfifo(tmp_write_file, 0666) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "Error opening output fifo: ");

        // Set them to NULL and open lately, we don't want to block here upon call to open()
        fd_read = fd_write = 0;

        if((fd_read < 0) | (fd_write < 0))
            throw std::runtime_error("Can't open named pipes");
        
        fd_read_name = tmp_read_file;
        fd_write_name = tmp_write_file;

        buffer = std::make_shared<StreamBuffer>();
    }

    PDBProcess::~PDBProcess()
    {
        if(thread.joinable())
            thread.join();

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
            if(fds[0].revents & POLLIN)
            {
                int bytes_available;
                if (ioctl(fd_read, FIONREAD, &bytes_available) == -1) 
                    throw std::system_error(std::error_code(errno, std::generic_category()), 
                        "PDB: error ioctl process handler: ");
                
                ret = bytes_available;
            }
        }
        else 
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "PDB: poll error: ");
            
        return ret;
    }

    void monitor(int fd, std::mutex &file_mut, std::shared_ptr<PDBProcess::StreamBuffer> buffer)
    {
        struct pollfd fds[1];
        fds[0].fd = fd;
        fds[0].events = POLLIN;

        int ret = poll(fds, 1, 0);
        int result;

        while(true)
        {
            if(ret > 0)
            {
                if(fds[0].revents & POLLIN)
                {
                    int bytes_available;
                    if(ioctl(fd, FIONREAD, &bytes_available) == -1) 
                        throw std::system_error(std::error_code(errno, std::generic_category()), 
                            "ioctl error");
                    
                    result = bytes_available;
                }
            }
            else 
                result = 0;

            // Read whatever we have on descriptor fd to shared buffer
            if(result > 0)
            {   
                std::lock_guard<std::mutex> lock(file_mut);
                std::vector<char> to_read(result);
                if(::read(fd, to_read.data(), result) < 0)
                    throw std::system_error(std::error_code(errno, std::generic_category()), 
                        "PDB: error reading pipe");
        
                std::string read_str(to_read.begin(), to_read.end());
                buffer->add(read_str);
            }            

            usleep(1000);
        }
    }

    int PDBProcess::openFIFO()
    {
        fd_read = open(fd_read_name.c_str(), O_RDONLY);
        if(fd_read < 0)
            return fd_read;

        fd_write = open(fd_write_name.c_str(), O_WRONLY);
        if(fd_write < 0)
            return fd_write;
        
        thread = std::thread(monitor, fd_read, std::ref(file_mut), buffer);
        return 0;
    }

    void PDBProcess::write(std::string msg)
    {
        if(msg.length() == 0)
            return;

        if(::write(fd_write, msg.c_str(), msg.length()) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "PDB: error writting to pipe");
    }

    std::string PDBProcess::read()
    {
        // Wait until we have something to read
        int nbytes;
        while((nbytes = pollRead()) == 0)
            ;

        std::vector<char> to_read(nbytes);
        if(::read(fd_read, to_read.data(), nbytes) < 0)
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                "PDB: error reading pipe");
        
        std::string result(to_read.begin(), to_read.end());
        return result;
    }

    void PDBProcess::StreamBuffer::add(std::string str)
    {
        if(str.length() == 0)
            return;

        std::lock_guard<std::mutex> lock(mut);
        stream << str;
        stream_size += str.length();
    }

    std::string PDBProcess::StreamBuffer::get()
    {
        std::lock_guard<std::mutex> lock(mut);
        std::string result;
        stream >> result;
        return result;
        // std::vector<char> vect_array;
        // vect_array.resize(stream_size);
        // stream.read(vect_array.data(), stream_size);
    }
}
