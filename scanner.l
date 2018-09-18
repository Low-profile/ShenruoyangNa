%{
#include "scanner.h"
int cur_line = 1;
void yyerror(const char *msg);
void unrecognized_char(char c);

%}
%option noyywrap

/* note \042 is '"' */
WHITESPACE          ([ \t\r\a]+)
SINGLE_COMMENT1     ("//"[^\n]*)
SINGLE_COMMENT2     ("#"[^\n]*)
OPERATOR            ([\^+*-/%=,;!<>(){}:\[\]])
INTEGER             ([0-9]+)
IDENTIFIER          ([_a-zA-Z][_a-zA-Z0-9]*)
STRING              (\042[^\042\n]*\042)

%%

\n                  { cur_line++;  yylineno++; return T_NewLine;  }
{WHITESPACE}        { /* ignore every whitespace */ }
{SINGLE_COMMENT1}   { /* skip for single line comment */    }
{SINGLE_COMMENT2}   { /* skip for single line comment */    }
{OPERATOR}          { return yytext[0]; }

"int"               { return T_Int; }
"bool"              { return T_Bool; }
"array" 	        { return T_Array; }
"void"              { return T_Void; }
"return"            { return T_Return; }

"while"             { return T_While; }
"if"                { return T_If; }
"else"              { return T_Else; }
"for"		        { return T_For; }

"<="                { return T_Le; }
">="                { return T_Ge; }
"=="                { return T_Eq; }
"!="                { return T_Ne; }

"true"		        { return T_True; }
"false"		        { return T_False; }
"sizeof"	        { return T_SizeOf; }
"print"             { return T_Print; }



{INTEGER}           { return T_IntConstant; }
{STRING}            { return T_StringConstant; }
{IDENTIFIER}        { return T_Identifier; }
.                   { unrecognized_char(yytext[0]); }


%%

void yyerror(const char *msg) {
    fprintf(stderr, "Error at line %d:\n\t%s\n", cur_line, msg);
    exit(-1);
}

void unrecognized_char(char c) {
    char buf[32] = "Unrecognized character: ?";
    buf[24] = c;
    yyerror(buf);
}