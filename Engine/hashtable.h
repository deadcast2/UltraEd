#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#define HASHSIZE 1000

struct nlist
{
  struct nlist *next;
  char *name;
  struct sos_model *gameObject;
};

static struct nlist *hashtable[HASHSIZE];

unsigned hash(char *s)
{
  unsigned hashval;
  for(hashval = 0; *s != '\0'; s++)
  {
    hashval = *s + 31 * hashval;
  }
  return hashval % HASHSIZE;
}

struct nlist *lookup(char *s)
{
  struct nlist *np;
  for(np = hashtable[hash(s)]; np != NULL; np = np->next)
  {
    if(strcmp(s, np->name) == 0)
    {
      return np;
    }
  }
  return NULL;
}

struct nlist *insert(char *name, struct sos_model *gameObject)
{
  struct nlist *np;
  unsigned hashval;
  if((np = lookup(name)) == NULL)
  {
    np = (struct nlist*)malloc(sizeof(*np));
    if(np == NULL || (np->name = strdup(name)) == NULL) return NULL;
    hashval = hash(name);
    np->next = hashtable[hashval];
    hashtable[hashval] = np;
  }
  else
  {
    free((void*)np->gameObject);
  }

  if((np->gameObject = gameObject) == NULL) return NULL;
  
  return np;
}

#endif