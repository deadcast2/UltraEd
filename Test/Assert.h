#pragma once

#include <sstream>

using namespace std;

class CAssert
{
public:
	void Equal(string expected, string actual)
	{
		stringstream ss;
		ss << "'" << expected << "'" << " does not equal " << "'" << actual << "'";
		if (expected != actual) throw exception(ss.str().c_str());
	}
};
