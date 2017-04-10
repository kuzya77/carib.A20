#include <ctype.h>
#include <string.h>

#include "lexer.h"

char* Lexer::null_ptr  = NULL;
char  Lexer::null_char = ' ';

Lexer::Lexer (char* buf)
{
  cur_lexem   = UNDEFINED;
  cur_error   = NO_LEX_ERROR;
  pos         = buf;
  cursor      = pos; 
  cur_token   = NULL;
  line_number = 1;
  line_start  = pos;

  erased_char     = ' ';
  erased_char_pos = &null_char;
}

void Lexer::SetError (Error err,char* _pos)
{
  cur_error = err;
  cur_lexem = UNDEFINED;
  cur_token = NULL;
  cursor    = _pos?_pos:pos;
}

void Lexer::skip_rest_of_line ()
{
  for (;;pos++)
    switch (*pos)
    {
      case '\n':
      case '\0':
        return;
    }  
}

void Lexer::nextline ()
{
  line_number++;
  line_start = pos+1;
}

void Lexer::skip_block_comment ()
{
  for (;;pos++)
    switch (*pos)
    {
      case '\0':
        SetError (UNCLOSED_COMMENT);
        return;
      case '*':
        if (pos [1] == '/')
        {
          pos+=2;
          return;
        }
        break;
      case '\n':
        nextline ();
    }
}

void Lexer::skip ()
{
  for (;;)
    switch (*pos)
    {
      case '\t':
      case '\r':
      case ' ':
        pos++;
        break;
      case '/':
        switch (pos [1])
        {
          case '/': 
            pos+=2;
            skip_rest_of_line ();
            return;
          case '*':
            pos+=2;
            skip_block_comment ();
            return;
        }
      default:
        return;
    }
}

void Lexer::read_string (char border)
{
  cur_token = pos;
  cur_lexem = STRING;
  cursor    = pos;

  for (;;)
    switch (*pos)  
    {
      case '\0':
      case '\n':
      case '\r':
        SetError (UNCLOSED_STRING,cur_token);
        return;
      default:
        if (*pos++ == border)
        {
          pos [-1] = 0;
          return;
        }  
        break;
    }
}

void Lexer::read_identifier ()
{
  cur_token = pos;
  cur_lexem = IDENTIFIER;
  cursor    = pos;

  while (isalnum (*pos) || *pos == '_' || *pos == '.') pos++;

  switch (*pos)
  {
    case '{':
    case '}':
    case '(':
    case ')':
    case '/':
    case '\t':
    case ' ':
    case '\r':
    case '\n':
      erased_char     = *pos;
      erased_char_pos = pos;
      *pos = 0;
      break;
    default:
      SetError (WRONG_IDENTIFIER,cur_token);
      break;
  }
}

void Lexer::read_unsigned ()
{
  if (!*pos)
    return;

  if (!memcmp (pos,"0X",2) || !memcmp (pos,"0x",2))
    pos += 2;

  while (isdigit (*pos) || *pos == '.' || *pos == 'e') pos++;  
}

void Lexer::read_value ()
{
  cur_token = pos;
  cur_lexem = VALUE;
  cursor    = pos;

  switch (*pos)
  {
    case '+':
    case '-': pos++;
    default:  read_unsigned (); 
  }  

  switch (*pos)
  {      
    case '{':
    case '}':
    case '(':
    case ')':
    case '/':
    case '\t':
    case ' ':
    case '\r':
    case '\n':
      erased_char     = *pos;
      erased_char_pos = pos;
      *pos = 0;
      break;
    default:
      SetError (WRONG_VALUE,cur_token);
      break;
  }
}

Lexer::Lexem Lexer::next ()
{
  char *old_erased_char_pos = erased_char_pos;

  *erased_char_pos = erased_char;

  cur_lexem   = UNDEFINED;
  cur_error   = NO_LEX_ERROR;
  cur_token   = NULL;
  erased_char = ' ';
  erased_char_pos = &null_char;

  skip ();

  cursor = pos;

  if (cur_error == NO_LEX_ERROR)
  {
    switch (*pos)
    {
      case '\0':
        pos++;
        cur_lexem = END_OF_FILE;  
        break;
      case '{':  
        pos++;
        cur_lexem = BEGIN_FRAME;  
        break;
      case '}':  
        pos++;
        cur_lexem = END_FRAME;    
        break;
      case '(':  
        pos++;
        cur_lexem = BEGIN_BRACET; 
        break;
      case ')':  
        pos++;
        cur_lexem = END_BRACET;   
        break;
      case '\n':
        nextline ();
        cursor = ++pos;
        cur_lexem = NEW_LINE;
        break;
      case '"':
      case '\'':
        read_string (*pos++);
        break;  
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
      case '8': case '9': case '.': case '+':
      case '-':
        read_value ();
        break;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
      case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
      case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
      case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
      case 'Y': case 'Z':
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
      case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
      case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
      case 's': case 't': case 'u': case 'v': case 'w': case 'x':
      case 'y': case 'z':
      case '_':
        read_identifier ();
        break;
      default:
        SetError (WRONG_CHAR,pos++);
        break;
    }
  }    

  *old_erased_char_pos = 0;

  return cur_lexem;
}
