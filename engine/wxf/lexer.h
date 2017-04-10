#ifndef __WXF_I_LEXER_H__
#define __WXF_I_LEXER_H__

#include <mem.h>

//лексический анализатор
class Lexer
{
  public:
      //лексема
    enum Lexem {
      UNDEFINED,
      END_OF_FILE,
      NEW_LINE,
      VALUE,
      IDENTIFIER,
      STRING,
      BEGIN_BRACET,
      END_BRACET,
      BEGIN_FRAME,
      END_FRAME,
    };

      //ошибка
    enum Error {
      NO_LEX_ERROR,
      UNCLOSED_COMMENT,
      UNCLOSED_STRING,
      WRONG_CHAR,
      WRONG_IDENTIFIER,
      WRONG_VALUE
    };

    Lexer  (char* buf);

      //следующая лексема                
    Lexem next  ();

      //доступ
    Lexem lexem () { return cur_lexem; }
    Error error () { return cur_error; }
    int   line  () { return line_number; }
    int   offs  () { return cursor - line_start + 1; }

    char* token () { return cur_token; }    
    char  curchar () const { return cursor == erased_char_pos ? erased_char : *cursor; }

  private:
    void skip ();
    void skip_block_comment ();
    void skip_rest_of_line ();
    void nextline ();
    void read_string (char border);
    void read_identifier ();
    void read_value ();
    void read_unsigned ();
    void read_signed ();

    void SetError (Error err,char* _pos=NULL);

  private:    
    static char* null_ptr;
    static char  null_char;

    Lexem cur_lexem;
    Error cur_error;
    char  erased_char, *erased_char_pos;
    char* pos;
    char* buf;
    char* cur_token;
    char* line_start;
    char* cursor;
    int   line_number;
};

#endif
