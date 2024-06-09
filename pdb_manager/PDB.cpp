#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include "PDB.hpp"

bool is_signaled = false;

int main()
{
    PDBDebug pdb_instance("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3");
    size_t pdb_size = pdb_instance.size();

    while(!is_signaled)
    {
        for(size_t i = 0; i < pdb_size; i++)
        {
            std::string main_buffer;

            char buff[4096];
            int nbytes;
            auto pipe = pdb_instance.getProc(i).getPipe();

            // Wait until we have something to read
            while((nbytes = read(pipe.first, buff, 4096)) > 0)
            {
                buff[nbytes] = 0;
                main_buffer += buff;

                if(strstr(buff, "(gdb)") != NULL)
                    break;
            }

            printf("%ld responded: %s\n", i, main_buffer.c_str()); 
            usleep(10000);
        }

        printf("Command: ");

        std::string message;   
        getline(std::cin, message);
        message += "\n";

        for(size_t i = 0; i < pdb_size; i++)
        {
            auto pipe = pdb_instance.getProc(i).getPipe();
            
            if(write(pipe.second, message.c_str(), message.length()) < 0)
                printf("Error writing to a file\n");
        }
    }

    return 0;
}
