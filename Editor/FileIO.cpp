#include "FileIO.h"
#include "cJSON.h"

CFileIO::CFileIO()
{
}

CFileIO::~CFileIO()
{
}

BOOL CFileIO::Save(std::vector<CSavable*> savables)
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
  
  return TRUE;
}