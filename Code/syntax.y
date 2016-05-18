%locations
%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

/*typedef struct Node{
	union {
			int type_int;
			float type_float;
			char type_char[32];
	};
	char* name;
	int line;
	int node_type;
	struct Node* children;
	struct Node* sibling;
}Node; */

Node* root = NULL;
Node* create_new_node(char* name,int location,int node_type);
static int is_error = 0;
void create_tree(Node* father,Node* child);
void print_tree(Node* root,int level);
void print();
void enter_queue(Node* p);

%}
 
%union{
	int type_int;
	float type_float;
	struct Node* node;
}

%token <node> INT
%token <node> FLOAT
%token <node> ID
%token <node> SEMI COMMA
%token <node> ASSIGNOP RELOP
%token <node> PLUS MINUS STAR DIV AND OR
%token <node> DOT NOT
%token <node> TYPE
%token <node> LP RP LB RB LC RC
%token <node> STRUCT RETURN IF ELSE WHILE

%type <node>
	Program ExtDefList ExtDef Specifier ExtDecList Exp Args VarDec FunDec CompSt
	StructSpecifier OptTag Tag VarList ParamDec StmtList Stmt DefList Def Dec
	DecList 

%nonassoc LOW_THAN_ID
%nonassoc ID

%right ASSIGNOP
%left  AND OR
%left  RELOP
%left  PLUS MINUS
%left  STAR DIV
%right UMINUS
%left  LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/*High-level Definitions*/
Program	: ExtDefList {$$ = create_new_node("Program",@$.first_line,-1);
					 create_tree($$,$1);
					 root = $$;}
		;
