#include "utils.h"
#include <string.h>

bool maskcmp (const char* s,const char* mask)
{
  while (*s && *mask) 
    switch (*mask)
    {
      case '*':
        do mask++; while (*mask == '*' || *mask == '?');

        if (!*mask)
          return true;

        for (s=strchr(s,*mask);s;s=strchr (s+1,*mask))
          if(maskcmp (s,mask))
            return true;

        return false;
      default:
        if (*mask != *s) 
          return false;
        mask++;
        s++;
        break;
      case '?':
        mask++;       
        return maskcmp(s,mask) || maskcmp(s+1,mask);
    }

  if (!*s)
  {
    while (*mask)
      switch (*mask)
      {
        case '*':
        case '?':  
          mask++;
          break;             
        default:
          return false;          
      }

    return true;
  }
  
  return false;
}
