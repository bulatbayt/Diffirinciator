#ifndef DIFF_H
#define DIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/stat.h>

enum NodeType
{
    NUM = 1,
    VAR = 2,
    OP  = 3
};

enum Operation_t
{
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4
};

struct Node
{
    int type;
    double val;
    struct Node* left;
    struct Node* right;
};

#define _NUM(val)         New_Node (NUM, (double)(val), NULL, NULL)
#define _VAR(name)        New_Node (VAR, (double)(name), NULL, NULL)
#define _ADD(left, right) New_Node (OP, ADD, (left), (right))
#define _SUB(left, right) New_Node (OP, SUB, (left), (right))
#define _MUL(left, right) New_Node (OP, MUL, (left), (right))
#define _DIV(left, right) New_Node (OP, DIV, (left), (right))

#define _CL Copy_Tree (node->left)
#define _CR Copy_Tree (node->right)
#define _DL Diff (node->left, var_name)
#define _DR Diff (node->right, var_name)

struct Node*          New_Node                (int type, double val, struct Node* left, struct Node* right);
struct Node*          Copy_Tree               (const struct Node* node);
void                  Tree_Destroy            (struct Node* node);

int                   ReadFromFile            (const char* name_file, char** buffer);
struct Node*          Load_Tree_From_Buffer   (char** ptr);
struct Node*          Parse_Elem_To_Node      (const char* elem_str);

double                Count_Tree               (const struct Node* node, double x_val, double y_val);
struct Node*          Diff                    (const struct Node* node, char var_name);

void                  Graph_Dump              (const struct Node* root, const char* dot_file, const char* png_file);
void                  Run_Draw                (const struct Node* node, FILE* out);
void                  Tex_Dump                (const struct Node* root, FILE* tex_file, const char* title);
void                  Print_Tex               (const struct Node* node, FILE* out);

#endif 