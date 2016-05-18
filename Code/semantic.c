#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "semantic.h"
#include "syntax.tab.h"


extern Node* get_root();

int level = 0;

void semantic(){
	init_hash();
	init_structhash();
	Node* root = get_root();

	if(root == NULL){
		//printf("the program is empty\n");
		exit(-1);
	}

	travel_tree(root);
}

void travel_tree(Node* root){
	if(root->node_type == -1 && root->children == NULL)
		return;
	//printf("%s\n",root->name);

	if(strcmp(root->name,"ExtDef") == 0){
		deal_ExtDef(root);
	}

	//if(root->children == NULL)
	//	return;
	if(root->children != NULL){
		travel_tree(root->children);
		Node* temp = root->children;
		while(temp->sibling != NULL){
			travel_tree(temp->sibling);
			temp = temp->sibling;
		}
	}
}

void deal_ExtDef(Node* p){
	//Type t = (Type*)malloc(sizeof(struct Type));
	Type* t = deal_Specifier(p->children);

	p = p->children;
	if(strcmp(p->sibling->name,"ExtDecList") == 0){
		deal_ExtDecList(p->sibling,t);
	}
	Type* funcType = (Type*)malloc(sizeof(struct Type));
	if(strcmp(p->sibling->name,"FunDec") == 0){
		funcType = deal_FunDec(p->sibling,t);
	}

	p = p->sibling;
	if(p->sibling == NULL){
		return;
	}
	if(strcmp(p->sibling->name,"CompSt") == 0){
		deal_CompSt(p->sibling,funcType);
	}
}

Type* deal_Specifier(Node* p){
	Type* t = NULL;
	t = (Type*)malloc(sizeof(struct Type));
	p = p->children;
	//Type* t1 = t;
	if(strcmp(p->name,"TYPE") == 0){
		t->kind = BASIC;
		if(strcmp(p->type_char,"int") == 0){
			t->u.basic = 1;
		}
		else{
			t->u.basic = 2;	
		}
	}
	else{
		t = deal_StructSpecifier(p);

	}
	return t;	
}

Type* deal_StructSpecifier(Node* p){
	p = p->children;
	Type* t = NULL;
	if(p->sibling->sibling == NULL){
		t = deal_Tag(p->sibling);
	}
	else{
		t = deal_OptTag(p->sibling);
		p = p->sibling->sibling->sibling;
		deal_DefList(p,t);
	}
	return t;
}

Type* deal_OptTag(Node* p){
	Type* t = NULL;
	t = (Type*)malloc(sizeof(Type));
	t->kind = STRUCTURE;
	t->u.structure.field = NULL;
	structNode* sNode;
	if(p->children != NULL){
		strcpy(t->u.structure.name,p->children->type_char);
		sNode = create_structNode(t,p->children);
		insert_struct_hashtable(sNode);
	}
	else{
		strcpy(t->u.structure.name,"");
	}
	
	//printf("OptTag:%s\n",t->u.structure.name);
	//structNode* sNode = create_structNode(t,p->children);
	//insert_struct_hashtable(sNode);
	return t;
}

Type* deal_Tag(Node* p){
	/*check ID*/
	p = p->children;
	//printf("111\n");
	if(check_in_structhash(p) == 0){
		printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",p->line,p->type_char);
		return NULL;
	}
	Type* t = get_struct_Type(p);
	return t;
}

void deal_ExtDecList(Node* p,Type* t){
	p = p->children;
	if(p->sibling == NULL){
		deal_VarDec(p,t,NULL);
	}
	else{
		deal_VarDec(p,t,NULL);
		deal_ExtDecList(p,t);	
	}
}

