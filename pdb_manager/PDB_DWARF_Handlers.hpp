#pragma once

#include <boost/leaf.hpp>
#include <string>
#include <vector>

boost::leaf::result<std::vector<std::string>>
dwarfGetSourceFiles(const std::string &exec_path);

boost::leaf::result<std::pair<std::size_t, std::string>>
dwarfGetFunctionLocation(const std::string &exec_path,
                         const std::string &func_name);