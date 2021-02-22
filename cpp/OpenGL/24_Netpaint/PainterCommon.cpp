#include "PainterCommon.h"

MessageType identifyString(const std::string& s) {
  std::vector<std::string> l = Coder::decode(s,2);

  if (l.size() < 2) return MessageType::InvalidMessage;
  if (l[0] != "painter") return MessageType::InvalidMessage;
  
  int i;
  try {
    i = std::stoi( l[1] );
    if (i < 0 || i > int(MessageType::ConnectMessage)) return MessageType::InvalidMessage;
  } catch (const std::invalid_argument&) {
    return MessageType::InvalidMessage;
  }
  
  return MessageType(std::stoi(l[1]));
}
