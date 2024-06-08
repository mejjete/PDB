/**
 *  Additional hooks for obtaining valuable information about Open MPI runtime.
 *  Build into shared which is specified in LD_PRELOAD.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <iostream>
#include <mpi.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function pointer for the original MPI_Init function
typedef int (*MPI_Init_t)(int *, char ***);
MPI_Init_t original_MPI_Init = nullptr;

// Wrapper for MPI_Init
int MPI_Init(int *argc, char ***argv) 
{
    int result = PMPI_Init(argc, argv);

    // Get the original MPI_Init function address
    if (!original_MPI_Init) {
        original_MPI_Init = (MPI_Init_t) dlsym(RTLD_NEXT, "MPI_Init");
        if (!original_MPI_Init) {
            fprintf(stderr, "Error loading original MPI_Init: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
    }

    /**
     *  Get the process-related handler to MPI_COMM_WORLD
     *  The following code is highly unportable and should work only for OpenMPI
     */
    MPI_Comm comm_world = (MPI_Comm) dlsym(RTLD_DEFAULT, "MPI_COMM_WORLD");

    int mpi_rank;
    MPI_Comm_rank(comm_world, &mpi_rank);

    // Do some preparation

    return result;
}