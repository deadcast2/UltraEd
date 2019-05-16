#include "Settings.h"

namespace UltraEd
{
	const char *CSettings::m_key = "Software\\UltraEd";

	bool CSettings::Set(const char *key, const char *value)
	{
		HKEY regKey;
		bool success = false;

		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, CSettings::m_key, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &regKey, NULL) == ERROR_SUCCESS)
		{
			int valueLength = strnlen(value, REG_DATA_LENGTH);
			if (valueLength > 0 && RegSetValueEx(regKey, key, 0, REG_SZ, (const BYTE*)value, valueLength) == ERROR_SUCCESS)
			{
				success = true;
			}

			RegCloseKey(regKey);
		}

		return success;
	}

	bool CSettings::Get(const char *key, string &value)
	{
		HKEY regKey;
		bool success = false;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CSettings::m_key, 0, KEY_ALL_ACCESS, &regKey) == ERROR_SUCCESS)
		{
			char buffer[256];
			DWORD bufferSize = sizeof(buffer);
			if (RegQueryValueEx(regKey, key, NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
			{
				value = string(buffer);
				success = true;
			}

			RegCloseKey(regKey);
		}

		return success;
	}
}
