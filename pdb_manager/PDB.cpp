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
    PDBDebug pdb_instance("mpicxx mpi_test.c -o mpi_test.out", "mpirun -np 4 ./mpi_test.out arg1");
    size_t pdb_size = pdb_instance.size();

    while(!is_signaled)
    {
        printf("Message: ");

        std::string message;   
        getline(std::cin, message);
        message += "\n";

        for(size_t i = 0; i < pdb_size; i++)
        {
            auto pipe = pdb_instance.getProc(i).getPipe();
            write(pipe.second, message.c_str(), message.length());
        }

        for(size_t i = 0; i < pdb_size; i++)
        {
            char buff[4096];
            int nbytes;
            auto pipe = pdb_instance.getProc(i).getPipe();

            // Wait until we have something to read
            while((nbytes = read(pipe.first, buff, 4096)) <= 0)
                ;
            
            buff[nbytes] = 0;
            printf("%ld responded: %s\n", i, buff);
            usleep(10000);
        }
    }

    return 0;
}
