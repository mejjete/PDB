#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>

int dwarfGetSourceFiles(const std::string &exec_path,
                        std::vector<std::string> &result) {
  // Create and initialize in-memory representation of DWARF information
  // containing in executable
  auto expected_buffer = llvm::MemoryBuffer::getFile(exec_path);
  if (!expected_buffer)
    return -1;

  auto expected_obj_file = llvm::object::ObjectFile::createObjectFile(
      expected_buffer->get()->getMemBufferRef());
  if (!expected_obj_file)
    return -1;

  auto dwarf_context = llvm::DWARFContext::create(**expected_obj_file);
  if (!dwarf_context)
    return -1;

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
        result.push_back(path);
    }
  }

  return 0;
}

int dwarfGetFunctionLocation(const std::string &exec_path,
                             const std::string &func_name,
                             std::pair<uint64_t, std::string> &result) {
  // Create and initialize in-memory representation of DWARF information
  // containing in executable
  auto expected_buffer = llvm::MemoryBuffer::getFile(exec_path);
  if (!expected_buffer)
    return -1;

  auto expected_obj_file = llvm::object::ObjectFile::createObjectFile(
      expected_buffer->get()->getMemBufferRef());
  if (!expected_obj_file)
    return -1;

  auto dwarf_context = llvm::DWARFContext::create(**expected_obj_file);
  if (!dwarf_context)
    return -1;

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
            result.first = file_line;
            result.second = file_idx;
          }
        }
      }
    }
  }

  return -1;
}
