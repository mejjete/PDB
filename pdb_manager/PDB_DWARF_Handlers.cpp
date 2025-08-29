#include <boost/leaf.hpp>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>

boost::leaf::result<std::vector<std::string>>
dwarfGetSourceFiles(const std::string &exec_path) {
  // Create and initialize in-memory representation of DWARF information
  // containing in executable
  auto expected_buffer = llvm::MemoryBuffer::getFile(exec_path);
  if (!expected_buffer)
    return boost::leaf::new_error<std::string>("Error reading executable");

  auto expected_obj_file = llvm::object::ObjectFile::createObjectFile(
      expected_buffer->get()->getMemBufferRef());
  if (!expected_obj_file)
    return boost::leaf::new_error<std::string>(
        "Error creating in-memory executable object");

  auto dwarf_context = llvm::DWARFContext::create(**expected_obj_file);
  if (!dwarf_context)
    return boost::leaf::new_error<std::string>(
        "Error initializing DWARF information");

  std::vector<std::string> files;

  for (const auto &CU : dwarf_context->compile_units()) {
    if (!CU)
      continue;
    const auto *lt = dwarf_context->getLineTableForUnit(CU.get());
    if (!lt)
      continue;

    for (const auto &entry : lt->Prologue.FileNames) {
      std::string path;

      if (entry.DirIdx > 0 &&
          entry.DirIdx <= lt->Prologue.IncludeDirectories.size()) {
        if (auto dir = lt->Prologue.IncludeDirectories[entry.DirIdx - 1]
                           .getAsCString())
          path = *dir;
        path += "/";
      }

      if (auto name = entry.Name.getAsCString())
        path += *name;

      if (!path.empty())
        files.push_back(path);
    }
  }

  return files;
}

boost::leaf::result<std::pair<uint64_t, std::string>>
dwarfGetFunctionLocation(const std::string &exec_path,
                         const std::string &func_name) {
  // Create and initialize in-memory representation of DWARF information
  // containing in executable
  auto expected_buffer = llvm::MemoryBuffer::getFile(exec_path);
  if (!expected_buffer)
    return boost::leaf::new_error<std::string>("Error reading executable");

  auto expected_obj_file = llvm::object::ObjectFile::createObjectFile(
      expected_buffer->get()->getMemBufferRef());
  if (!expected_obj_file)
    return boost::leaf::new_error<std::string>(
        "Error creating in-memory executable object");

  auto dwarf_context = llvm::DWARFContext::create(**expected_obj_file);
  if (!dwarf_context)
    return boost::leaf::new_error<std::string>(
        "Error initializing DWARF information");

  for (const auto &CU : dwarf_context->compile_units()) {
    for (const auto &entry : CU->dies()) {
      llvm::DWARFDie die(CU.get(), &entry);

      if (die.getTag() == llvm::dwarf::DW_TAG_subprogram) {
        if (const char *name = die.getName(llvm::DINameKind::ShortName)) {
          if (func_name == name) {
            // Obtain function source file
            std::string file_idx = die.getDeclFile(
                llvm::DILineInfoSpecifier::FileLineInfoKind::RawValue);
            // Obtain function line number
            uint64_t file_line = die.getDeclLine();
            return std::make_pair(file_line, file_idx);
          }
        }
      }
    }
  }

  return boost::leaf::new_error<std::string>("Unknown function: " + func_name);
}
