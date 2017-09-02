#include "FileIO.h"
#include "cJSON.h"
#include "microtar.h"
#include "fastlz.h"
#include <shlwapi.h>

bool CFileIO::Save(std::vector<CSavable*> savables)
{
  OPENFILENAME ofn;
  char szFile[260];
  
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "UltraEd (*.ultra)";
  ofn.nFilterIndex = 1;
  ofn.lpstrTitle = "Save Scene";
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_OVERWRITEPROMPT;
  
  if(GetSaveFileName(&ofn))
  {
    char* saveName = ofn.lpstrFile;

    // Add the extension if not supplied in the dialog.
    if(strstr(saveName, ".ultra") == NULL)
    {
      sprintf(saveName, "%s.ultra", saveName);
    }

    // Prepare tar file for writing.
    mtar_t tar;
    mtar_open(&tar, saveName, "w");
    
    // Write the scene JSON data.
    cJSON* root = cJSON_CreateObject();
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "models", array);

    std::vector<CSavable*>::iterator it;
    for(it = savables.begin(); it != savables.end(); ++it)
    {
      Savable current = (*it)->Save();
      cJSON* object = current.object->child;

      // Rewrite and archive the attached resources.
      std::map<char*, char*>::iterator rit;
      std::map<char*, char*> resources = (*it)->GetResources();
      for(rit = resources.begin(); rit != resources.end(); rit++)
      {
        const char* fileName = PathFindFileName(rit->second);
        FILE* file = fopen(rit->second, "rb");

        if(file == NULL) continue;

        cJSON_AddStringToObject(object, rit->first, fileName);
        
        // Calculate resource length.
        fseek(file, 0, SEEK_END);
        long fileLength = ftell(file);
        rewind(file);

        // Read all contents of resource into a buffer.
        char* fileContents = (char*)malloc(fileLength);
        fread(fileContents, fileLength, 1, file);

        // Write the buffer to the tar archive.
        mtar_write_file_header(&tar, fileName, fileLength);
        mtar_write_data(&tar, fileContents, fileLength);

        fclose(file);
        free(fileContents);
      }

      if(current.type == SavableType::Editor)
      {
        cJSON_AddItemToObject(root, object->string, object);
      }
      else if(current.type == SavableType::Model)
      {
        cJSON_AddItemToArray(array, object);
      }
    }

    char* rendered = cJSON_Print(root);
    cJSON_Delete(root);

    mtar_write_file_header(&tar, "scene.json", strlen(rendered));
    mtar_write_data(&tar, rendered, strlen(rendered));

    mtar_finalize(&tar);
    mtar_close(&tar);

    return Compress(saveName);
  }
  
  return false;
}

bool CFileIO::Load(char** data)
{
  OPENFILENAME ofn;
  char szFile[260];
  
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "UltraEd (*.ultra)";
  ofn.nFilterIndex = 1;
  ofn.lpstrTitle = "Load Scene";
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  
  if(GetOpenFileName(&ofn) && Decompress(&ofn.lpstrFile))
  {
    mtar_t tar;
    mtar_header_t header;
    
    mtar_open(&tar, ofn.lpstrFile, "r");
    mtar_find(&tar, "scene.json", &header);

    char *contents = (char*)calloc(1, header.size + 1);
    mtar_read_data(&tar, contents, header.size);
    mtar_close(&tar);
    remove(ofn.lpstrFile);

    *data = contents;

    return true;
  }

  return false;
}

FileInfo CFileIO::Import(const char* file)
{
  const char* root = "Library";
  const char* assets = "Assets/";

  if(CreateDirectory(root, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
  {
    char* name = PathFindFileName(file);
    char* target = (char*)malloc(128);
    char guidBuffer[40];
    GUID uniqueIdentifier;
    wchar_t guidWide[40];

    // Cache the starting directory.
    if(strlen(startingDir) == 0)
    {
      GetCurrentDirectory(128, startingDir);
    }

    if(strncmp(file, assets, strlen(assets)) == 0)
    {
      FileInfo info = { strdup(file), Editor };
      return info;
    }

    // Create a unique identifier and convert to a char array.
    CoCreateGuid(&uniqueIdentifier);
    StringFromGUID2(uniqueIdentifier, guidWide, 40);
    wcstombs(guidBuffer, guidWide, 40);

    // Remove the first and last curly brace from GUID.
    memmove(guidBuffer, guidBuffer + 1, strlen(guidBuffer));
    guidBuffer[strlen(guidBuffer) - 1] = '\0';

    // Format new imported path.
    sprintf(target, "%s\\%s\\%s-%s", startingDir, root, guidBuffer, name);

    if(CopyFile(file, target, FALSE))
    {
      FileInfo info = { target, User };
      return info;
    }
  }

  FileInfo info = { strdup(file), Unknown };
  return info;
}

bool CFileIO::Compress(const char* path)
{
  FILE* file = fopen(path, "rb");
  if(file == NULL) return false;

  // Get the total size of the file.
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  // Read in entire file.
  char* data = (char*)malloc(size);
  if(data == NULL) return false;
  int bytesRead = fread(data, 1, size, file);
  if(bytesRead != size) return false;
  fclose(file);

  // Compressed buffer must be at least 5% larger.
  char* compressed = (char*)malloc(size + (size * 0.05));
  if(compressed == NULL) return false;
  int bytesCompressed = fastlz_compress(data, size, compressed);
  if(bytesCompressed == 0) return false;

  // Write compressed file back out.
  file = fopen(path, "wb");
  if(file == NULL) return false;
  unsigned int bytesWritten = fwrite(compressed, 1, bytesCompressed, file);
  if(bytesWritten != bytesCompressed) return false;
  fclose(file);
  free(compressed);
  free(data);

  return true;
}

bool CFileIO::Decompress(char** path)
{
  FILE* file = fopen(*path, "rb");
  if(file == NULL) return false;

  // Get the total size of the file.
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  // Read in entire file.
  char* data = (char*)malloc(size);
  if(data == NULL) return false;
  int bytesRead = fread(data, 1, size, file);
  if(bytesRead != size) return false;
  fclose(file);

  int maxout = size * size; // Need to fix this!
  char* decompressed = (char*)malloc(maxout);
  if(decompressed == NULL) return false;
  int bytesDecompressed = fastlz_decompress(data, size, decompressed, maxout);
  if(bytesDecompressed == 0) return false;

  // Create a temp path to extract the scene file.
  std::string pathBuffer(*path);
  std::string tempName(tmpnam(NULL));
  pathBuffer.append(tempName.erase(0,1));

  // Write decompressed file back out.
  file = fopen(pathBuffer.c_str(), "wb");
  if(file == NULL) return false;
  unsigned int bytesWritten = fwrite(decompressed, 1, bytesDecompressed, file);
  if(bytesWritten != bytesDecompressed) return false;
  fclose(file);
  free(decompressed);
  free(data);

  // Pass the new path out.
  *path = strdup(pathBuffer.c_str());

  return true;
}