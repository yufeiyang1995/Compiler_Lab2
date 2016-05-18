%{
	#include "syntax.tab.h"
	#include <string.h>
	/*LT,LE,EQ,NE,GT,GE,IF,ELSE,INT,FLOAT,
	TYPE,ID,SEMI,COMMA,ASSIGNOP,RELOP,PLUS,MINUS,
	STAR,DIV,AND,OR,DOT,NOT,LP,RP,LB,RB,LC,RC,
	STRUCT,RETURN,WHILE;*/
	int yycolumn = 1;
	Node* create_new_node(char* name,int location,int node_type);
	#define YY_USER_ACTION \
	yylloc.first_line = yylloc.last_line = yylineno; \
	yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng - 1; \
	yycolumn += yyleng;
%}

%option yylineno

delim	[ \t\r\n]
ws	{delim}+
letter	[_A-Za-z]
digit	[0-9]
digit1	[1-9]
id	{letter}({letter}|{digit}|_)*
integer  {digit}+
ffloat	 [0-9]+\.?[0-9]+
octal	0[0-7]+
not_octal 0[0-9]+
hex	0[x|X][0-9a-fA-F]+
not_hex 0[x|X][0-9a-zA-Z]+

%%

"\r\n" { yycolumn = 1;}
{ws}	{}
if	{yylval.node = create_new_node("IF",yylineno,IF); return IF;}
else	{yylval.node = create_new_node("ELSE",yylineno,ELSE); return ELSE;}
struct	{yylval.node = create_new_node("STRUCT",yylineno,STRUCT); return STRUCT;}
while	{yylval.node = create_new_node("WHILE",yylineno,WHILE); return WHILE;}
return  {yylval.node = create_new_node("RETURN",yylineno,RETURN); return RETURN;}
int		{yylval.node = create_new_node("TYPE",yylineno,TYPE); 
		strcpy(yylval.node->type_char,yytext); return TYPE;}
float	{yylval.node = create_new_node("TYPE",yylineno,TYPE); 
		strcpy(yylval.node->type_char,yytext); return TYPE;}
{id}		{yylval.node = create_new_node("ID",yylineno,ID);
	strcpy(yylval.node->type_char,yytext); return ID;}
{hex}	{yylval.node = create_new_node("INT",yylineno,INT);
		yylval.node->type_int = sixteen_to_ten(yytext); return INT;}
{not_hex} {printf("Error type A at Line %d: Wrong INT \'%s\'\n",yylineno,yytext);}
{octal} {yylval.node = create_new_node("INT",yylineno,INT);
		yylval.node->type_int = eight_to_ten(yytext); return INT;}
{not_octal} {printf("Error type A at Line %d: Wrong INT \'%s\'\n",yylineno,yytext);}
{integer}	{yylval.node = create_new_node("INT",yylineno,INT);
			yylval.node->type_int = atoi(yytext); return INT;}
{ffloat}	{yylval.node = create_new_node("FLOAT",yylineno,FLOAT);
			yylval.node->type_float = atof(yytext);return FLOAT;}
";"	{yylval.node = create_new_node("SEMI",yylineno,SEMI);return SEMI;}
","	{yylval.node = create_new_node("COMMA",yylineno,COMMA);return COMMA;}
"="	{yylval.node = create_new_node("ASSIGNOP",yylineno,ASSIGNOP);return ASSIGNOP;}
"<"	{yylval.node = create_new_node("RELOP",yylineno,SEMI);return RELOP;}
"<="	{yylval.node = create_new_node("RELOP",yylineno,RELOP); return RELOP;}
"=="	{yylval.node = create_new_node("RELOP",yylineno,RELOP); return RELOP;}
"!="	{yylval.node = create_new_node("RELOP",yylineno,RELOP); return RELOP;}
">"	{yylval.node = create_new_node("RELOP",yylineno,RELOP); return RELOP;}
">="	{yylval.node = create_new_node("RELOP",yylineno,RELOP); return RELOP;}
"+"	{yylval.node = create_new_node("PLUS",yylineno,PLUS);return PLUS;}
"-"	{yylval.node = create_new_node("MINUS",yylineno,MINUS);return MINUS;}
"*"	{yylval.node = create_new_node("STAR",yylineno,STAR);return STAR;}
"/"	{yylval.node = create_new_node("DIV",yylineno,DIV);return DIV;}
"&&"	{yylval.node = create_new_node("AND",yylineno,AND);return AND;}
"||"	{yylval.node = create_new_node("OR",yylineno,OR);return OR;}
"."	{yylval.node = create_new_node("DOT",yylineno,DOT);return DOT;}
"!"	{yylval.node = create_new_node("NOT",yylineno,NOT);return NOT;}
"("	{yylval.node = create_new_node("LP",yylineno,LP);return LP;}
")"	{yylval.node = create_new_node("RP",yylineno,RP);return RP;}
"["	{yylval.node = create_new_node("LB",yylineno,LB);return LB;}
"]"	{yylval.node = create_new_node("RB",yylineno,RB);return RB;}
"{"	{yylval.node = create_new_node("LC",yylineno,LC);return LC;}
"}"	{yylval.node = create_new_node("RC",yylineno,RC); return RC;}
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno,yytext);}


%%
int eight_to_ten(char* s){
	int len = strlen(s);
	int sum = 0;
	int i = 0;
	for(i = 1;i < len;i++){
		sum = sum * 8 + s[i] - '0';
	}
	return sum;
}

int sixteen_to_ten(char* s){
	int len = strlen(s);
	int sum = 0;
	int  i =0;
	for(i = 2;i < len;i++){
		if(s[i] >= '0' && s[i] <= '9')
			sum = sum * 16 + s[i] - '0';
		else{
			switch(s[i]){
				case 'a':case 'A':sum = sum * 16 + 10;break;
				case 'b':case 'B':sum = sum * 16 + 11;break;
				case 'c':case 'C':sum = sum * 16 + 12;break;
				case 'd':case 'D':sum = sum * 16 + 13;break;
				case 'e':case 'E':sum = sum * 16 + 14;break;
				case 'f':case 'F':sum = sum * 16 + 15;break;
				default:break;
			}

		}
	}
	return sum;
}

/*int installID(){
	
}

int installInt(){

}

int installFloat(){
	
}*/

