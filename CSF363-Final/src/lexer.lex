%option noyywrap

%x DECLARATION_ID
%x DECLARATION_TY

%{
#include "parser.hh"
#include <string>

extern int yyerror(std::string msg);

%}

%%
<INITIAL>{
    "+"       {return TPLUS; }
    "-"       { return TDASH; }
    "*"       { return TSTAR; }
    "/"       { return TSLASH; }
    ";"       { return TSCOL; }
    "("       { return TLPAREN; }
    ")"       { return TRPAREN; }
    "="       { return TEQUAL; }
    "?"       { return TQUES;  }
    ":"       { return TCOL;  }
    ","       { return TCOMMA; }
    ret       { return TRET; }
    fun       { return TFUN; }
    if        { return TIF;}
    dbg       { return TDBG; }
    \{        { return TLBRACE; }
    \}[ \t\n]*else[ \t]*\{    {return TELSE;}
    \}        { return TRBRACE;}
    let       { BEGIN(DECLARATION_ID); return TLET; }
    [0-9]+    { yylval.lexeme = std::string(yytext); return TINT_LIT; }
    [A-Za-z_][A-Za-z_0-9]* { yylval.lexeme = std::string(yytext); return TIDENT; }
    [ \t\n]   { /* skip */}
    "\0"    {return 0;}

    .         { yyerror("unknown char");} 
}

<DECLARATION_ID>{
    [A-Za-z_][A-Za-z_0-9]*  {
        yylval.lexeme = std::string(yytext); return TIDENT;
    }

    [ \t]   { /* skip */ }

    ":" {
        BEGIN(DECLARATION_TY);
        return TCOL;
        }

    .         { yyerror("unknown char");} 
}

<DECLARATION_TY>{
    [A-Za-z_][A-Za-z_0-9]*  {
        yylval.lexeme = std::string(yytext); 
        BEGIN(INITIAL);
        return TTYPE;
    }

    [ \t]   { /* skip */ }

    .         { yyerror("unknown char");} 
}

%%

std::string token_to_string(int token, const char *lexeme) {
    std::string s;
    switch (token) {
        case TPLUS: s = "TPLUS"; break;
        case TDASH: s = "TDASH"; break;
        case TSTAR: s = "TSTAR"; break;
        case TSLASH: s = "TSLASH"; break;
        case TSCOL: s = "TSCOL"; break;
        case TLPAREN: s = "TLPAREN"; break;
        case TRPAREN: s = "TRPAREN"; break;
        case TEQUAL: s = "TEQUAL"; break;
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;
        
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;
    }

    return s;
}