void deal_VarDec(Node* p,Type* t,Type* scope){
	p = p->children;
	if(scope == NULL){
		if(p->sibling == NULL){
			if(redefine(p,level) == 1){
				printf("Error type 3 at Line %d:Redefined variable \"%s\"\n",p->line,p->type_char);
				return;
			}
			else{
				semanNode* s = create_semanNode(t,p,level);
				insert_hash(s);
			}
		}
	}
	else if(scope->kind == FUNCTION){
		if(p->sibling == NULL){
			if(redefine(p,level) == 1){
				printf("Error type 3 at Line %d:Redefined variable \"%s\"\n",p->line,p->type_char);
				return;
			}
			else{
				semanNode* s = create_semanNode(t,p,level);
				insert_hash(s);
			}
		}
		else{
			Type* arrayType = NULL;
			while(p->sibling != NULL){
				Type* stype = (Type*)malloc(sizeof(struct Type));
				stype->kind = ARRAY;
				stype->u.array.size = p->sibling->sibling->type_int;
				stype->u.array.elem = NULL;

				arrayType = add_array(arrayType,stype);

				p = p->children;
			}
			//printf("create_semanNode%d\n",arrayType->kind);
			arrayType = add_array(arrayType,t);
			if(redefine(p,level) == 1){
				printf("Error type 3 at Line %d:Redefined variable \"%s\"\n",p->line,p->type_char);
				return;
			}
			else{
				semanNode* s = create_semanNode(arrayType,p,level);
				insert_hash(s);
			}
		}
	}
	else{
		structField* sField = (structField*)malloc(sizeof(structField));
		if(redefine_in_struct(p,scope) == 1){
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n",p->line,p->type_char);
			return;
		}
		sField->name = p->type_char;
		sField->type = t;
		sField->next = NULL;
		if(scope->u.structure.field == NULL){
			scope->u.structure.field = sField;
		}
		else{
			//printf("VarDec:%s\n",p->type_char);
			structField* temp = scope->u.structure.field;
			while(temp->next != NULL){
				temp = temp->next;
			}
			temp->next = sField;
		}
	}
}

Type* add_array(Type* arrayType,Type* t){
	if(arrayType == NULL){
		arrayType = t;
		//printf("create_semanNode%d\n",arrayType->kind);
	}
	else{
		Type* h = arrayType;
		while(h->u.array.elem != NULL)
			h = h->u.array.elem;
		h->u.array.elem = t;
	}
	return arrayType;
}

Type* deal_FunDec(Node* p,Type* t){
	p = p->children;
	Type* funcType = NULL;
	funcType = (Type*)malloc(sizeof(struct Type));
	funcType->kind = FUNCTION;
	funcType->u.func = (Func*)malloc(sizeof(struct Func));
	funcType->u.func->return_type = t;

	if(redefine(p,level) == 1){
		printf("Error type 4 at Line %d:Redefined variable \"%s\"\n",p->line,p->type_char);
		//exit(-1);
		return NULL;
	}
	semanNode* sNode = (semanNode*)malloc(sizeof(semanNode));
	strcpy(sNode->name,p->type_char);
	sNode->line = p->line;
	sNode->level = level;
	sNode->next_in_hash = NULL;


	level++;
	p = p->sibling->sibling;
	if(p->sibling == NULL){
		funcType->u.func->para = NULL;
		funcType->u.func->para_num = 0;
		sNode->type = funcType;
		insert_hash(sNode);
	}
	else{
		deal_VarList(funcType,p);
		sNode->type = funcType;
		insert_hash(sNode);
	}
	level--;
	return funcType;
}

void deal_VarList(Type* funcType,Node* p){
	p = p->children;
	deal_ParamDec(funcType,p);
	if(p->sibling == NULL)
		return;
	deal_VarList(funcType,p->sibling->sibling);
}

void deal_ParamDec(Type* funcType,Node* p){
	p = p->children;
	Type* para = deal_Specifier(p);

	funcPara* fPara = (funcPara*)malloc(sizeof(struct funcPara));
	if(funcType->u.func->para == NULL){
		funcType->u.func->para = (funcPara*)malloc(sizeof(struct funcPara));
		funcType->u.func->para->type = para;
		funcType->u.func->para_num = 1;
		funcType->u.func->para->next_para = NULL;
	}
	else{
		funcPara* fPara = funcType->u.func->para;
		funcPara* tempPara = (funcPara*)malloc(sizeof(struct funcPara));
		tempPara->type = para;
		tempPara->next_para = NULL;
		while(fPara->next_para != NULL){
			fPara = fPara->next_para;
		}
		fPara->next_para = tempPara;
		funcType->u.func->para_num++;
	}
	//fPara = funcType->u.func->para;
	/*while(fPara != NULL){
		fPara = fPara->next_para;
	}
	fPara->type = para;
	fPara->next_para = NULL;
	funcType->u.func->para_num++;
*/
	deal_VarDec(p->sibling,para,funcType);
}

void deal_CompSt(Node* p,Type* scope){
	level++;

	p = p->children;
	p = p->sibling;
	deal_DefList(p,scope);

	p = p->sibling;

	deal_StmtList(p,scope);

	delete_variable_level(level);
	level--;
}

