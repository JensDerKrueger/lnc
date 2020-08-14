#define _CRT_SECURE_NO_WARNINGS 1

#include "ClockManager.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"

#include <sstream>
#include <functional>
#include <ctime>


ClockManager::ClockManager() {
}

double ClockManager::getClock(const std::string& name) const {
  std::string lName = ParserTools::toLowerCase(name);

  time_t rawtime;
  tm * timeinfo;
  time(&rawtime);
  timeinfo=localtime(&rawtime);
  
  if (lName == "hour") return timeinfo->tm_hour;
  if (lName == "min") return timeinfo->tm_min;
  if (lName == "sec") return timeinfo->tm_sec;
  if (lName == "day") return timeinfo->tm_mday;
  if (lName == "month") return timeinfo->tm_mon+1;
  if (lName == "year") return timeinfo->tm_year+1900;
  if (lName == "mod") return timeinfo->tm_hour*60+timeinfo->tm_min; // minute of day
  if (lName == "dow") return (timeinfo->tm_wday+6)%7; // day of week
  if (lName == "doy") return timeinfo->tm_yday+1; // day of year
  if (lName == "woy") return 1+(timeinfo->tm_yday + 6 - (timeinfo->tm_wday+6)%7)/7; // week of year
  
  std::stringstream ss;
  ss << "invalid clock value \"" << name << "\"";
  throw ParseException(ss.str());
}


std::string ClockManager::toString() const {
  std::stringstream ss;
  ss << getClock("hour") << ":" << getClock("min") << ":" << getClock("sec")
  << "  " << getClock("day") << "." << getClock("month") << "."
  << getClock("year") << " Day of week (starting monday): " << getClock("dow")
  << ", Day of year: " << getClock("doy") << ", Week of year: "
  << getClock("woy") << std::endl;
  return ss.str();
}

