#include "build.h"

bool CBuild::Start()
{
  return Compile();
}

bool CBuild::Compile()
{
  string sdkPath;
  if(CSettings::Get("N64 SDK Path", sdkPath))
  {
    // Set the root env variable for the N64 build tools.
    SetEnvironmentVariable("ROOT", sdkPath.c_str());

    // Get the path to where the program is running.
    char buffer[MAX_PATH];
    if(GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
    {
      DWORD exitCode;
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);
      ZeroMemory(&pi, sizeof(pi));

      // Format the path to execute the ROM build.
      string currDir(buffer);
      currDir.append("\\..\\..\\Engine");

      // Start the build with no window.
      CreateProcess(NULL, "cmd /c build.bat", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);
      WaitForSingleObject(pi.hProcess, INFINITE);
      GetExitCodeProcess(pi.hProcess, &exitCode);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      if(exitCode) 
      {
        MessageBox(NULL, "The ROM build has failed. Make sure the correct N64 SDK path was specified.", "Error", MB_OK);
      }
      else
      {
        MessageBox(NULL, "The ROM has been successfully built!", "Success", MB_OK);
      }
    }
  }
  else
  {
    MessageBox(NULL, "The N64 SDK path must be set before trying to build.", "Error", MB_OK);
  }

  return true;
}