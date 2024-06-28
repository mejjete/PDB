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
#include <list>

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
         *  Container represents an asynchronous stream from a read-end pipe. Among all, 
         *  PDBProcess is also responsible for reading and writting commands to communication 
         *  channels. This class solves this problem.
         *  
         *  It safely fetches input data from channel and provides interface for reading and writting.
         *  Also, it stringifies incoming data.
         */
        class StreamBuffer
        {
        private:
            std::mutex mut;

            // First-in first-out buffer 
            std::list<std::string> stream_buffer;

        public:
            StreamBuffer() {};

            // Number of lines ready to be fetched with get() method
            size_t size() const { return stream_buffer.size(); };
            
            // Returns 1 string per 1 call. If no string is available to read, returns ""
            std::string get();

            /**
             *  @param str - raw string
             *  Separates input string str by newline character and adds each string
             *  to FIFO buffer stream_buffer.
             */
            void add(std::string str);
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
        size_t size() const { return buffer->size(); };

    protected:
        // Issues a read from a process read-end pipe
        std::string read();

        // Issues a write to a process write-end pipe
        void write(std::string);
    };
}
