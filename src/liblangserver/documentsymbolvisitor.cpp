#include "documentsymbolvisitor.hpp"

#include "langserverutils.hpp"
#include "lsptypes.hpp"
#include "node.hpp"
#include "type.hpp"

void DocumentSymbolVisitor::visitArgumentList(ArgumentList *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitArrayLiteral(ArrayLiteral *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitAssignmentStatement(
    AssignmentStatement *node) {
  node->visitChildren(this);
  if (node->op != AssignmentOperator::EQUALS) {
    return;
  }
  const auto *lhsId = dynamic_cast<IdExpression *>(node->lhs.get());
  if (lhsId) {
    this->createSymbol(lhsId);
  }
}

void DocumentSymbolVisitor::visitBinaryExpression(BinaryExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitBooleanLiteral(BooleanLiteral *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitBuildDefinition(BuildDefinition *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitConditionalExpression(
    ConditionalExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitDictionaryLiteral(DictionaryLiteral *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitFunctionExpression(FunctionExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitIdExpression(IdExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitIntegerLiteral(IntegerLiteral *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitIterationStatement(IterationStatement *node) {
  node->visitChildren(this);
  for (const auto &iter : node->ids) {
    const auto *idExpr = dynamic_cast<IdExpression *>(iter.get());
    if (!idExpr) {
      continue;
    }
    this->createSymbol(idExpr);
  }
}

void DocumentSymbolVisitor::visitKeyValueItem(KeyValueItem *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitKeywordItem(KeywordItem *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitMethodExpression(MethodExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitSelectionStatement(SelectionStatement *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitStringLiteral(StringLiteral *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitSubscriptExpression(
    SubscriptExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitUnaryExpression(UnaryExpression *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitErrorNode(ErrorNode *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitBreakNode(BreakNode *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::visitContinueNode(ContinueNode *node) {
  node->visitChildren(this);
}

void DocumentSymbolVisitor::createSymbol(const IdExpression *idExpr) {
  using enum SymbolKind;
  auto name = idExpr->id;
  const auto &loc = idExpr->location;
  auto range = LSPRange(LSPPosition(loc.startLine, loc.startColumn),
                        LSPPosition(loc.endLine, loc.endColumn));
  auto lspLocation = LSPLocation(pathToUrl(idExpr->file->file), range);
  auto kind = VARIABLE_KIND;
  auto types = idExpr->types;
  if (types.size() == 1) {
    auto *type = types[0].get();
    if (dynamic_cast<AbstractObject *>(type)) {
      kind = OBJECT_KIND;
    } else if (dynamic_cast<Str *>(type)) {
      kind = STRING_KIND;
    } else if (dynamic_cast<IntType *>(type)) {
      kind = NUMBER_KIND;
    } else if (dynamic_cast<BoolType *>(type)) {
      kind = BOOLEAN_KIND;
    } else if (dynamic_cast<List *>(type)) {
      kind = LIST_KIND;
    }
  }
  this->symbols.emplace_back(name, kind, lspLocation);
}
