#pragma once
#include "jsonrpc.hpp"
#include "lsptypes.hpp"

#include <nlohmann/json.hpp>
#include <vector>

class AbstractLanguageServer : public jsonrpc::JsonRpcHandler {
public:
  void handleNotification(std::string method, nlohmann::json params) override;
  void handleRequest(std::string method, nlohmann::json callId,
                     nlohmann::json params) override;

  virtual InitializeResult initialize(InitializeParams &params) = 0;
  virtual std::vector<InlayHint> inlayHints(InlayHintParams &params) = 0;
  virtual void shutdown() = 0;

  virtual void onInitialized(InitializedParams &params) = 0;
  virtual void onExit() = 0;
  virtual void onDidOpenTextDocument(DidOpenTextDocumentParams &params) = 0;
  virtual void onDidChangeTextDocument(DidChangeTextDocumentParams &params) = 0;
  virtual void onDidSaveTextDocument(DidSaveTextDocumentParams &params) = 0;
  virtual void onDidCloseTextDocument(DidCloseTextDocumentParams &params) = 0;
};