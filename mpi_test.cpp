/**
 *  Just a test MPI program which just echoes stdin to stdout
*/

#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) 
{
    MPI_Init(&argc, &argv);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char buff[4096] = {0};
    while(1)
    {
        int nbytes = read(STDIN_FILENO, buff, 4096);
        buff[nbytes] = 0;

        if(nbytes > 0)
            write(STDOUT_FILENO, buff, nbytes);
    }

    MPI_Finalize();
}