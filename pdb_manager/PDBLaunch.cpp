/**
 *  Auxiliary helper which set up the PDB runtime for each process 
 */
#include "PDB.hpp"
#include <cstdio>

std::pair<int, int> readPipeNames(const char *temp_file, int proc_num)
{
    int fd = open(temp_file, O_RDWR);
    if(fd < 0)
    {
        printf("Error opening file\n");
        exit(0);
    }

    if(flock(fd, LOCK_EX) < 0)
    {
        printf("Error locking file\n");
        exit(0);
    }

    std::vector<char> file(4096);
    std::vector<char> in_pipe(PDB_PIPE_LENGTH);
    std::vector<char> out_pipe(PDB_PIPE_LENGTH);
 
    int nbytes;
    if((nbytes = read(fd, file.data(), 4096)) < 0)
    {
        printf("Can't read temporal file\n");
        exit(0);
    }

    std::copy(file.begin(), file.begin() + PDB_PIPE_LENGTH, in_pipe.begin());
    std::copy(file.begin() + PDB_PIPE_LENGTH, file.begin() + PDB_PIPE_LENGTH * 2, out_pipe.begin());

    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    if(write(fd, file.data() + (PDB_PIPE_LENGTH * 2), nbytes - (PDB_PIPE_LENGTH * 2)) < 0)
    {
        printf("Can't write temporal file\n");
        exit(0);   
    }

    flock(fd, LOCK_UN);
    close(fd);

    printf("Input file: %s\n", in_pipe.data());
    printf("Output file: %s\n", out_pipe.data());
    return std::make_pair(0, 0);
}

int main(int argc, char **argv)
{
    // The last 2 parameters in the current scheme would be process specific arguments
    // for(int i = 1; i < argc; i++)
    //     printf("%s ", argv[i]);
    // printf("\n");

    int proc_num = atoi(argv[argc - 1]);
    auto pipes = readPipeNames(argv[argc - 2], proc_num);

    printf("\n");
    return 0;
}