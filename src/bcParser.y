%token_type {BC_VALUE}
%token_destructor { if ($$ != NULL) { bcValueCleanup($$); } }
%token_prefix TOK_
%extra_argument { bcCodeStream_t* cs }
%start_symbol program

%right SET.
%left LOR.
%left LND.
%left BOR.
%left XOR.
%left BND.
%nonassoc EQ NEQ.
%nonassoc GR GRE LS LSE.
%left BLS BRS.
%left ADD SUB.
%left DIV MUL MOD.
%right LNOT BNOT.
%right INT NUM STR.

%include {
  #include <bcPrivate.h>

  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
}

%syntax_error {
  fprintf(stderr, "Syntax Error!\n");
}

program ::= .     { bcCodeStreamAppendOpcode(cs, BC_HALT); }
program ::= expr. { bcCodeStreamAppendOpcode(cs, BC_HALT); }

expr ::= ID SET expr. { bcCodeStreamAppendOpcode(cs, BC_POP); }
expr ::= expr LOR expr. { bcCodeStreamAppendOpcode(cs, BC_LOR); }
expr ::= expr LND expr. { bcCodeStreamAppendOpcode(cs, BC_LND); }
expr ::= expr BOR expr. { bcCodeStreamAppendOpcode(cs, BC_BOR); }
expr ::= expr XOR expr. { bcCodeStreamAppendOpcode(cs, BC_XOR); }
expr ::= expr BND expr. { bcCodeStreamAppendOpcode(cs, BC_BND); }
expr ::= expr EQ  expr. { bcCodeStreamAppendOpcode(cs, BC_EQ);  }
expr ::= expr NEQ expr. { bcCodeStreamAppendOpcode(cs, BC_NEQ); }
expr ::= expr GR  expr. { bcCodeStreamAppendOpcode(cs, BC_GR);  }
expr ::= expr GRE expr. { bcCodeStreamAppendOpcode(cs, BC_GRE); }
expr ::= expr LS  expr. { bcCodeStreamAppendOpcode(cs, BC_LS);  }
expr ::= expr LSE expr. { bcCodeStreamAppendOpcode(cs, BC_LSE); }
expr ::= expr BLS expr. { bcCodeStreamAppendOpcode(cs, BC_BLS); }
expr ::= expr BRS expr. { bcCodeStreamAppendOpcode(cs, BC_BRS); }
expr ::= expr SUB expr. { bcCodeStreamAppendOpcode(cs, BC_SUB); }
expr ::= expr ADD expr. { bcCodeStreamAppendOpcode(cs, BC_ADD); }
expr ::= expr MUL expr. { bcCodeStreamAppendOpcode(cs, BC_MUL); }
expr ::= expr DIV expr. { bcCodeStreamAppendOpcode(cs, BC_DIV); }
expr ::= expr MOD expr. { bcCodeStreamAppendOpcode(cs, BC_MOD); }
expr ::= OPENBR expr CLOSEBR. 
expr ::= LNOT expr.               { bcCodeStreamAppendOpcode(cs, BC_LNT); }
expr ::= BNOT expr.               { bcCodeStreamAppendOpcode(cs, BC_BNT); }
expr ::= SUB expr. [LNOT]         { bcCodeStreamAppendOpcode(cs, BC_NEG); }
expr ::= OPENBR INT CLOSEBR expr. { bcCodeStreamAppendOpcode(cs, BC_INT); }
expr ::= OPENBR NUM CLOSEBR expr. { bcCodeStreamAppendOpcode(cs, BC_NUM); }
expr ::= OPENBR STR CLOSEBR expr. { bcCodeStreamAppendOpcode(cs, BC_STR); }

expr ::= CONSTANT(VALUE). {
  uint8_t conCode;

  bcCodeStreamAppendConstant(cs, VALUE, &conCode);
  bcCodeStreamAppendOpcode(cs, BC_PSH);
  bcCodeStreamAppendOpcode(cs, conCode);
  bcValueCleanup(VALUE);
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

    BC_VALUE tmptok = NULL;
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
      Parse(parser, tok, bcValueCopy(tmptok), &tempCodeStream);
      bcValueCleanup(tmptok);
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