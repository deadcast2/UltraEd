#include "FileIO.h"
#include "cJSON.h"

CFileIO::CFileIO()
{
}

CFileIO::~CFileIO()
{
}

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
    char *saveName = ofn.lpstrFile;

    // Add the extension if not supplied in the dialog.
    if(strstr(saveName, ".ultra") == NULL)
    {
      sprintf(saveName, "%s.ultra", saveName);
    }
    
    // Write the JSON data out.
    FILE *file = fopen(saveName, "w");
    std::vector<CSavable*>::iterator it;

    for(it = savables.begin(); it != savables.end(); ++it)
    {      
      fprintf(file, (*it)->Save());
    }

    fclose(file);
  }
  
  return true;
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
  
  if(GetOpenFileName(&ofn))
  {
    FILE *file = fopen(ofn.lpstrFile, "r");
    if(file == NULL) return false;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *contents = (char*)malloc(size);
    if(contents == NULL) return false;

    fread(contents, size, 1, file);
    fclose(file);

    *data = contents;

    return true;
  }

  return false;
}