void deal_DefList(Node* p,Type* scope){
	if(p->children == NULL)
		return;
	p = p->children;
	deal_Def(p,scope);
	deal_DefList(p->sibling,scope);
}

void deal_Def(Node* p,Type* scope){
	Type* t = deal_Specifier(p->children);

	p = p->children;
	deal_DecList(t,p->sibling,scope);
}

void deal_DecList(Type* t,Node* p,Type* scope){
	//printf("deal_DecList");
	p = p->children;
	if(p->sibling == NULL){
		deal_Dec(t,p,scope);
	}
	else{
		deal_Dec(t,p,scope);
		deal_DecList(t,p->sibling->sibling,scope);	
	}
}

void deal_Dec(Type* t,Node* p,Type* scope){
	p = p->children;
	deal_VarDec(p,t,scope);
	if(p->sibling == NULL){
		return;
	}
	else{
		if(scope->kind == STRUCTURE){
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n",p->line,p->children->type_char);
			return;
		}
		deal_Exp(p->sibling->sibling,scope);
	}
}

Type* deal_Exp(Node* p,Type* scope){
	p = p->children;
	if(p->sibling == NULL){
		if(strcmp(p->name,"ID") == 0){
			if(ID_has_define(p) == 0){
				printf("Error type 1 at Line %d: Undefined variable \"%s\"\n",p->line,p->type_char);
				return NULL;
			}
			else{
				Type* t = get_Type(p);
				return t;
			}
		}
		if(strcmp(p->name,"INT") == 0){
			Type* t = (Type*)malloc(sizeof(struct Type));
			t->kind = BASIC;
			t->u.basic = 1;
			return t;
		}
		if(strcmp(p->name,"FLOAT") == 0){
			Type* t = (Type*)malloc(sizeof(struct Type));
			t->kind = BASIC;
			t->u.basic = 2;
			return t;
		}
	}
	else if(p->sibling->sibling == NULL){
		deal_Exp(p->sibling,scope);
	}
	else if(p->sibling->sibling->sibling == NULL){
		if(strcmp(p->name,"ID") == 0){
			if(ID_has_define(p) == 0){
				printf("Error type 2 at Line %d: Undefined function \"%s\"\n",p->line,p->type_char);
				return NULL;
			}
			Type* t = get_Type(p);
			if(t->kind != FUNCTION){
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n",p->line,p->type_char);
				return NULL;
			}
			Type* funcType = NULL;
			funcType = (Type*)malloc(sizeof(struct Type));
			funcType->kind = FUNCTION;
			funcType->u.func = (Func*)malloc(sizeof(struct Func));
			funcType->u.func->return_type = t;

			return t->u.func->return_type;
		}
		else if(strcmp(p->sibling->name,"Exp") == 0){
			Type* t = deal_Exp(p->sibling,scope);
			return t;
		}
		else if(strcmp(p->sibling->name,"DOT") == 0){
			Type* sType = deal_Exp(p,scope);
			if(sType == NULL)
				return NULL;
			if(sType->kind != STRUCTURE){
				printf("Error type 13 at Line %d: Illegal use of \".\".\n",p->line);
				return NULL;
			}
			if(check_in_struct(sType,p->sibling->sibling) == 0){
				printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",p->line,p->sibling->sibling->type_char);
				//exit(-1);
				return NULL;
			}
			Type* t = get_elem_Type(p->sibling->sibling,sType);
			return t;
		}
		else{
			if(strcmp(p->sibling->name,"ASSIGNOP") == 0){
				if(check_variable(p) == 0){
					printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",p->line);
					return NULL;
				}
			}
			Type* lType = deal_Exp(p,scope);
			Type* rType = deal_Exp(p->sibling->sibling,scope);
			if(lType == NULL || rType == NULL){
				if(strcmp(p->sibling->name,"ASSIGNOP") == 0)
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n", p->line);
				else
					printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->line);
				return NULL;
			}
			if(lType->kind != rType->kind){
				if(strcmp(p->sibling->name,"ASSIGNOP") == 0)
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n", p->line);
				else
					printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->line);
				return NULL;
			}
			else{
				if(lType->kind == BASIC && lType->u.basic != rType->u.basic){
					if(strcmp(p->sibling->name,"ASSIGNOP") == 0)
						printf("Error type 5 at Line %d: Type mismatched for assignment.\n", p->line);	
					else{
						printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->line);
					}
					return NULL;
				}
				if(lType->kind == STRUCTURE){
					//printf("check:%s\n",lType->u.structure.name);
					if(strcmp(lType->u.structure.name,rType->u.structure.name) != 0){
						if(strcmp(p->sibling->name,"ASSIGNOP") == 0)
							printf("Error type 5 at Line %d: Type mismatched for assignment.\n", p->line);	
						else{
							printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->line);
						}
						return NULL;
					}
				}
			}
			return lType;
		}
	}
	else{
		if(strcmp(p->name,"Exp") ==  0){
			Type* arrayType = deal_Exp(p,scope);
			//printf("Exp%d\n",arrayType->kind);
			if(arrayType->kind != ARRAY){
				printf("Error type 10 at Line %d: \"%s\" is not an array.\n",p->line,p->children->type_char);
			}
			Type* numType = deal_Exp(p->sibling->sibling,scope);
			if(numType->kind != BASIC){
				printf("Error type 12 at Line %d: \"%s\" is not an integer.\n",p->line,p->sibling->sibling->children->type_char);
				return NULL;
			}
			if(numType->u.basic != 1){
				printf("Error type 12 at Line %d: \"%f\" is not an integer.\n",p->line,p->sibling->sibling->children->type_float);
				return NULL;
			}
			
			return arrayType->u.array.elem;
		}
		else/*Args*/{
			if(ID_has_define(p) == 0){
				printf("Error type 2 at Line %d: Undefined function \"%s\"\n",p->line,p->type_char);
				return NULL;
			}
			Type* t = get_Type(p);
			if(t->kind != FUNCTION){
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n",p->line,p->type_char);
				return NULL;
			}
			Type* funcType = NULL;
			funcType = (Type*)malloc(sizeof(struct Type));
			funcType->kind = FUNCTION;
			funcType->u.func = (Func*)malloc(sizeof(struct Func));
			funcType->u.func->return_type = t->u.func->return_type;
			//fType->u.func = NULL;
			deal_Args(p->sibling->sibling,funcType,scope);

			/*DEAL ARGS WRONG*/
			if(funcType->u.func->para_num != t->u.func->para_num){
				printf("Error type 9 at Line %d: Function \"",p->line);
				printf("%s",p->type_char);
				print_Para(t);
				printf("\" is not applicable for arguments ");
				print_Args(funcType);
				return NULL;
			}
			else{
				funcPara* f1 = (funcPara*)malloc(sizeof(struct funcPara));
				funcPara* f2 = (funcPara*)malloc(sizeof(struct funcPara));
				f1 = funcType->u.func->para;
				f2 = t->u.func->para;
				while(f1 != NULL){
					if(f1->type->kind != f2->type->kind){
						printf("Error type 9 at Line %d: Type mismatched for Function\n",p->line);
					}
					if(f1->type->u.basic != f2->type->u.basic){
						printf("Error type 9 at Line %d: Type mismatched for Function\n",p->line);
					}
					f1 = f1->next_para;
					f2 = f2->next_para;
				}
			}

			return t->u.func->return_type;
		}
	}
}

