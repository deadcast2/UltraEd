#include "Registry.h"

namespace UltraEd
{
    const string CRegistry::m_key = "Software\\UltraEd";

    bool CRegistry::Set(const string &key, const string &value)
    {
        HKEY regKey;
        bool success = false;

        if (RegCreateKeyEx(HKEY_CURRENT_USER, CRegistry::m_key.c_str(), 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &regKey, NULL) == ERROR_SUCCESS)
        {
            int valueLength = strnlen(value.c_str(), REG_DATA_LENGTH);
            if (valueLength > 0 && RegSetValueEx(regKey, key.c_str(), 0, REG_SZ, 
                (const BYTE*)value.c_str(), valueLength) == ERROR_SUCCESS)
            {
                success = true;
            }

            RegCloseKey(regKey);
        }

        return success;
    }

    bool CRegistry::Get(const string &key, string &value)
    {
        HKEY regKey;
        bool success = false;

        if (RegOpenKeyEx(HKEY_CURRENT_USER, CRegistry::m_key.c_str(), 0, KEY_QUERY_VALUE, &regKey) == ERROR_SUCCESS)
        {
            char buffer[256];
            DWORD bufferSize = sizeof(buffer);
            if (RegQueryValueEx(regKey, key.c_str(), NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
            {
                value = string(buffer);
                success = true;
            }

            RegCloseKey(regKey);
        }

        return success;
    }
}
