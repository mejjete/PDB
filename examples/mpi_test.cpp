/**
 *  Just a test MPI program which prints its rank
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
    printf("I am process: %d\n", rank);
    MPI_Finalize();
}