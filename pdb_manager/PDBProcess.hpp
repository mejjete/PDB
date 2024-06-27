#pragma once 

#include <string>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sstream>
#include <mutex>
#include <thread>
#include <memory>

namespace pdb
{
    /**
     *  Process handler that creates connections to spawned processes.
     *  Does not spawn any process by itself.
    */
    class PDBProcess
    {
    public:
        
        /**
         *  
         */
        class StreamBuffer
        {
        private:
            std::mutex mut;
            std::stringstream stream;
            size_t stream_size;

        public:
            StreamBuffer() : stream_size(0) {};
            std::string get();
            void add(std::string);
        };

        /**
         * 
         */
        class InputBuffer : protected StreamBuffer
        {
        public:
            InputBuffer() : StreamBuffer() {};
            std::string get() { return StreamBuffer::get(); };
        };

    private:
        int fd_read;
        int fd_write;

        // Protects read-end and write-end pipes
        std::mutex file_mut;

        // Data stream
        std::shared_ptr<StreamBuffer> buffer;

        // File names for named pipes
        std::string fd_read_name;
        std::string fd_write_name;

        // Thread that constantly polls read-end pipe to check if it has something to read
        std::thread thread;

    public:
        PDBProcess();
        PDBProcess(const PDBProcess&) = delete;
        PDBProcess(PDBProcess&&) = default;
        ~PDBProcess();

        std::pair<std::string, std::string> getPipeNames() const { return std::make_pair(fd_read_name, fd_write_name); };
        std::pair<int, int> getPipe() const { return std::make_pair(fd_read, fd_write); };
        int openFIFO();
        int pollRead() const;

    protected:
        // Issues a read from a process read-end pipe
        std::string read();

        // Issues a write to a process write-end pipe
        void write(std::string);
    };
}
