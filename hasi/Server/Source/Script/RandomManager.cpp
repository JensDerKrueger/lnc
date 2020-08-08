#define _CRT_SECURE_NO_WARNINGS 1

#include "RandomManager.h"
#include "ExpressionExceptions.h"
#include "ParserTools.h"


#include <limits>
#include <ctime>
#include <random>
#include <memory>
typedef std::mt19937 mtRNG;

RandomManager::RandomManager() {
  time_t rawtime;
  tm * timeinfo;
  time(&rawtime);
  timeinfo=localtime(&rawtime);
  
  m_rng.seed(timeinfo->tm_hour*60*60+timeinfo->tm_min*60+timeinfo->tm_sec);
}

double RandomManager::getRandom(const std::string& name) {
  std::uniform_int_distribution<uint32_t> uniform_dist;
  std::normal_distribution<double> normal_dist(0, 1);
  
  std::string lName = ParserTools::toLowerCase(name);

  if (ParserTools::startsWith(lName, "uniform"))
    return double(uniform_dist(m_rng))/double(std::numeric_limits<uint32_t>::max());
  
  if (ParserTools::startsWith(lName, "normal"))
    return normal_dist(m_rng);
  
  std::stringstream ss;
  ss << "invalid Random value \"" << name << "\"";
  throw ParseException(ss.str());
}


std::string RandomManager::toString() const {
  return "A random number generator";
}

