#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <string>

using namespace std;

namespace UltraEd
{
	class CDialog
	{
	public:
		static bool Open(const char *title, const char *filter, string &file);
		static bool Save(const char *title, const char *filter, string &file);

	private:
		CDialog() {}
	};
}

#endif
