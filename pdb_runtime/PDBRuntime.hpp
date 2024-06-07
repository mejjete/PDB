#pragma once

/**
 *  Class for providing runtime support for running processes.
 *  
 *  Actions defined as RuntimeInit and RuntimeExit will 
 *  be called process-wise and have to perform process
 *  workspace initialization
*/
template <typename Debugger>
class PDBRuntime
{
public:
    template <typename ...Args>
    PDBRuntime(Args... args);

    // Mandatory calls 
    int PDBRuntimeInit();
    int PDBRuntimeExit();

    // Non-mandatory calls
    int PDBAuxiliaryRuntimeInit();
    int PDBAuxiliaryRuntimeExit();
};
