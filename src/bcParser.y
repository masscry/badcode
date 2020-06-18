%token_type {BC_VALUE}
%token_destructor { if ($$ != NULL) { bcValueCleanup($$); } }
%token_prefix TOK_
%extra_argument { bcCodeStream_t* cs }
%start_symbol program

%left ADD SUB.
%left DIV MUL.

%include {
  #include <bcPrivate.h>

  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
}

%syntax_error {
  fprintf(stderr, "Syntax Error!\n");
}

program ::= expr. { bcCodeStreamAppendOpcode(cs, BC_HALT); }

expr ::= expr SUB expr. { bcCodeStreamAppendOpcode(cs, BC_SUB); }
expr ::= expr ADD expr. { bcCodeStreamAppendOpcode(cs, BC_ADD); }
expr ::= expr MUL expr. { bcCodeStreamAppendOpcode(cs, BC_MUL); }
expr ::= expr DIV expr. { bcCodeStreamAppendOpcode(cs, BC_DIV); }

expr ::= CONSTANT(VALUE). {
  uint8_t conCode;

  bcCodeStreamAppendConstant(cs, bcValueCopy(VALUE), &conCode);
  bcCodeStreamAppendOpcode(cs, BC_PSH);
  bcCodeStreamAppendOpcode(cs, conCode);
}

%code {

  bcStatus_t bcParseString(const char* str, bcCodeStream_t* codeStream, char** endp)
  {
    if ((codeStream == NULL) || (str == NULL))
    {
      return BC_INVALID_ARG;
    }

    bcCodeStream_t tempCodeStream;

    bcStatus_t status = bcCodeStreamInit(&tempCodeStream);
    if (status != BC_OK)
    {
      if (endp != NULL)
      {
        *endp = (char*) str;
      }
      return status;
    }

    void* parser = ParseAlloc(malloc);
    if (parser == NULL)
    {
      bcCodeStreamCleanup(&tempCodeStream);
      if (endp != NULL)
      {
        *endp = (char*) str;
      }
      return BC_NO_MEMORY;
    }

    BC_VALUE tmptok;
    const char* cursor = str;

    //
    // I didn't found info on how properly raise internal errors from parser,
    // Internet says, that parsing context must have some error flags when 
    // some internal error happened and break on parsing in outer loop.
    //
    // So, now we just don't do any checks.
    //
    // Later, I'll add them.
    //

    for(int tok = bcGetToken(cursor, &cursor, &tmptok); tok != 0; tok = bcGetToken(cursor, &cursor, &tmptok))
    {
      Parse(parser, tok, tmptok, &tempCodeStream);
      tmptok = NULL;
    }

    // When no more tokens are available, we need to give parser to know about it.
    Parse(parser, 0, 0, &tempCodeStream);

    //
    // Here parser done it's job and dies
    //
    ParseFree(parser, free);

    *codeStream = tempCodeStream;
    if (endp != NULL)
    {
      *endp = (char*) cursor;
    }
    return BC_OK;
  }

}