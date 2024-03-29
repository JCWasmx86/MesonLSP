#pragma once

#include "lsptypes.hpp"
#include "mesonmetadata.hpp"
#include "node.hpp"
#include "polyfill.hpp"

#include <ada.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

inline std::filesystem::path extractPathFromUrl(const std::string &urlStr) {
  auto url = ada::parse<ada::url>(urlStr);
  if (!url) {
    throw std::runtime_error(std::format("Unknown URL: {}", urlStr));
  }
  if (url->get_protocol() != "file:") {
    throw std::runtime_error(
        std::format("Unknown protocol: {}", url->get_protocol()));
  }
  auto input = url->get_pathname();
  auto ret = ada::unicode::percent_decode(input, input.find('%'));
#ifdef _WIN32
  // For Windows: We get e.g. /D:/a/...
  // We will transform it to D:/a/...
  if (ret.size() > 3 && ret[0] == '/' && ret[2] == ':') {
    ret = ret.substr(1);
  }
#endif
  return {ret};
}

inline std::string pathToUrl(const std::filesystem::path &path) {
  auto ret = ada::href_from_file(path.generic_string());
  return ret;
}

inline LSPDiagnostic makeLSPDiagnostic(const Diagnostic &diag) {
  auto range = LSPRange(LSPPosition(diag.startLine, diag.startColumn),
                        LSPPosition(diag.endLine, diag.endColumn));
  auto severity = diag.severity == Severity::ERROR
                      ? DiagnosticSeverity::LSP_ERROR
                      : DiagnosticSeverity::LSP_WARNING;
  std::vector<DiagnosticTag> tags;
  if (diag.deprecated) {
    tags.push_back(DiagnosticTag::LSP_DEPRECATED);
  }
  if (diag.unnecessary) {
    tags.push_back(DiagnosticTag::LSP_UNNECESSARY);
  }
  return {range, severity, diag.message, tags};
}

inline LSPRange nodeToRange(const Node *node) {
  const auto &loc = node->location;
  return {LSPPosition(loc.startLine, loc.startColumn),
          LSPPosition(loc.endLine, loc.endColumn)};
}