void deal_Args(Node* p,Type* fType,Type* scope){
	p = p->children;
	Type* para = deal_Exp(p,scope);
	funcPara* fPara = (funcPara*)malloc(sizeof(struct funcPara));
	if(fType->u.func->para == NULL){
		fType->u.func->para = (funcPara*)malloc(sizeof(struct funcPara));
		fType->u.func->para->type = para;
		fType->u.func->para_num = 1;
		fType->u.func->para->next_para = NULL;
	}
	else{
		funcPara* fPara = fType->u.func->para;
		funcPara* tempPara = (funcPara*)malloc(sizeof(struct funcPara));
		tempPara->type = para;
		tempPara->next_para = NULL;
		while(fPara->next_para != NULL){
			fPara = fPara->next_para;
		}
		fPara->next_para = tempPara;
		fType->u.func->para_num++;
	}
	if(p->sibling == NULL)
		return;
	else{
		deal_Args(p->sibling->sibling,fType,scope);	
	}
}

void deal_StmtList(Node* p,Type* scope){
	p = p->children;
	if(p == NULL){
		return;
	}
	else{
		deal_Stmt(p,scope);
		deal_StmtList(p->sibling,scope);
	}
}

void deal_Stmt(Node* p,Type* scope){
	p = p->children;
	if(strcmp(p->name,"Exp") == 0){
		deal_Exp(p,scope);
	}
	else if(strcmp(p->name,"CompSt") == 0){
		deal_CompSt(p,scope);
	}
	else if(strcmp(p->name,"RETURN") == 0){
		Type* rType = deal_Exp(p->sibling,scope);
		if(rType == NULL){
			return;
		}
		if(rType->kind != scope->u.func->return_type->kind){
			printf("Error type 8 at Line %d: Type mismatched for return.\n", p->line);
			return;
		}
		else{
			if(rType->u.basic != scope->u.func->return_type->u.basic){
				printf("Error type 8 at Line %d: Type mismatched for return.\n", p->line);		
				return;
			}
		}
	}
	else if(strcmp(p->name,"IF") == 0){
		if(p->sibling->sibling->sibling->sibling->sibling == NULL){
			deal_Exp(p->sibling->sibling,scope);
			deal_Stmt(p->sibling->sibling->sibling->sibling,scope);
		}
		else{
			deal_Exp(p->sibling->sibling,scope);
			deal_Stmt(p->sibling->sibling->sibling->sibling,scope);
			deal_Stmt(p->sibling->sibling->sibling->sibling->sibling->sibling,scope);
		}
	}
	else if(strcmp(p->name,"WHILE") == 0){
		deal_Exp(p->sibling->sibling,scope);
		deal_Stmt(p->sibling->sibling->sibling->sibling,scope);
	}
}

