/*!re2c re2c:flags:i = 0; */

/*!max:re2c*/
/*!re2c
    digit = [0-9];
    integer = digit+;
    spaces = [\t\n ]+;
    add = [+];
    sub = [-];
    mul = [*];
    div = [/];
    frac = [0-9]* "." [0-9]+ | [0-9]+ ".";
    exp = 'e' [+-]? [0-9]+;
    number = (frac exp? | [0-9]+ exp);
    end = [\x00];
*/

#include <bcParser.h>
#include <bcPrivate.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bcGetToken(const char* head, const char** tail, BC_VALUE* pData)
{
  // head - first character of new token
  // YYCURSOR - last character of new token

  const char* YYMARKER; // inner lexer variable is used when there can be longer string to match
  const char* YYCURSOR = head; // initialize cursor to first character position

GET_NEXT_TOKEN: // jump to this label, if processed token is skipped (like spaces)
  /*!re2c

    re2c:define:YYCTYPE = char;
    re2c:yyfill:enable = 0; // this disable interactive character read

    * {
      // Exit with error on any unknown symbols.
      fprintf(stderr, "Unknown Symbol: '%c' (0x%02x)\n", *head, *head);
      *tail = head;
      return 0;
    }

    end {
      // '\0' found, string ended.
      return 0;
    }

    add {
      // '+'
      *tail = YYCURSOR;
      return TOK_ADD;
    }

    sub {
      // '-'
      *tail = YYCURSOR;
      return TOK_SUB;
    }

    mul {
      // '*'
      *tail = YYCURSOR;
      return TOK_MUL;
    }
    
    div {
      // '/'
      *tail = YYCURSOR;
      return TOK_DIV;
    }

    spaces {
      // Skips any amount of spaces, tabs and newlines.
      head = YYCURSOR;
      goto GET_NEXT_TOKEN;
    }

    integer {
      // Simple C integer.

      char* tmpInteger = (char*) malloc((YYCURSOR - head) + 1);
      memcpy(tmpInteger, head, (YYCURSOR - head)); // copy token symbols to temp buffer
      tmpInteger[YYCURSOR - head] = 0;
      *pData = bcValueInteger(strtoll(tmpInteger, NULL, 10));
      // currently there are no checks for strtoll produced a valid integer.

      free(tmpInteger);

      *tail = YYCURSOR;
      return TOK_CONSTANT;
    }

    number {
      // Simple C float

      char* tmpNumber = (char*) malloc((YYCURSOR - head) + 1);
      memcpy(tmpNumber, head, (YYCURSOR - head)); // copy token symbols to temp buffer
      tmpNumber[YYCURSOR - head] = 0;
      *pData = bcValueNumber(strtod(tmpNumber, NULL));
      // currently there are no checks for strtod produced a valid number

      free(tmpNumber);

      *tail = YYCURSOR;
      return TOK_CONSTANT;
    }

  */

}
