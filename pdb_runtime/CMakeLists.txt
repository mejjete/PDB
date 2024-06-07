project(PDB)

# Cmake contatining rules for building custom runtime library
# The following CMake is default choice and made for Open MPI PDB runtime
add_library(pdb_runtime SHARED 
	PDBRuntime.cpp)
