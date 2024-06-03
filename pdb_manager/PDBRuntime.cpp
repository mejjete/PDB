/**
 *  PDB profiling source file which is compiled along with a
 *  main executable to grand runtime support for message-passing
 *  and debug options.
 * 
 *  The following runtime support is for Open MPI.
 *  Any debug-side communication is going through a named pipe.
 *  Where each process has 2 pipes, 1 read only - redirected to STDINT,
 *  and 1 write only - redirected to STDOUT.
*/

#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Intercept MPI_Init
int MPI_Init(int *argc, char ***argv) 
{
    // After this call is completed, the whole MPI runtime is set up and it is save to call MPI_Comm_size
    int init_code = PMPI_Init(argc, argv);

    int mpi_rank, mpi_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    char **argv_array = *argv;
    int pdb_argc_start = *argc - (mpi_size * 2);
    int pdb_proc_argc = pdb_argc_start + (mpi_rank * 2);

    int fd_write = open(argv_array[pdb_proc_argc], O_WRONLY);
    if(fd_write < 0)
    {
        perror("Can't open PDB debug pipe for writting\n");
        MPI_Abort(MPI_COMM_WORLD, errno);
    }

    int fd_read = open(argv_array[pdb_proc_argc + 1], O_RDONLY);
    if(fd_read < 0)
    {
        perror("Can't open PDB debug pipe for reading\n");
        MPI_Abort(MPI_COMM_WORLD, errno);
    }

    if(dup2(fd_write, STDOUT_FILENO) < 0)
    {
        perror("Can't substitute STDOUT");
        MPI_Abort(MPI_COMM_WORLD, errno);
    }
    
    if(dup2(fd_read, STDIN_FILENO) < 0)
    {
        perror("Can't substitute STDIN");
        MPI_Abort(MPI_COMM_WORLD, errno);
    }

    // Make stdin and stdout line buffered
    if(setvbuf(stdin, NULL, _IOLBF, 0) != 0)
    {
        perror("setvbuf error");
        MPI_Abort(MPI_COMM_WORLD, errno);
    }

    if(setvbuf(stdout, NULL, _IONBF, 0) != 0)
    {
        perror("setvbuf error");
        MPI_Abort(MPI_COMM_WORLD, errno);
    }

    close(fd_read);
    close(fd_write);

    *argc -= mpi_size * 2;
    return init_code;
}
