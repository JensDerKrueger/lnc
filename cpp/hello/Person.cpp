#include <string>

#include "Person.h"


Person::Person(const std::string& vn, const std::string& nn) :
	strVorname(vn),
	strNachname(nn)
{
}

std::string Person::nachname() {
	return strNachname;
}

std::string Person::vorname() {
	return strVorname;
}