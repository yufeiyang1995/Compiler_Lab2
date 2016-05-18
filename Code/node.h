#ifndef NODE_H
#define NODE_H

typedef struct Node{
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
}Node;

#endif