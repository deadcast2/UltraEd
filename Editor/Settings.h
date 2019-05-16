#pragma once

#include <windows.h>
#include <string>

#define REG_DATA_LENGTH SIZE_MAX

using namespace std;

namespace UltraEd
{
	class CSettings
	{
	public:
		static bool Set(const char *key, const char *value);
		static bool Get(const char *key, string &value);

	private:
		static const char *m_key;
	};
}
