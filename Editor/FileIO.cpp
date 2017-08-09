#include "FileIO.h"
#include "cJSON.h"

CFileIO::CFileIO()
{
}

CFileIO::~CFileIO()
{
}

BOOL CFileIO::Save(CScene *scene)
{
  OPENFILENAME ofn;
  char szFile[260];
  
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "JSON (*.json)";
  ofn.nFilterIndex = 1;
  ofn.lpstrTitle = "Select scene";
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_OVERWRITEPROMPT;
  
  if(GetSaveFileName(&ofn))
  {
    char *saveName = ofn.lpstrFile;

    // Add the extension if not supplied in the dialog.
    if(strstr(saveName, ".json") == NULL)
    {
      sprintf(saveName, "%s.json", saveName);
    }

    // Create some dummy JSON data.
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "test", cJSON_CreateString("hi"));
    char *rendered = cJSON_Print(root);
    
    // Write the JSON data out.
    FILE *file = fopen(saveName, "w");
    fprintf(file, rendered);
    fclose(file);
    
    // Clean!
    cJSON_Delete(root); 
  }
  
  return TRUE;
}