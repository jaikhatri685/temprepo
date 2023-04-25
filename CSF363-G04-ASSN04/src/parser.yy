%define api.value.type { ParserValue }

%code requires {
#include <iostream>
#include <vector>
#include <string>

#include "parser_util.hh"
#include "symbol.hh"

}

%code {

#include <cstdlib>

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTable symbol_table;

int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT TTYPE
%token INT TLET TDBG TIF TELSE TCOMMA TRET TFUN
%token TSCOL TLPAREN TRPAREN TEQUAL TLBRACE TRBRACE
%token TQUES TCOL

%type <node> Expr Stmt LitExpr FuncInp Func CallInp
%type <stmts> Program StmtList

%left TQUES TCOL
%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | StmtList 
        { final_values = $1; }
	    ;

StmtList : Stmt                
        { $$ = new NodeStmts(); $$->push_back($1); }
	    | StmtList Stmt 
        { $$->push_back($2); }
	    ;

Stmt : TLET TIDENT TCOL TTYPE TEQUAL Expr TSCOL
     {
        $$ = new NodeAssn($2, $6, $4);
     }
     | TDBG Expr TSCOL
     {  
        $$ = new NodeDebug($2);
     }
     | TIDENT TEQUAL Expr TSCOL
     {
        $$  = new NodeReAssn($1, $3);
     }
     | TIF Expr TLBRACE StmtList TELSE StmtList TRBRACE
     {
        $$ = new NodeIf($2, $4, $6);
     }
     | TIF LitExpr TLBRACE StmtList TELSE StmtList TRBRACE
     {
       int value = ((NodeInt *)$2)->value;
       if(value == 0){
         $$ = $6;
       }
       else{
         $$ = $4;
       }
     }
     | Func
     {
      $$ = $1;
     }
     | TRET Expr TSCOL
     { $$ = new NodeRet($2);}
     ;

LitExpr: TINT_LIT               
     { $$ = new NodeInt(stoi($1)); }
     | LitExpr TPLUS LitExpr
     { $$ = new NodeInt(((NodeInt *)$1)->value + ((NodeInt *)$3)->value); }
     | LitExpr TDASH LitExpr
     { $$ = new NodeInt(((NodeInt *)$1)->value - ((NodeInt *)$3)->value); }
     | LitExpr TSTAR LitExpr
     { $$ = new NodeInt(((NodeInt *)$1)->value * ((NodeInt *)$3)->value); }
     | LitExpr TSLASH LitExpr
     { $$ = new NodeInt(((NodeInt *)$1)->value / ((NodeInt *)$3)->value); }
     ;

Expr : TIDENT
     { 
        $$ = new NodeIdent($1); 
     }
     | LitExpr
     {
         $$ = new NodeInt(((NodeInt *)$1)->value);
     }
     | Expr TQUES Expr TCOL Expr  {
        $$ = new NodeTerOp($1, $3, $5);
     }
     | Expr TPLUS Expr
     { $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); }
     | Expr TDASH Expr
     { $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); }
     | Expr TSTAR Expr
     { $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); }
     | Expr TSLASH Expr
     { $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); }
     | TLPAREN Expr TRPAREN 
     { $$ = $2; }
     | TIDENT TLPAREN CallInp TRPAREN
     { 
         $$ = new NodeCall(((NodeCallIn *)$3)->expressions, $1); 
     }
     ;

CallInp : Expr
      {$$ = new NodeCallIn(); ((NodeCallIn *)$$)->expressions.push_back($1);}
      | CallInp TCOMMA Expr
      {((NodeCallIn *)$$)->expressions.push_back($3);}
      ;

FuncInp : TIDENT TCOL TIDENT
      {$$ = new NodeFuncIn(); ((NodeFuncIn *)$$)->inputs.push_back({$1, $3});}
      | FuncInp TCOMMA TIDENT TCOL TIDENT
      {((NodeFuncIn *)$$)->inputs.push_back({$3, $5});}
      ;

Func : TFUN TIDENT TLPAREN FuncInp TRPAREN TCOL TIDENT TLBRACE StmtList TRBRACE
      {
         $$ = new NodeFunction($9, ((NodeFuncIn *)$4)->inputs, $7, $2);
      }
      | TFUN TIDENT TLPAREN TRPAREN TCOL TIDENT TLBRACE StmtList TRBRACE
      {
         $$ = new NodeFunction($8, {}, $6, $2);
      }
      ;
%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}