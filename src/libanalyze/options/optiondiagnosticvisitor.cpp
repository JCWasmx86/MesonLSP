#include "optiondiagnosticvisitor.hpp"

#include "log.hpp"
#include "mesonmetadata.hpp"
#include "node.hpp"
#include "optionstate.hpp"

#include <cctype>
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>

const static Logger LOG("OptionDiagnosticVisitor"); // NOLINT
static const OptionState STATE;                     // NOLINT

void OptionDiagnosticVisitor::visitArgumentList(ArgumentList *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitArrayLiteral(ArrayLiteral *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitAssignmentStatement(
    AssignmentStatement *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitBinaryExpression(BinaryExpression *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitBooleanLiteral(BooleanLiteral *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitBuildDefinition(BuildDefinition *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitConditionalExpression(
    ConditionalExpression *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitDictionaryLiteral(DictionaryLiteral *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::checkName(const StringLiteral *sl) {
  const auto &contents = sl->id;
  if (this->options.contains(contents)) {
    this->metadata->registerDiagnostic(
        sl, Diagnostic(Severity::ERROR, sl, "Duplicate option: " + contents));
  }
  if (STATE.findOption(contents)) {
    this->metadata->registerDiagnostic(
        sl, Diagnostic(Severity::ERROR, sl,
                       "Declaration of reserved option: " + contents));
  }
  this->options.insert(contents);
  for (const auto chr : contents) {
    if ((std::isalnum(chr) != 0) || chr == '_' || chr == '-') {
      continue;
    }
    this->metadata->registerDiagnostic(
        sl,
        Diagnostic(
            Severity::ERROR, sl,
            "Invalid chars in name: Expected `a-z`, `A-Z`, `0-9`, `-` or `_`"));
    return;
  }
}

void OptionDiagnosticVisitor::validateStringOption(
    const Node *defaultValue) const {
  if (dynamic_cast<const StringLiteral *>(defaultValue)) {
    return;
  }
  this->metadata->registerDiagnostic(
      defaultValue,
      Diagnostic(Severity::ERROR, defaultValue, "Expected string literal"));
}

std::optional<int64_t>
OptionDiagnosticVisitor::parseString(const Node *node) const {
  const auto *asSL = dynamic_cast<const StringLiteral *>(node);
  if (!asSL) {
    return std::nullopt;
  }
  const auto &value = asSL->id;
  if (value.empty()) {
    return std::nullopt;
  }
  try {
    int64_t ret = 0;
    if (value.starts_with("0x") || value.starts_with("0X")) {
      ret = std::stoll(value, nullptr, 16);
    } else if (value.starts_with("0b") || value.starts_with("0B")) {
      ret = std::stoll(value.substr(2), nullptr, 2);
    } else if (value.starts_with("0O") || value.starts_with("0o")) {
      ret = std::stoll(value.substr(2), nullptr, 8);
    } else {
      ret = std::stoll(value);
    }
    this->metadata->registerDiagnostic(
        node, Diagnostic(Severity::WARNING, node,
                         "String literals as value where integers are "
                         "expected, are deprecated"));
    return ret;
  } catch (...) {
    this->metadata->registerDiagnostic(
        node, Diagnostic(Severity::ERROR, node, "Unable to parse as integer"));
  }
  return std::nullopt;
}

std::optional<int64_t> OptionDiagnosticVisitor::parseInt(const Node *node) {
  if (const auto *integerLit = dynamic_cast<const IntegerLiteral *>(node)) {
    return integerLit->valueAsInt;
  }
  if (const auto *unaryExpr = dynamic_cast<const UnaryExpression *>(node)) {
    if (unaryExpr->op != UnaryOperator::UNARY_MINUS) {
      return std::nullopt;
    }
    auto parsed = this->parseInt(unaryExpr->expression.get());
    if (parsed.has_value()) {
      return -parsed.value();
    }
  }
  return std::nullopt;
}

void OptionDiagnosticVisitor::validateBooleanOption(
    const Node *defaultValue) const {
  if (dynamic_cast<const BooleanLiteral *>(defaultValue)) {
    return;
  }
  const auto *sl = dynamic_cast<const StringLiteral *>(defaultValue);
  if (!sl) {
    this->metadata->registerDiagnostic(
        defaultValue, Diagnostic(Severity::ERROR, defaultValue,
                                 "Expected boolean value for boolean option"));
    return;
  }
  if (sl->id != "true" && sl->id != "false") {
    this->metadata->registerDiagnostic(
        defaultValue, Diagnostic(Severity::ERROR, defaultValue,
                                 "Expected 'true' or 'false'"));
  }
  this->metadata->registerDiagnostic(
      defaultValue,
      Diagnostic(
          Severity::WARNING, defaultValue,
          "String literals as value for boolean options are deprecated."));
}

void OptionDiagnosticVisitor::validateFeatureOption(
    const Node *defaultValue) const {
  const auto *slNode = dynamic_cast<const StringLiteral *>(defaultValue);
  if (!slNode) {
    this->metadata->registerDiagnostic(
        defaultValue,
        Diagnostic(Severity::ERROR, defaultValue, "Expected string"));
    return;
  }
  const auto &contents = slNode->id;
  if (contents == "enabled" || contents == "disabled" || contents == "auto") {
    return;
  }
  this->metadata->registerDiagnostic(
      defaultValue,
      Diagnostic(Severity::ERROR, defaultValue,
                 "Expected one of: 'enabled', 'disabled', 'auto'"));
}

void OptionDiagnosticVisitor::validateComboOption(
    const Node *defaultValue, const ArgumentList *al) const {
  const auto *defaultSL = dynamic_cast<const StringLiteral *>(defaultValue);
  std::optional<std::string> defaultString;
  if (!defaultSL) {
    this->metadata->registerDiagnostic(
        defaultValue,
        Diagnostic(Severity::ERROR, defaultValue, "Expected string literal"));
  } else {
    defaultString = defaultSL->id;
  }
  auto choicesKwarg = al->getKwarg("choices");
  if (!choicesKwarg.has_value()) {
    this->metadata->registerDiagnostic(
        al->parent,
        Diagnostic(Severity::ERROR, al->parent, "Missing 'choices' kwarg"));
    return;
  }
  std::set<std::string> foundChoices;
  const auto *choicesArray =
      dynamic_cast<ArrayLiteral *>(choicesKwarg.value().get());
  if (!choicesArray) {
    this->metadata->registerDiagnostic(
        choicesKwarg->get(), Diagnostic(Severity::ERROR, choicesKwarg->get(),
                                        "Expected array of strings"));
    return;
  }
  for (const auto &choice : choicesArray->args) {
    const auto *asLiteral = dynamic_cast<const StringLiteral *>(choice.get());
    if (!asLiteral) {
      this->metadata->registerDiagnostic(
          choice.get(),
          Diagnostic(Severity::ERROR, choice.get(), "Expected string literal"));
      continue;
    }
    const auto &content = asLiteral->id;
    if (foundChoices.contains(content)) {
      this->metadata->registerDiagnostic(
          choice.get(), Diagnostic(Severity::WARNING, choice.get(),
                                   "Duplicate choice '" + content + "'"));
      continue;
    }
    foundChoices.insert(content);
  }
  if (defaultString.has_value() &&
      !foundChoices.contains(defaultString.value())) {
    this->metadata->registerDiagnostic(
        defaultValue,
        Diagnostic(Severity::ERROR, defaultValue,
                   "Default value is not contained in the choices array."));
  }
}

void OptionDiagnosticVisitor::extractArrayChoices(
    const ArgumentList *al,
    std::shared_ptr<std::set<std::string>> *choices) const {
  auto kwarg = al->getKwarg("choices");
  if (!kwarg.has_value()) {
    *choices = nullptr;
    return;
  }
  const auto *arrayLit = dynamic_cast<const ArrayLiteral *>(kwarg->get());
  if (!arrayLit) {
    *choices = nullptr;
    this->metadata->registerDiagnostic(
        kwarg->get(),
        Diagnostic(Severity::ERROR, kwarg->get(), "Expected array literal"));
    return;
  }
  auto ret = std::set<std::string>();
  for (const auto &node : arrayLit->args) {
    const auto *asStr = dynamic_cast<const StringLiteral *>(node.get());
    if (!asStr) {
      this->metadata->registerDiagnostic(
          node.get(),
          Diagnostic(Severity::ERROR, node.get(), "Expected string literal"));
      continue;
    }
    const auto &content = asStr->id;
    if (!ret.contains(content)) {
      ret.insert(content);
      continue;
    }
    this->metadata->registerDiagnostic(
        node.get(),
        Diagnostic(Severity::WARNING, node.get(), "Duplicate choice"));
  }
  *choices = std::make_shared<std::set<std::string>>(ret);
}

void OptionDiagnosticVisitor::validateArrayOption(
    const Node *defaultValue, const ArgumentList *al) const {
  std::shared_ptr<std::set<std::string>> choices = nullptr;
  extractArrayChoices(al, &choices);
  const auto *arrLit = dynamic_cast<const ArrayLiteral *>(defaultValue);
  if (!arrLit) {
    this->metadata->registerDiagnostic(
        defaultValue,
        Diagnostic(Severity::ERROR, defaultValue, "Expected array literal"));
    return;
  }
  for (const auto &node : arrLit->args) {
    const auto *asStr = dynamic_cast<StringLiteral *>(node.get());
    if (!asStr) {
      this->metadata->registerDiagnostic(
          node.get(),
          Diagnostic(Severity::ERROR, node.get(), "Expected string literal"));
      continue;
    }
    const auto &content = asStr->id;
    if (!choices || choices->contains(content)) {
      continue;
    }
    this->metadata->registerDiagnostic(
        node.get(), Diagnostic(Severity::ERROR, node.get(),
                               "Value is not a valid choice!"));
  }
}

std::optional<int64_t>
OptionDiagnosticVisitor::fetchIntOrNullOpt(const ArgumentList *al,
                                           const std::string &kwarg) {
  const auto &kwargNode = al->getKwarg(kwarg);
  if (!kwargNode.has_value()) {
    return std::nullopt;
  }
  const auto *node = kwargNode.value().get();
  auto val = this->parseInt(node);
  if (val.has_value()) {
    return val;
  }
  val = this->parseString(node);
  if (val.has_value()) {
    return val;
  }
  this->metadata->registerDiagnostic(
      node,
      Diagnostic(Severity::ERROR, node, "Unable to parse as integer literal"));
  return std::nullopt;
}

void OptionDiagnosticVisitor::validateIntegerOption(const ArgumentList *al,
                                                    const Node *defaultValue) {
  auto defaultInt = this->parseInt(defaultValue);
  if (!defaultInt.has_value()) {
    defaultInt = this->parseString(defaultValue);
    if (!defaultInt.has_value()) {
      this->metadata->registerDiagnostic(
          defaultValue, Diagnostic(Severity::ERROR, defaultValue,
                                   "Unable to parse as integer literal"));
    }
  }
  auto minValue = this->fetchIntOrNullOpt(al, "min");
  auto maxValue = this->fetchIntOrNullOpt(al, "max");

  if (!minValue.has_value() && !maxValue.has_value() &&
      !defaultInt.has_value()) {
    return;
  }
  if (minValue.has_value() && maxValue.has_value()) {
    auto minV = minValue.value();
    auto maxV = maxValue.value();
    const auto *minKwarg = /*NOLINT*/ al->getKwarg("min").value().get();
    if (minV > maxV) {
      this->metadata->registerDiagnostic(
          minKwarg,
          Diagnostic(Severity::WARNING, minKwarg,
                     "Minimum value is greater than the maximum value"));
    } else if (minV == maxV) {
      this->metadata->registerDiagnostic(
          minKwarg, Diagnostic(Severity::WARNING, minKwarg,
                               "Minimum value is equals to the maximum value"));
    }
  }
  if (minValue.has_value() && defaultInt.has_value()) {
    auto minV = minValue.value();
    auto defaultV = defaultInt.value();
    if (minV > defaultV) {
      this->metadata->registerDiagnostic(
          defaultValue,
          Diagnostic(Severity::WARNING, defaultValue,
                     "Default value is lower than the minimum value"));
    }
  }
  if (maxValue.has_value() && defaultInt.has_value()) {
    auto maxV = maxValue.value();
    auto defaultV = defaultInt.value();
    if (maxV < defaultV) {
      this->metadata->registerDiagnostic(
          defaultValue,
          Diagnostic(Severity::WARNING, defaultValue,
                     "Default value is greater than the maximum value"));
    }
  }
}

void OptionDiagnosticVisitor::visitFunctionExpression(
    FunctionExpression *node) {
  node->visitChildren(this);
  if (node->functionName() != "option") {
    this->metadata->registerDiagnostic(
        node, Diagnostic(Severity::ERROR, node,
                         "Invalid function call in meson options file: " +
                             node->functionName()));
    return;
  }
  const auto *al = dynamic_cast<ArgumentList *>(node->args.get());
  if (!al) {
    this->metadata->registerDiagnostic(
        node, Diagnostic(Severity::ERROR, node,
                         "Missing arguments in call to `option`"));
    return;
  }
  auto nameNode = al->getPositionalArg(0);
  if (!nameNode.has_value()) {
    this->metadata->registerDiagnostic(
        node, Diagnostic(Severity::ERROR, node, "Missing option name"));
    return;
  }
  const auto *nameNodeSl = dynamic_cast<StringLiteral *>(nameNode->get());
  if (!nameNodeSl) {
    this->metadata->registerDiagnostic(
        nameNode->get(), Diagnostic(Severity::ERROR, nameNode->get(),
                                    "Expected string literal"));
    return;
  }
  this->checkName(nameNodeSl);
  auto optionTypeNode = al->getKwarg("type");
  if (!optionTypeNode.has_value()) {
    this->metadata->registerDiagnostic(
        nameNode->get(), Diagnostic(Severity::ERROR, nameNode->get(),
                                    "Missing option type kwarg"));
    return;
  }
  const auto *optionTypeSL =
      dynamic_cast<StringLiteral *>(optionTypeNode->get());
  if (!optionTypeSL) {
    this->metadata->registerDiagnostic(
        nameNode->get(),
        Diagnostic(Severity::ERROR, nameNode->get(),
                   "Expected option type to be a string literal"));
    return;
  }
  const auto &optionType = optionTypeSL->id;
  if (optionType != "string" && optionType != "integer" &&
      optionType != "boolean" && optionType != "combo" &&
      optionType != "array" && optionType != "feature") {
    this->metadata->registerDiagnostic(
        nameNode->get(), Diagnostic(Severity::ERROR, nameNode->get(),
                                    "Unknown option type: " + optionType));
  }
  this->metadata->registerOption(nameNodeSl);
  auto defaultValue = al->getKwarg("value");
  if (!defaultValue.has_value()) {
    // TODO: Check e.g. min/max for integer
    return;
  }
  if (optionType == "string") {
    validateStringOption(defaultValue.value().get());
  } else if (optionType == "integer") {
    validateIntegerOption(al, defaultValue.value().get());
  } else if (optionType == "boolean") {
    validateBooleanOption(defaultValue.value().get());
  } else if (optionType == "feature") {
    validateFeatureOption(defaultValue.value().get());
  } else if (optionType == "combo") {
    validateComboOption(defaultValue.value().get(), al);
  } else if (optionType == "array") {
    validateArrayOption(defaultValue.value().get(), al);
  }
}

void OptionDiagnosticVisitor::visitIdExpression(IdExpression *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitIntegerLiteral(IntegerLiteral *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitIterationStatement(
    IterationStatement *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitKeyValueItem(KeyValueItem *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitKeywordItem(KeywordItem *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitMethodExpression(MethodExpression *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitSelectionStatement(
    SelectionStatement *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitStringLiteral(StringLiteral *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitSubscriptExpression(
    SubscriptExpression *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitUnaryExpression(UnaryExpression *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitErrorNode(ErrorNode *node) {
  node->visitChildren(this);
  this->metadata->registerDiagnostic(
      node, Diagnostic(Severity::ERROR, node, node->message));
}

void OptionDiagnosticVisitor::visitBreakNode(BreakNode *node) {
  node->visitChildren(this);
}

void OptionDiagnosticVisitor::visitContinueNode(ContinueNode *node) {
  node->visitChildren(this);
}
