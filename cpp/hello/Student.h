#pragma once

#include "Person.h"

class Student : public Person {
	public:
		Student(const std::string& vn, const std::string& nn, int mat);
		int nummer();
		
	private:
		int intNummer;
};



