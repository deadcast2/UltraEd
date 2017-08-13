#ifndef _SAVABLE_H_
#define _SAVABLE_H_

enum PackType { Editor, Object };

typedef struct
{
  PackType type;
  char *data;
} Pack;

class CSavable
{
public:
  virtual Pack Save() = 0;
};

#endif