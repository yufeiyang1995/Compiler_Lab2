#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "node.h"


//typedef struct Type* Type;
typedef struct Type{
	enum {BASIC,ARRAY,STRUCTURE,FUNCTION} kind;
	union {
		int basic; //1-INT,2-FLOAT
		struct {
			struct Type* elem;
			int size;
		} array; 
		struct {
			struct structField* field;
			char name[30];
		}structure;
		struct Func* func;
	} u;
}Type;

typedef struct funcPara{
	Type* type;
	char* name;
	struct funcPara* next_para;
}funcPara;

typedef struct Func{
	Type* return_type;
	funcPara* para;
	int para_num;
}Func;

typedef struct structField{
	char* name;
	Type* type;
	struct structField* next;
}structField;

typedef struct semanNode{
	Type* type;
	char name[30];
	int line;
	int level;
	struct semanNode* next_in_hash;
	//struct semanNode* next_in_scope;
}semanNode;

typedef struct structNode{
	char name[30];
	Type* type;
	int line;
}structNode;

semanNode* create_semanNode(Type* t,Node* p,int level){
	semanNode* sNode = (semanNode*)malloc(sizeof(semanNode));
	strcpy(sNode->name,p->type_char);
	sNode->type = t;
	sNode->line = p->line;
	sNode->level = level;
	sNode->next_in_hash = NULL;
	return sNode;
}

structNode* create_structNode(Type* t,Node* p){
	structNode* sNode = (structNode*)malloc(sizeof(structNode));
	//printf("create_structNode:%s\n",p->type_char);
	strcpy(sNode->name,p->type_char);
	sNode->line = p->line;
	sNode->type = t;
	return sNode;
}

semanNode* hashtable[101];
structNode* struct_hashtable[101];

int hash(char* name){
	unsigned int val = 0, i;
 	for (; *name; ++name)
 	{
 		/*val = (val << 2) + *name;
 		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;*/
 		val = val * 31 + *name;
 	}	
 	val = val % 101;
 	return val;
}

void init_hash(){
	for(int i = 0;i < 101;i++){
		hashtable[i] = NULL;
	}
}

void init_structhash(){
	for(int i = 0;i < 101;i++){
		struct_hashtable[i] = NULL;
	}
} 

void insert_hash(semanNode* sNode){
	int i = hash(sNode->name);
	if(hashtable[i] == NULL){
		hashtable[i] = sNode;
	}
	else{
		//deal collision
		sNode->next_in_hash = hashtable[i];
		hashtable[i] = sNode;
	}
}

void insert_struct_hashtable(structNode* Node){
	int i = hash(Node->name);
	//printf("check:%s\n",Node->name);
	if(struct_hashtable[i] == NULL){
		struct_hashtable[i] = Node;
	}
	else{
		printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",Node->line,Node->name );
	}
}

/*1-true 0-false*/
int check_in_structhash(Node* p){
	int i = hash(p->type_char);
	structNode* s = struct_hashtable[i];
	if(s == NULL){
		return 0;
	}
	if(strcmp(p->type_char,s->name) == 0){
		return 1;
	}
	else{
		return 0;
	}
}

/*1-true 0-false*/
int check_in_struct(Type* sType,Node* elem){
	int i = hash(sType->u.structure.name);
	if(strcmp(sType->u.structure.name,"") == 0){
		return 1;
	}
	//printf("check_in_struct:%s\n",sType->u.structure.name);
	structNode* s = struct_hashtable[i];
	Type* t = s->type;
	structField* sField = t->u.structure.field;
	while(sField != NULL){
		//printf("%s\n",sField->name);
		if(strcmp(sField->name,elem->type_char) == 0)
			return 1;
		sField = sField->next;
	}
	return 0;
}

int redefine_in_struct(Node* p,Type* t){
	structField* sField = t->u.structure.field;
	while(sField != NULL){
		if(strcmp(sField->name,p->type_char) == 0)
			return 1;
		sField = sField->next;
	}
	return 0;
}

Type* get_struct_Type(Node* p){
	int i = hash(p->type_char);
	structNode* s = struct_hashtable[i];
	//printf("check:%s\n",s->name );
	return s->type;
}

Type* get_elem_Type(Node* p,Type* head){
	structField* sField = head->u.structure.field;
	while(sField != NULL){
		//printf("%s\n",sField->name);
		if(strcmp(sField->name,p->type_char) == 0)
			return sField->type;
		sField = sField->next;
	}
	return NULL;
}

void delete_variable_level(int level){
	for(int i = 0;i < 101;i++){
		if(hashtable[i] == NULL)
			continue;
		else{
			semanNode* s = hashtable[i];
			while(s->level == level){	
				s = s->next_in_hash;
				if(s == NULL)break;
			}
			hashtable[i] = s;
			//printf("var:%d\n", i);
		}
	}
}

void travel_tree(Node* root);
void deal_ExtDef(Node* p);
Type* deal_Specifier(Node* p);
void deal_VarDec(Node* p,Type* t,Type* scope);
void deal_ExtDecList(Node* p,Type* t);
Type* deal_FunDec(Node* p,Type* t);
void deal_VarList(Type* funcType,Node* p);
Type* deal_StructSpecifier(Node* p);
Type* deal_OptTag(Node* p);
Type* deal_Tag(Node* p);
void deal_ParamDec(Type* funcType,Node* p);
void deal_CompSt(Node* p,Type* scope);
void deal_Def(Node* p,Type* scope);
void deal_DecList(Type* t,Node* p,Type* scope);
void deal_Dec(Type* t,Node* p,Type* scope);
void deal_DefList(Node* p,Type* scope);
Type* deal_Exp(Node* p,Type* scope);
void deal_Args(Node* p,Type* fType,Type* scope);
void deal_StmtList(Node* p,Type* scope);
void deal_Stmt(Node* p,Type* scope);
int redefine(Node* p,int level);
int ID_has_define(Node* p);
int check_variable(Node* p);
void print_Para(Type* t);
void print_Args(Type* t);
void print_p(Type* t);
Type* add_array(Type* arrayType,Type* t);
Type* get_Type(Node* p);

#endif