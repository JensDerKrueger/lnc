#include "PainterCommon.h"

MessageType identifyString(const std::string& s) {
  Tokenizer t(s);
  if (t.nextString() != "painter") return MessageType::InvalidMessage;
  uint32_t i;
  try {
    i = t.nextUint32();
    if (i > uint32_t(MessageType::ConnectMessage)) return MessageType::InvalidMessage;
  } catch (const std::invalid_argument&) {
    return MessageType::InvalidMessage;
  }
  return MessageType(i);
}
