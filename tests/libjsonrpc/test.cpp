#include "polyfill.hpp"

#include <cstring>
#include <iostream>
#include <jsonrpc.hpp>
#include <log.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>

class TestJsonRpcHandler : public jsonrpc::JsonRpcHandler {
private:
  Logger logger;

public:
  TestJsonRpcHandler() : logger(Logger("TestJsonRpcHandler")) {}

  void handleNotification(std::string method,
                          nlohmann::json /*params*/) override {
    this->logger.info(std::format("Got notification: {}", method));
  }

  void handleRequest(std::string method, nlohmann::json callId,
                     nlohmann::json params) override {
    this->logger.info(std::format("Got request: {}", method));
    if (strcmp(method.data(), "add") == 0) {
      int const a = params["a"];
      int const b = params["b"];
      nlohmann::json data;
      data["c"] = a + b;
      this->server->reply(callId, data);
    }
  }

  ~TestJsonRpcHandler() override = default;
};

static std::string makeJsonrpcCall(int id, const std::string &method,
                                   nlohmann::json params) {
  nlohmann::json data;
  data["jsonrpc"] = "2.0";
  data["method"] = method;
  data["params"] = std::move(params);
  data["id"] = id;
  std::string payload = data.dump();
  auto len = payload.size();
  return std::format("Content-Length:{}\r\n\r\n{}", len, payload);
}

static std::string makeJsonRpcNotification(const std::string &method,
                                           nlohmann::json params) {
  nlohmann::json data;
  data["jsonrpc"] = "2.0";
  data["method"] = method;
  data["params"] = std::move(params);
  std::string payload = data.dump();
  auto len = payload.size();
  return std::format("Content-Length:{}\r\n\r\n{}", len, payload);
}

static std::string makeInputMessage() {
  std::string ret;
  ret += makeJsonrpcCall(1, "add", R"({"a": 3, "b": 2})"_json);
  ret += makeJsonRpcNotification("notif", R"({"msg": "Foo"})"_json);
  return ret;
}

int main(int /*unused*/, char ** /*unused*/) {
  auto handler = std::make_shared<TestJsonRpcHandler>();
  std::istringstream newSin(makeInputMessage());
  std::ostringstream newSout;
  auto server = std::make_shared<jsonrpc::JsonRpcServer>(newSin, newSout);
  handler->server = server;
  server->loop(handler);
  server->wait();
  std::cout << newSout.str() << std::endl;
}
