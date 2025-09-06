#pragma once

#include <array>
#include <boost/json.hpp>
#include <string>

namespace json = boost::json;

namespace pdb {
enum class DAPType { Request, Event, Response };
enum class DAPCommand { Initialize, Disconnect };
enum class DAPEvent { Initialized };

inline std::string to_string(DAPType type) {
  switch (type) {
  case DAPType::Request:
    return "request";
  case DAPType::Event:
    return "event";
  case DAPType::Response:
    return "response";
  default:
    return "invalid";
  }
}

inline std::string to_string(DAPCommand comm) {
  switch (comm) {
  case DAPCommand::Initialize:
    return "initialize";
  case DAPCommand::Disconnect:
    return "disconnect";
  default:
    return "invalid";
  }
}

inline std::string to_string(DAPEvent event) {
  switch (event) {
  case DAPEvent::Initialized:
    return "initialized";
  default:
    return "invalid";
  }
}

class ProtocolMessage {
public:
  ProtocolMessage(int seq, DAPType tp) : sequence(seq), type(tp) {};
  virtual ~ProtocolMessage() {};

  virtual std::string getFullMessage() {
    auto contentMessage = getContentMessage();
    auto serializedContent = json::serialize(contentMessage);
    auto contentLength = std::to_string(serializedContent.length());

    std::string headerMessage("Content-Length: " + contentLength + "\r\n\r\n");
    return headerMessage + serializedContent;
  }

  virtual json::object getContentMessage() {
    return json::object{{"seq", sequence}, {"type", to_string(type)}};
  };

private:
  int sequence;
  DAPType type;
};

class Request : public ProtocolMessage {
public:
  Request(int seq, DAPCommand comm, json::object args = json::object{})
      : ProtocolMessage(seq, DAPType::Request), command(comm),
        arguments(std::move(args)) {};
  virtual ~Request() {};

  virtual json::object getContentMessage() {
    auto baseMessage = ProtocolMessage::getContentMessage();
    baseMessage["command"] = to_string(command);

    if (arguments.size() > 0)
      baseMessage["arguments"] = arguments;

    return baseMessage;
  };

private:
  DAPCommand command;
  json::object arguments;
};

class Event : public ProtocolMessage {
public:
  Event(int seq, DAPEvent ev, json::object bd = json::object{})
      : ProtocolMessage(seq, DAPType::Event), event(ev), body(std::move(bd)) {};
  virtual ~Event() {};

  virtual json::object getContentMessage() {
    auto baseMessage = ProtocolMessage::getContentMessage();
    baseMessage["event"] = to_string(event);

    if (body.size() > 0)
      baseMessage["body"] = body;

    return baseMessage;
  };

private:
  DAPEvent event;
  json::object body;
};

class Response : public ProtocolMessage {
public:
  Response(int seq, int req_seq, bool succ, DAPCommand comm,
           const std::string &msg = "", json::object bd = json::object{})
      : ProtocolMessage(seq, DAPType::Response), req_sequence(req_seq),
        success(succ), command(comm), message(msg), body(std::move(bd)) {};
  virtual ~Response() {};

  virtual json::object getContentMessage() {
    auto baseMessage = ProtocolMessage::getContentMessage();
    baseMessage["req_sequence"] = req_sequence;
    baseMessage["success"] = success;
    baseMessage["command"] = to_string(command);

    if (message.length() > 0)
      baseMessage["message"] = message;

    if (body.size() > 0)
      baseMessage["body"] = body;

    return baseMessage;
  };

private:
  int req_sequence;
  bool success;
  DAPCommand command;
  std::string message;
  json::object body;
};
} // namespace pdb