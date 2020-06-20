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
program ::= rightExpr. { bcCodeStreamAppendOpcode(cs, BC_HALT); }

rightExpr ::= leftExpr SET rightExpr.  { bcCodeStreamAppendOpcode(cs, BC_SET); }
rightExpr ::= rightExpr LOR rightExpr. { bcCodeStreamAppendOpcode(cs, BC_LOR); }
rightExpr ::= rightExpr LND rightExpr. { bcCodeStreamAppendOpcode(cs, BC_LND); }
rightExpr ::= rightExpr BOR rightExpr. { bcCodeStreamAppendOpcode(cs, BC_BOR); }
rightExpr ::= rightExpr XOR rightExpr. { bcCodeStreamAppendOpcode(cs, BC_XOR); }
rightExpr ::= rightExpr BND rightExpr. { bcCodeStreamAppendOpcode(cs, BC_BND); }
rightExpr ::= rightExpr EQ  rightExpr. { bcCodeStreamAppendOpcode(cs, BC_EQ);  }
rightExpr ::= rightExpr NEQ rightExpr. { bcCodeStreamAppendOpcode(cs, BC_NEQ); }
rightExpr ::= rightExpr GR  rightExpr. { bcCodeStreamAppendOpcode(cs, BC_GR);  }
rightExpr ::= rightExpr GRE rightExpr. { bcCodeStreamAppendOpcode(cs, BC_GRE); }
rightExpr ::= rightExpr LS  rightExpr. { bcCodeStreamAppendOpcode(cs, BC_LS);  }
rightExpr ::= rightExpr LSE rightExpr. { bcCodeStreamAppendOpcode(cs, BC_LSE); }
rightExpr ::= rightExpr BLS rightExpr. { bcCodeStreamAppendOpcode(cs, BC_BLS); }
rightExpr ::= rightExpr BRS rightExpr. { bcCodeStreamAppendOpcode(cs, BC_BRS); }
rightExpr ::= rightExpr SUB rightExpr. { bcCodeStreamAppendOpcode(cs, BC_SUB); }
rightExpr ::= rightExpr ADD rightExpr. { bcCodeStreamAppendOpcode(cs, BC_ADD); }
rightExpr ::= rightExpr MUL rightExpr. { bcCodeStreamAppendOpcode(cs, BC_MUL); }
rightExpr ::= rightExpr DIV rightExpr. { bcCodeStreamAppendOpcode(cs, BC_DIV); }
rightExpr ::= rightExpr MOD rightExpr. { bcCodeStreamAppendOpcode(cs, BC_MOD); }
rightExpr ::= OPENBR rightExpr CLOSEBR. 
rightExpr ::= LNOT rightExpr.               { bcCodeStreamAppendOpcode(cs, BC_LNT); }
rightExpr ::= BNOT rightExpr.               { bcCodeStreamAppendOpcode(cs, BC_BNT); }
rightExpr ::= SUB rightExpr. [LNOT]         { bcCodeStreamAppendOpcode(cs, BC_NEG); }
rightExpr ::= OPENBR INT CLOSEBR rightExpr. { bcCodeStreamAppendOpcode(cs, BC_INT); }
rightExpr ::= OPENBR NUM CLOSEBR rightExpr. { bcCodeStreamAppendOpcode(cs, BC_NUM); }
rightExpr ::= OPENBR STR CLOSEBR rightExpr. { bcCodeStreamAppendOpcode(cs, BC_STR); }

rightExpr ::= CONSTANT(VALUE). {
  uint8_t conCode;

  bcCodeStreamAppendConstant(cs, VALUE, &conCode);
  bcCodeStreamAppendOpcode(cs, BC_PSH);
  bcCodeStreamAppendOpcode(cs, conCode);
  bcValueCleanup(VALUE);
}

rightExpr ::= leftExpr. {
  bcCodeStreamAppendOpcode(cs, BC_VAL);
}

leftExpr ::= ID(NAME). {
  uint8_t conCode;

  bcCodeStreamAppendConstant(cs, NAME, &conCode);
  bcCodeStreamAppendOpcode(cs, BC_PSH);
  bcCodeStreamAppendOpcode(cs, conCode);
  bcValueCleanup(NAME);
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