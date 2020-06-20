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
    mod = [%];
    eq = [=];
    neq = [!][=];
    gr = [>];
    ls = [<];
    gre = [>][=];
    lse = [<][=];
    lnd = [&][&];
    lor = [|][|];
    bnd = [&];
    bor = [|];
    xor = [\^];
    bls = [<][<];
    brs = [>][>];
    openbr = [(];
    closebr = [)];
    lnot = [!];
    bnot = [~];
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

    lnot {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_LNOT;
    }

    bnot {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_BNOT;
    }

    openbr {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_OPENBR;
    }

    closebr {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_CLOSEBR;
    }

    end {
      // '\0' found, string ended.
      *pData = NULL;
      return 0;
    }

    add {
      // '+'
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_ADD;
    }

    sub {
      // '-'
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_SUB;
    }

    mul {
      // '*'
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_MUL;
    }
    
    div {
      // '/'
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_DIV;
    }

    mod {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_MOD;
    }

    eq  {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_EQ;
    }

    neq {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_NEQ;
    }

    gr {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_GR;
    }

    ls {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_LS;
    }

    gre {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_GRE;
    }

    lse {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_LSE;
    }

    lnd {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_LND;
    }

    lor {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_LOR;
    }

    bnd {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_BND;
    }

    bor {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_BOR;
    }

    xor {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_XOR;
    }

    bls {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_BLS;
    }

    brs {
      *tail = YYCURSOR;
      *pData = NULL;
      return TOK_BRS;
    }

    spaces {
      // Skips any amount of spaces, tabs and newlines.
      head = YYCURSOR;
      goto GET_NEXT_TOKEN;
    }

    integer {
      // Simple C integer.

      char* tmpInteger = (char*) malloc((size_t)((YYCURSOR - head) + 1));
      memcpy(tmpInteger, head, (size_t)(YYCURSOR - head)); // copy token symbols to temp buffer
      tmpInteger[YYCURSOR - head] = 0;
      *pData = bcValueInteger(strtoll(tmpInteger, NULL, 10));
      // currently there are no checks for strtoll produced a valid integer.

      free(tmpInteger);

      *tail = YYCURSOR;
      return TOK_CONSTANT;
    }

    number {
      // Simple C float

      char* tmpNumber = (char*) malloc((size_t)((YYCURSOR - head) + 1));
      memcpy(tmpNumber, head, (size_t)(YYCURSOR - head)); // copy token symbols to temp buffer
      tmpNumber[YYCURSOR - head] = 0;
      *pData = bcValueNumber(strtod(tmpNumber, NULL));
      // currently there are no checks for strtod produced a valid number

      free(tmpNumber);

      *tail = YYCURSOR;
      return TOK_CONSTANT;
    }

  */

}
