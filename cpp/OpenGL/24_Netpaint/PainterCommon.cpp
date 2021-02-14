#include "PainterCommon.h"

PayloadType identifyString(const std::string& s) {
  std::vector<std::string> l = Coder::decode(s,2);

  if (l.size() < 2) return PayloadType::InvalidPayload;
  if (l[0] != "painter") return PayloadType::InvalidPayload;
  
  int i;
  try {
    i = std::stoi( l[1] );
    if (i < 0 || i > int(PayloadType::ConnectPayload)) return PayloadType::InvalidPayload;
  } catch (const std::invalid_argument&) {
    return PayloadType::InvalidPayload;
  }
  
  return PayloadType(std::stoi(l[1]));
}
