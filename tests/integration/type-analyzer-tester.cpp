#include "analysisoptions.hpp"
#include "log.hpp"
#include "mesontree.hpp"
#include "polyfill.hpp"
#include "typeanalyzer.hpp"
#include "typenamespace.hpp"

#include <cassert>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <map>
#include <optional>
#include <string>
#include <system_error>
#include <unistd.h>

int main(int argc, char **argv) {
  Logger const logger("type-analyzer-tester");
  std::filesystem::path const toParse = argv[1];
  std::map<std::string, std::string> haveToExist;
  std::optional<std::filesystem::path> optionsFile;
  for (int i = 2; i < argc; i += 2) {
    if (strcmp(argv[i], "--") == 0) {
      assert(i + 1 < argc);
      optionsFile = argv[i + 1];
      break;
    }
    haveToExist[argv[i]] = argv[i + 1];
  }
#ifndef __APPLE__
  std::filesystem::path const parent(
      std::format("{}/type-analyzer{:%F%H%I%M}-{}",
                  std::filesystem::temp_directory_path().generic_string(),
                  std::chrono::system_clock::now(), getpid()));
#else
  std::filesystem::path const parent(std::format(
      "/tmp/type-analyzer{}-{}", std::chrono::system_clock::now(), getpid()));
#endif
  std::filesystem::create_directories(parent);
  TypeNamespace const ns;
  std::error_code err;
  logger.info(std::format("Copying {} -> {}", toParse.generic_string(),
                          (parent / "meson.build").generic_string()));
  std::filesystem::copy_file(toParse, parent / "meson.build",
                             std::filesystem::copy_options::overwrite_existing,
                             err);
  if (err != std::errc{}) {
    logger.error(err.message());
  }
  assert(err == std::errc{});
  if (optionsFile.has_value()) {
    logger.info(std::format("Copying {} -> {}", optionsFile->generic_string(),
                            (parent / "meson.options").generic_string()));
    std::filesystem::copy_file(
        optionsFile.value(), parent / "meson.options",
        std::filesystem::copy_options::overwrite_existing, err);
  }
  MesonTree tree(parent, ns);
  AnalysisOptions const opts(false, false, false, false, false, false, false);
  tree.partialParse(opts);
  const auto &metadata = tree.scope.variables;
  auto errors = 0;
  for (const auto &expectedPair : haveToExist) {
    if (!metadata.contains(expectedPair.first)) {
      logger.error("Failed to find variable " + expectedPair.first);
      errors++;
      continue;
    }
    const auto &referenceTypes = metadata.at(expectedPair.first);
    const auto &referenceString = joinTypes(referenceTypes);
    if (referenceString != expectedPair.second) {
      logger.error(std::format("Expected types {} for variable {}, but got {}",
                               expectedPair.second, expectedPair.first,
                               referenceString));
      errors++;
      continue;
    }
  }
  if (errors != 0) {
    logger.error(std::format("{} errors", errors));
    logger.error(" === SCOPE === ");
    for (const auto &[varname, types] : metadata) {
      logger.error(std::format("{} = {}", varname, joinTypes(types)));
    }
  }
  return static_cast<int>(errors != 0);
}