/*0-false 1-true*/
int redefine(Node* p,int level){
	int i = hash(p->type_char);
	//printf("check:%s\n",p->type_char);
	semanNode* s = hashtable[i];
	while(s != NULL){
		if(strcmp(p->type_char,s->name) == 0 && s->level == level){
			return 1;
		}
		else{
			s = s->next_in_hash;
		}
	}
	structNode* sNode = struct_hashtable[i];
	if(sNode != NULL){
		if(strcmp(p->type_char,sNode->name) == 0){
			return 1;
		}
	}
	return 0;
}

/*0-false 1-true*/
int ID_has_define(Node* p){
	int i = hash(p->type_char);
	semanNode* s = hashtable[i];
	while(s != NULL){
		if(strcmp(p->type_char,s->name) == 0){
			return 1;
		}
		else{
			s = s->next_in_hash;
		}
	}
	return 0;
}

/*0-false 1-true*/
int check_variable(Node* p){
	p = p->children;
	if(p->sibling == NULL){
		if(strcmp(p->name,"ID") == 0)
			return 1;
		else
			return 0;
	}
	else if(p->sibling->sibling->sibling == NULL){
		if(strcmp(p->sibling->name,"DOT") == 0)
			return 1;
		else
			return 0;
	}
	else if(p->sibling->sibling->sibling->sibling == NULL){
		if(strcmp(p->sibling->name,"LB") == 0)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

/*get the variable type*/
Type* get_Type(Node* p){
	int i = hash(p->type_char);
	semanNode* s = hashtable[i];
	while(s != NULL){
		if(strcmp(p->type_char,s->name) == 0){
			return s->type;
		}
		else{
			s = s->next_in_hash;
		}
	}
	return NULL;
}

/*print the function para*/
void print_Para(Type* t){
	printf("(");
	Func* f = t->u.func;
	if(f->para == NULL){
		printf(")");
	}
	else{
		funcPara* para = f->para;
		print_p(para->type);
		//printf("%d",para->type->kind);
		para = para->next_para;
		while(para != NULL){
			printf(",");
			print_p(para->type);
			//printf(",%d",para->type->kind);
			para = para->next_para;
		}
	}
	printf(")" );
}

/*print the function's Args when using the func*/
void print_Args(Type* t){
	printf("\"(");
	Func* f = t->u.func;
	if(f->para == NULL){
		printf(")");
	}
	else{
		funcPara* para = f->para;
		print_p(para->type);
		para = para->next_para;
		while(para != NULL){
			//printf(",%d",para->type->kind);
			printf(",");
			print_p(para->type);
			para = para->next_para;
		}
	}
	printf(")\".\n" );
}

/*print para type*/
void print_p(Type* t){
	if(t->kind == BASIC){
		if(t->u.basic == 1)
			printf("int");
		if(t->u.basic == 2)
			printf("float");
	}
}
