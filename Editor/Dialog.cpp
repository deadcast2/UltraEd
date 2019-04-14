#include "Dialog.h"
#include <windows.h>

namespace UltraEd
{
	bool CDialog::Open(const char *title, const char *filter, string &file)
	{
		OPENFILENAME ofn;
		char szFile[MAX_PATH];

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrTitle = title;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		if (GetOpenFileName(&ofn))
		{
			file = ofn.lpstrFile;
			return true;
		}

		return false;
	}

	bool CDialog::Save(const char *title, const char *filter, string &file)
	{
		OPENFILENAME ofn;
		char szFile[MAX_PATH];

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrTitle = title;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn))
		{
			file = ofn.lpstrFile;
			return true;
		}

		return false;
	}
}
