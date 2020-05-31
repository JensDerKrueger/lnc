#include <string>
#include "Student.h"

Student::Student(const std::string& vn, const std::string& nn, int mat) :
	Person(vn, nn),
	intNummer(mat)
{
}

int Student::nummer() {
	return intNummer;
}
