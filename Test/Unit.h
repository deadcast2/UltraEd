#pragma once

#include <iostream>
#include <map>
#include <functional>
#include "Assert.h"

using namespace std;

class CUnit
{
public:
	void It(string context, function<void(CAssert)> test)
	{
		m_tests[context] = test;
	}

	void Run()
	{
		CAssert assert;
		cout << "Starting tests\n";

		for (auto test : m_tests)
		{
			try
			{
				test.second(assert);
				cout << ".";
			}
			catch (exception ex)
			{
				cout << "E\n" << test.first.c_str() << ": " << ex.what();
			}
		}

		cout << "\nDone!\n";
		cin.get();
	}

private:
	map<string, function<void(CAssert)>> m_tests;
};
