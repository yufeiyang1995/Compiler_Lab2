#include <stdio.h>
extern FILE* yyin;
extern int yydebug;
extern void yyrestart(FILE* f);
extern void yyparse();
extern void semantic();
void print();
 
int main(int argc, char** argv) {
	if (argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if (!f){
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	yyparse();
	//print();
	semantic();
	return 0;
}
