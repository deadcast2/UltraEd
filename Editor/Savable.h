#ifndef _SAVABLE_H_
#define _SAVABLE_H_

class CSavable
{
public:
  virtual char* Save() = 0;
  virtual bool Load(char *data) = 0;
};

#endif