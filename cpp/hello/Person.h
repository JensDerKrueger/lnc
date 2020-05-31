#pragma once

class Person {
	public:
		Person(const std::string& vn, const std::string& nn);
		std::string nachname();
		std::string vorname();
		
	private:
		std::string strVorname;
		std::string strNachname;
};
