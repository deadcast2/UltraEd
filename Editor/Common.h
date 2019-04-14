#pragma once

#include <windows.h>

#define APP_NAME "UltraEd v0.1"
#define APP_FILE_EXT ".ultra"
#define APP_FILE_FILTER "UltraEd (*.ultra)\0*.ultra"
#define snap(x) (floor(x + 0.5f))

inline bool operator<(const GUID &first, const GUID &second)
{
	return memcmp(&first, &second, sizeof(GUID)) < 0;
}
