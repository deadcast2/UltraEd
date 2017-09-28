#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"
#include "build.h"
#include "util.h"

bool CBuild::Start(vector<CModel*> models)
{
  for(vector<CModel*>::iterator it = models.begin(); it != models.end(); ++it)
  {
    // Save mesh information.
    int i = 0;
    string id = CUtil::GuidToString((*it)->GetId());
    id.insert(0, CUtil::RootPath().append("\\"));
    id.append(".rom.sos");
    FILE *file = fopen(id.c_str(), "w");
    if(file == NULL) return false;
    
    vector<MeshVertex> vertices = (*it)->GetVertices();
    
    fprintf(file, "%i\n", vertices.size());
    
    for(i = 0; i < vertices.size(); i++)
    {
      MeshVertex vert = vertices[i];
      fprintf(file, "%f %f %f %f %f\n", 
        vert.position.x,
        vert.position.y,
        vert.position.z,
        vert.tu,
        vert.tv);
    }
    
    fclose(file);
    
    // Save texture data.
    map<string, string> resources = (*it)->GetResources();
    if(resources.count("textureDataPath"))
    {
      // Load the set texture and resize to required dimensions.
      string path = resources["textureDataPath"];
      int width, height, channels;
      unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
      if(data)
      {
        // Force 32 x 32 texture for now.
        if(stbir_resize_uint8(data, width, height, 0, data, 32, 32, 0, channels))
        {
          path.append(".rom.png");
          stbi_write_png(path.c_str(), 32, 32, 4, data, 0);
        }
        
        stbi_image_free(data);
      }
    }
  }
  
  return Compile();
}

bool CBuild::Run()
{
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
    currDir.append("\\..\\..\\Player");
    
    // Start the build with no window.
    CreateProcess(NULL, "cmd /c Project64.exe ..\\Engine\\main.n64", NULL, NULL, FALSE,
      CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return exitCode == 0;
  }

  return false;
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
      
      return exitCode == 0;
    }
  }
  
  return false;
}