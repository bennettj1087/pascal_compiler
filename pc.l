%{
#include <stdio.h>
#include <stdarg.h>
#include "tree.h"
#include "y.tab.h"

int lineno = 0;
extern char *filename;

void yyerror(char *str) {
	fprintf(stderr, "%s:%d: error: %s near '%s'\n", filename, lineno, str, yytext);
	exit(1);
}

void yywarn(char *str) {
	fprintf(stderr, "%s:%d: warning: %s near '%s'\n", filename, lineno, str, yytext);
}

void echo (char*);
void echo_c (char*, char);
void echo_s (char*, char*);
void echo_i (char*, int);
void echo_f (char*, float);
%}

%option		nounput

%x			COMMENTP COMMENTB

digit		[0-9]
letter		[a-zA-Z]
whitespace	[ \t]+
identifier	{letter}({letter}|{digit})*
number		{digit}+

%%

"(*"					{ BEGIN COMMENTP; }
<COMMENTP>"*)"			{ BEGIN INITIAL; /* return to normal */ }
<COMMENTP>.				; /* throw away comment text */
<COMMENTP>\n			{ lineno++; }

"{"						{ BEGIN COMMENTB; }
<COMMENTB>"}"			{ BEGIN INITIAL; /* return to normal */ }
<COMMENTB>.				; /* throw away comment text */
<COMMENTB>\n			{ lineno++; }

"program"				{ echo("[PROGRAM]");	return PROGRAM; }
"var"					{ echo("[VAR]"); 		return VAR; }
"array"					{ echo("[ARRAY]"); 		return ARRAY; }
"of"					{ echo("[OF]"); 		return OF; }
"integer"				{ echo("[INTEGER]"); 	return INTEGER; }
"real"					{ echo("[REAL]"); 		return REAL; }
"function"				{ echo("[FUNCTION]"); 	return FUNCTION; }
"procedure"				{ echo("[PROCEDURE]"); 	return PROCEDURE; }
"begin"					{ echo("[BEGIN]"); 		return BBEGIN; }
"end"					{ echo("[END]"); 		return END; }
"if"					{ echo("[IF]"); 		return IF; }
"then"					{ echo("[THEN]"); 		return THEN; }
"else"					{ echo("[ELSE]"); 		return ELSE; }
"while"					{ echo("[WHILE]"); 		return WHILE; }
"do"					{ echo("[DO]"); 		return DO; }
"not"					{ echo("[NOT]"); 		return NOT; }
"for"					{ echo("[FOR]");		return FOR;	}
"to"					{ echo("[TO]");			return TO;	}

":="					{ echo("[ASSIGNOP]"); 	return ASSIGNOP; }
".."					{ echo("[DOTDOT]"); 	return DOTDOT; }

"<"						{
							echo_s("[RELOP:%s]", yytext );
							yylval.opval = LT;
							return RELOP;
						}
"<="					{
							echo_s("[RELOP:%s]", yytext );
							yylval.opval = LE;
							return RELOP;
						}
">"						{
							echo_s("[RELOP:%s]", yytext );
							yylval.opval = GT;
							return RELOP;
						}
">="					{
							echo_s("[RELOP:%s]", yytext );
							yylval.opval = GE;
							return RELOP;
						}
"="						{
							echo_s("[RELOP:%s]", yytext );
							yylval.opval = EQ;
							return RELOP;
						}
"<>"					{
							echo_s("[RELOP:%s]", yytext );
							yylval.opval = NE;
							return RELOP;
						}

"+"						{
							echo_s("[ADDOP:%s]", yytext );
							yylval.opval = PLUS;
							return ADDOP;
						}
"-"						{
							echo_s("[ADDOP:%s]", yytext );
							yylval.opval = MINUS;
							return ADDOP;
						}
"or"					{
							echo_s("[ADDOP:%s]", yytext );
							yylval.opval = OR;
							return ADDOP;
						}

"*"						{
							echo_s("[MULOP:%s]", yytext );
							yylval.opval = STAR;
							return MULOP;
						}
"/"						{
							echo_s("[MULOP:%s]", yytext );
							yylval.opval = SLASH;
							return MULOP;
						}
"mod"					{
							echo_s("[MULOP:%s]", yytext );
							yylval.opval = MOD;
							return MULOP;
						}
"div"					{
							echo_s("[MULOP:%s]", yytext );
							yylval.opval = DIV;
							return MULOP;
						}
"and"					{
							echo_s("[MULOP:%s]", yytext );
							yylval.opval = AND;
							return MULOP;
						}

{identifier}			{
							echo_s("[IDENT:%s]", yytext );
							yylval.sval = strdup(yytext);
							return ID;
						}

{number}				{
							yylval.ival = atoi(yytext);
							echo_i("[INUM:%d]", yylval.ival);
							return INUM;
						}
{number}"."{number}		{
							yylval.rval = atof(yytext);
							echo_f("[RNUM:%f]", yylval.rval);
							return RNUM;
						}

{whitespace}			{ echo_s("%s", yytext); }
\n						{
							lineno++;
							echo_c("%c", yytext[0]);
						}
.						{
							echo_c("{%c}", yytext[0]);
							return yytext[0];
						}

%%

void echo( char *str ) {
	#ifdef SCANNER_DEBUG
	fprintf( stderr, str );
	#endif
}

void echo_c( char *str, char arg ) {
	#ifdef SCANNER_DEBUG
	fprintf( stderr, str, arg );
	#endif
}

void echo_s( char *str, char *arg ) {
	#ifdef SCANNER_DEBUG
	fprintf( stderr, str, arg );
	#endif
}

void echo_i( char *str, int arg ) {
	#ifdef SCANNER_DEBUG
	fprintf( stderr, str, arg );
	#endif
}

void echo_f( char *str, float arg ) {
	#ifdef SCANNER_DEBUG
	fprintf( stderr, str, arg );
	#endif
}
