#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"
#include "build.h"
#include "util.h"

bool CBuild::Start(vector<CGameObject*> gameObjects)
{
  const char *specHeader = "#include <nusys.h>\n\n"
    "beginseg"
    "\n\tname \"code\""
	  "\n\tflags BOOT OBJECT"
	  "\n\tentry nuBoot"
	  "\n\taddress NU_SPEC_BOOT_ADDR"
    "\n\tstack NU_SPEC_BOOT_STACK"
	  "\n\tinclude \"codesegment.o\""
	  "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\rspboot.o\""
	  "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\aspMain.o\""
	  "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.fifo.o\""
	  "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspL3DEX2.fifo.o\""
	  "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.Rej.fifo.o\""
    "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.NoN.fifo.o\""
    "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DLX2.Rej.fifo.o\""
	  "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspS2DEX2.fifo.o\""
    "\nendseg\n";

  const char *specIncludeStart = "\nbeginwave"
    "\n\tname \"main\""
    "\n\tinclude \"code\"";
  const char *specIncludeEnd = "\nendwave";

  string modelLoadStart("\nvoid _UER_Load() {");
  const char *modelLoadEnd = "}";

  string cameraSetStart("void _UER_Camera() {");
  const char *cameraSetEnd = "}";

  const char *drawStart = "\n\nvoid _UER_Draw(Gfx **display_list) {";
  const char *drawEnd = "}";

  string scriptStartStart("void _UER_Start() {");
  const char *scriptStartEnd = "}";

  string scriptUpdateStart("\n\nvoid _UER_Update() {");
  const char *scriptUpdateEnd = "}";

  string inputStart("\n\nvoid _UER_Input(NUContData gamepads[4]) {");
  const char *inputEnd = "}";

  string specSegments, specIncludes, romSegments;
  string modelInits, modelDraws, cameras, scripts;
  char countBuffer[10];
  int loopCount = 0, cameraCount = 0;

  for(vector<CGameObject*>::iterator it = gameObjects.begin(); it != gameObjects.end(); ++it)
  {
    string newResName = CUtil::NewResourceName(loopCount++);
    string script = (*it)->GetScript();
    char *result = CUtil::ReplaceString(script.c_str(), "@", newResName.c_str());
    
    if((*it)->GetType() == GameObjectType::Model)
    {
      itoa(loopCount-1, countBuffer, 10);
      string gameObjectRef("_UER_Models[");
      gameObjectRef.append(countBuffer).append("]->");
      result = CUtil::ReplaceString(result, "gameObject->", gameObjectRef.c_str());
      scripts.append(result).append("\n\n");
      if(scripts.find(string(newResName).append("start(")) != string::npos)
      {
        scriptStartStart.append("\n\t").append(newResName).append("start();\n");
      }
      if(scripts.find(string(newResName).append("update(")) != string::npos)
      {
        scriptUpdateStart.append("\n\t").append(newResName).append("update();\n");
      }
      if(scripts.find(string(newResName).append("input(")) != string::npos)
      {
        inputStart.append("\n\t").append(newResName).append("input(gamepads);\n");
      }
      free(result);

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

      string modelName(newResName);
      modelName.append("_M");
      specSegments.append("\nbeginseg\n\tname \"");
      specSegments.append(modelName);
      specSegments.append("\"\n\tflags RAW\n\tinclude \"");
      specSegments.append(id);
      specSegments.append("\"\nendseg\n");
    
      specIncludes.append("\n\tinclude \"");
      specIncludes.append(modelName);
      specIncludes.append("\"");
    
      romSegments.append("extern u8 _");
      romSegments.append(modelName);
      romSegments.append("SegmentRomStart[];\n");
      romSegments.append("extern u8 _");
      romSegments.append(modelName);
      romSegments.append("SegmentRomEnd[];\n");

      itoa(loopCount-1, countBuffer, 10);
      modelInits.append("\n\t_UER_Models[");
      modelInits.append(countBuffer);

      map<string, string> resources = (*it)->GetResources();
      if(resources.count("textureDataPath"))
      {
        modelInits.append("] = (struct sos_model*)load_sos_model_with_texture(_");
      } else {
        modelInits.append("] = (struct sos_model*)load_sos_model(_");
      }

      modelInits.append(modelName);
      modelInits.append("SegmentRomStart, _");
      modelInits.append(modelName);
      modelInits.append("SegmentRomEnd");

      modelDraws.append("\n\tsos_draw(_UER_Models[");
      modelDraws.append(countBuffer);
      modelDraws.append("], display_list);\n");
    
      // Save texture data.
      if(resources.count("textureDataPath"))
      {
        // Load the set texture and resize to required dimensions.
        string path = resources["textureDataPath"];
        int width, height, channels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 3);
        if(data)
        {
          // Force 32 x 32 texture for now.
          if(stbir_resize_uint8(data, width, height, 0, data, 32, 32, 0, 3))
          {
            path.append(".rom.png");
            stbi_write_png(path.c_str(), 32, 32, 3, data, 0);
          }
        
          stbi_image_free(data);
        }

        string textureName(newResName);
        textureName.append("_T");
        specSegments.append("\nbeginseg\n\tname \"");
        specSegments.append(textureName);
        specSegments.append("\"\n\tflags RAW\n\tinclude \"");
        specSegments.append(path);
        specSegments.append("\"\nendseg\n");
        specIncludes.append("\n\tinclude \"");
        specIncludes.append(textureName);
        specIncludes.append("\"");
        romSegments.append("extern u8 _");
        romSegments.append(textureName);
        romSegments.append("SegmentRomStart[];\n");
        romSegments.append("extern u8 _");
        romSegments.append(textureName);
        romSegments.append("SegmentRomEnd[];\n");

        modelInits.append(", _");
        modelInits.append(textureName);
        modelInits.append("SegmentRomStart, _");
        modelInits.append(textureName);
        modelInits.append("SegmentRomEnd");
      }

      // Add transform data.
      char vectorBuffer[128];
      D3DXVECTOR3 position = (*it)->GetPosition();
      D3DXVECTOR3 axis;
      float angle;
      D3DXVECTOR3 scale = (*it)->GetScale();
      (*it)->GetAxisAngle(&axis, &angle);
      sprintf(vectorBuffer, ", %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",
        position.x, position.y, position.z,
        axis.x, axis.y, axis.z, angle * (180/D3DX_PI),
        scale.x, scale.y, scale.z);
      modelInits.append(vectorBuffer);
      modelInits.append(");\n");
    } else {
      itoa(cameraCount, countBuffer, 10);
      cameras.append("\n\t_UER_Cameras[").append(countBuffer).append("] = (struct sos_model*)create_camera(");
      
      string gameObjectRef("_UER_Cameras[");
      gameObjectRef.append(countBuffer).append("]->");
      result = CUtil::ReplaceString(result, "gameObject->", gameObjectRef.c_str());
      scripts.append(result).append("\n\n");
      if(scripts.find(string(newResName).append("start(")) != string::npos)
      {
        scriptStartStart.append("\n\t").append(newResName).append("start();\n");
      }
      if(scripts.find(string(newResName).append("update(")) != string::npos)
      {
        scriptUpdateStart.append("\n\t").append(newResName).append("update();\n");
      }
      if(scripts.find(string(newResName).append("input(")) != string::npos)
      {
        inputStart.append("\n\t").append(newResName).append("input(gamepads);\n");
      }
      free(result);

      char vectorBuffer[128];
      D3DXVECTOR3 position = (*it)->GetPosition();
      D3DXVECTOR3 axis;
      float angle;
      (*it)->GetAxisAngle(&axis, &angle);
      sprintf(vectorBuffer, "%lf, %lf, %lf, %lf, %lf, %lf, %lf",
        position.x, position.y, position.z,
        axis.x, axis.y, axis.z, angle * (180/D3DX_PI));
      cameras.append(vectorBuffer);
      cameras.append(");\n");
      cameraCount++;
    }
  }

  itoa(loopCount, countBuffer, 10);
  string modelArray("struct sos_model *_UER_Models[");
  modelArray.append(countBuffer);
  modelArray.append("];\n");
  modelLoadStart.insert(0, modelArray);

  itoa(cameraCount, countBuffer, 10);
  string cameraArray("struct sos_model *_UER_Cameras[");
  cameraArray.append(countBuffer);
  cameraArray.append("];\n");
  cameraSetStart.insert(0, cameraArray);

  char buffer[MAX_PATH];
  if(GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
  {
    string specPath(buffer);
    specPath.append("\\..\\..\\Engine\\spec");
    FILE *file = fopen(specPath.c_str(), "w");
    if(file == NULL) return false;
    fwrite(specHeader, 1, strlen(specHeader), file);
    fwrite(specSegments.c_str(), 1, specSegments.size(), file);
    fwrite(specIncludeStart, 1, strlen(specIncludeStart), file);
    fwrite(specIncludes.c_str(), 1, specIncludes.size(), file);
    fwrite(specIncludeEnd, 1, strlen(specIncludeEnd), file);
    fclose(file);

    string segmentsPath(buffer);
    segmentsPath.append("\\..\\..\\Engine\\segments.h");
    file = fopen(segmentsPath.c_str(), "w");
    if(file == NULL) return false;
    fwrite(romSegments.c_str(), 1, romSegments.size(), file);
    fclose(file);

    string modelInitsPath(buffer);
    modelInitsPath.append("\\..\\..\\Engine\\models.h");
    file = fopen(modelInitsPath.c_str(), "w");
    if(file == NULL) return false;
    fwrite(modelLoadStart.c_str(), 1, modelLoadStart.size(), file);
    fwrite(modelInits.c_str(), 1, modelInits.size(), file);
    fwrite(modelLoadEnd, 1, strlen(modelLoadEnd), file);
    fwrite(drawStart, 1, strlen(drawStart), file);
    fwrite(modelDraws.c_str(), 1, modelDraws.size(), file);
    fwrite(drawEnd, 1, strlen(drawEnd), file);
    fclose(file);

    string camerasPath(buffer);
    camerasPath.append("\\..\\..\\Engine\\cameras.h");
    file = fopen(camerasPath.c_str(), "w");
    if(file == NULL) return false;
    fwrite(cameraSetStart.c_str(), 1, cameraSetStart.size(), file);
    fwrite(cameras.c_str(), 1, cameras.size(), file);
    fwrite(cameraSetEnd, 1, strlen(cameraSetEnd), file);
    fclose(file);

    string scriptsPath(buffer);
    scriptsPath.append("\\..\\..\\Engine\\scripts.h");
    file = fopen(scriptsPath.c_str(), "w");
    if(file == NULL) return false;
    fwrite(scripts.c_str(), 1, scripts.size(), file);
    fwrite(scriptStartStart.c_str(), 1, scriptStartStart.size(), file);
    fwrite(scriptStartEnd, 1, strlen(scriptStartEnd), file);
    fwrite(scriptUpdateStart.c_str(), 1, scriptUpdateStart.size(), file);
    fwrite(scriptUpdateEnd, 1, strlen(scriptUpdateEnd), file);
    fwrite(inputStart.c_str(), 1, inputStart.size(), file);
    fwrite(inputEnd, 1, strlen(inputEnd), file);
    fclose(file);
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

bool CBuild::Load()
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
    currDir.append("\\..\\..\\Player\\USB");
    
    // Start the USB loader with no window.
    CreateProcess(NULL, "cmd /c 64drive_usb.exe -l ..\\..\\Engine\\main.n64", NULL, NULL, FALSE,
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