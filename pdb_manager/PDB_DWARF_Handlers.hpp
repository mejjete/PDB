#pragma once

#include <boost/leaf.hpp>
#include <string>
#include <vector>

int dwarfGetSourceFiles(const std::string &exec_path,
                        std::vector<std::string> &result);

int dwarfGetFunctionLocation(const std::string &exec_path,
                             const std::string &func_name,
                             std::pair<uint64_t, std::string> &result);