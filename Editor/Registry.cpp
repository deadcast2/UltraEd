#include "Registry.h"

namespace UltraEd
{
    const std::string Registry::m_key = "Software\\UltraEd";

    bool Registry::Set(const std::string &key, const std::string &value)
    {
        HKEY regKey;
        bool success = false;

        if (RegCreateKeyEx(HKEY_CURRENT_USER, Registry::m_key.c_str(), 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &regKey, NULL) == ERROR_SUCCESS)
        {
            DWORD valueLength = static_cast<DWORD>(strnlen(value.c_str(), REG_DATA_LENGTH));
            if (valueLength > 0 && RegSetValueEx(regKey, key.c_str(), 0, REG_SZ, 
                (const BYTE*)value.c_str(), valueLength) == ERROR_SUCCESS)
            {
                success = true;
            }

            RegCloseKey(regKey);
        }

        return success;
    }

    bool Registry::Get(const std::string &key, std::string &value)
    {
        HKEY regKey;
        bool success = false;

        if (RegOpenKeyEx(HKEY_CURRENT_USER, Registry::m_key.c_str(), 0, KEY_QUERY_VALUE, &regKey) == ERROR_SUCCESS)
        {
            char buffer[256];
            DWORD bufferSize = sizeof(buffer);
            if (RegQueryValueEx(regKey, key.c_str(), NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
            {
                value = std::string(buffer);
                success = true;
            }

            RegCloseKey(regKey);
        }

        return success;
    }
}