ExtDefList	: ExtDef ExtDefList {$$ = create_new_node("ExtDefList",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			}
		| /*empty*/{$$ = create_new_node("ExtDefList",@$.first_line,-1);}
		;
ExtDef	: Specifier ExtDecList SEMI	{$$ = create_new_node("ExtDef",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
		| Specifier SEMI	{$$ = create_new_node("ExtDef",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}
		| Specifier FunDec CompSt	{$$ = create_new_node("ExtDef",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
		| error SEMI{is_error = 1; yyerror("Missing \";\"");yyerrok;}
		;
ExtDecList	: VarDec {$$ = create_new_node("ExtDecList",@$.first_line,-1);
			create_tree($$,$1);}
		| VarDec COMMA ExtDecList{$$ = create_new_node("ExtDecList",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
		;


/*Specifiers*/
Specifier	: TYPE {$$ = create_new_node("Specifier",@$.first_line,-1);
				create_tree($$,$1);}
			| StructSpecifier{$$ = create_new_node("Specifier",@$.first_line,-1);
				create_tree($$,$1);}
			;
StructSpecifier	: STRUCT OptTag LC DefList RC{$$ = create_new_node("StructSpecifier",@$.first_line,-1);
				create_tree($$,$1);
				create_tree($$,$2);
				create_tree($$,$3);
				create_tree($$,$4);
				create_tree($$,$5);}
				| STRUCT Tag {$$ = create_new_node("StructSpecifier",@$.first_line,-1);
				create_tree($$,$1);
				create_tree($$,$2);}
				;
OptTag	: ID {$$ = create_new_node("OptTag",@$.first_line,-1);
			create_tree($$,$1);}
	    | /*empty*/{$$ = create_new_node("OptTag",@$.first_line,-1);}
		;
Tag	: ID {$$ = create_new_node("Tag",@$.first_line,-1);
			create_tree($$,$1);}
	;


/*Declarators*/
VarDec	: ID {$$ = create_new_node("VarDec",@$.first_line,-1);
			create_tree($$,$1);}
	   | VarDec LB INT RB {$$ = create_new_node("VarDec",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);}
	   | error RB {is_error = 1; yyerror("Missing \"]\"");yyerrok;}
			;
FunDec	: ID LP VarList RP {$$ = create_new_node("FunDec",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);}
	   | ID LP RP {$$ = create_new_node("FunDec",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
			;
VarList	: ParamDec COMMA VarList {$$ = create_new_node("VarList",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	   | ParamDec {$$ = create_new_node("VarList",@$.first_line,-1);
			create_tree($$,$1);}
			;
ParamDec : Specifier VarDec {$$ = create_new_node("ParamDec",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}
			;


/*Statements*/
CompSt	: LC DefList StmtList RC {$$ = create_new_node("CompSt",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);}
			; 
StmtList : Stmt StmtList{$$ = create_new_node("StmtList",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}
		 | /*empty*/{$$ = create_new_node("StmtList",@$.first_line,-1);}
			;
Stmt	: Exp SEMI {$$ = create_new_node("Stmt",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}
		| CompSt {$$ = create_new_node("Stmt",@$.first_line,-1);
			create_tree($$,$1);}
		| RETURN Exp SEMI {$$ = create_new_node("Stmt",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
		| error SEMI{is_error=1; yyerror("Missing \";\"");yyerrok;}
		| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = create_new_node("Stmt",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);
			create_tree($$,$5);}
		| IF LP Exp RP Stmt ELSE Stmt {$$ = create_new_node("Stmt",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);
			create_tree($$,$5);
			create_tree($$,$6);
			create_tree($$,$7);}
		| WHILE LP Exp RP Stmt {$$ = create_new_node("Stmt",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);
			create_tree($$,$5);}
		| error RP {is_error=1;}
			;


/*Local Definition*/
DefList	: Def DefList {$$ = create_new_node("DefList",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}
		| /*empty*/{$$ = create_new_node("DefList",@$.first_line,-1);}
			;
Def	: Specifier DecList SEMI{$$ = create_new_node("Def",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
		| 	error SEMI{is_error=1;yyerror("Missing ;");}
			;
DecList	: Dec {$$ = create_new_node("DecList",@$.first_line,-1);
			create_tree($$,$1);}
		| Dec COMMA DecList {$$ = create_new_node("DecList",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
		;
Dec	: VarDec {$$ = create_new_node("Dec",@$.first_line,-1);
		create_tree($$,$1);}
	| VarDec ASSIGNOP Exp{$$ = create_new_node("Dec",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| VarDec ASSIGNOP error {is_error=1;yyerrok;}	
	;


/*Expressions*/
Exp	: Exp ASSIGNOP Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp AND Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp OR Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp RELOP Exp{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp PLUS Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp MINUS Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp STAR Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| Exp DIV Exp	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| LP Exp RP		{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| MINUS Exp	%prec UMINUS	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}		
	| NOT Exp	%prec UMINUS	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);}
	| ID LP Args RP	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);}
	| ID LP RP	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}	
	| Exp LB Exp RB	 {$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);
			create_tree($$,$4);}
	| Exp LB error RB{is_error=1; yyerror("Missing \"]\"");yyerrok;}
	| Exp DOT ID	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);
			create_tree($$,$2);
			create_tree($$,$3);}
	| ID %prec LOW_THAN_ID		{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);}	
	| INT	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);}	
	| FLOAT	{$$ = create_new_node("Exp",@$.first_line,-1);
			create_tree($$,$1);	}
	;
Args : Exp COMMA Args	{$$ = create_new_node("Args",@$.first_line,-1);
		create_tree($$,$1);
		create_tree($$,$2);
		create_tree($$,$3);}	
	 | Exp	{$$ = create_new_node("Args",@$.first_line,-1);
		create_tree($$,$1);}	
	;
%%
#include "lex.yy.c"

yyerror(char* msg){
	fprintf(stderr,"Error type B at line %d: %s\n",yylineno,msg);	
}

Node* create_new_node(char* name,int location,int node_type){
	//printf("creating a new node %s(%d)\n",name,yylineno);
	Node* p = (Node*)malloc(sizeof(Node));
	p->name = name;
	p->line = location;
	p->node_type = node_type;
	p->sibling = NULL;
	p->children = NULL;
	return p;
}

void create_tree(Node* father,Node* child){
	if(father->children == NULL){
		father->children = child;
		//printf("1.%s\n",father->children->name);
	}
	else{
		Node* temp = father->children;
		//printf("%s,%s\n",temp->name,temp->type_char );
		while(temp->sibling != NULL){

			temp = temp->sibling;
		}
		temp->sibling = child;
	}
}

void print_tree(Node* root,int level){
	int i,j;
	if(root->node_type == -1 && root->children == NULL)
		return;
	for(j = 0;j < level*2;j++)
		printf(" ");
	if(root->node_type != -1){
		if(root->node_type == ID)
			printf("ID: %s\n",root->type_char);
		else if(root->node_type == TYPE)
			printf("TYPE: %s\n",root->type_char);
		else if(root->node_type == INT)
			printf("INT: %d\n",root->type_int);
		else if(root->node_type == FLOAT)
			printf("FLOAT: %f\n",root->type_float);
		else
			printf("%s\n",root->name);
	}
	else
		printf("%s (%d)\n",root->name,root->line);
	if(root->children != NULL){
		print_tree(root->children,level+1);
		Node* temp = root->children;
		while(temp->sibling != NULL){
			print_tree(temp->sibling,level+1);
			temp = temp->sibling;
		}
	}
}

void delete_tree(Node* root){
	//init_queue_head();
	if(root->children == NULL){
		//printf("%s\n",root->name);
		free(root);
	}
	else{
		//enter_queue(root);
		Node* temp = root;
		root = root->children;
		//printf("%s\n",temp->name);
		free(temp);
		
		temp = root;
		root = root->sibling;
		delete_tree(temp);
		while(root != NULL){
			Node* temp1 = root;
			root = root->sibling;
			delete_tree(temp1);
		}
	}
}

void print(){
	//printf("%d\n", is_error);
	if(root != NULL){
		if(is_error == 0)
			print_tree(root,0);
		//delete_tree(root);
	}
}

Node* get_root(){
	return root;
}

