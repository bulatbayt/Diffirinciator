#ifndef DIFF_H
#define DIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>




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
    DIV = 4,
    POW = 5,
    SIN = 6,
    COS = 7,
    LN  = 8
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
#define _POW(left, right) New_Node (OP, POW, (left), (right))
#define _SIN(arg)         New_Node (OP, SIN, NULL,   (arg))
#define _COS(arg)         New_Node (OP, COS, NULL,   (arg))
#define _LN(arg)          New_Node (OP, LN,  NULL,   (arg))

#define _CL Copy_Tree (node->left)
#define _CR Copy_Tree (node->right)
#define _DL Diff      (node->left, var_name)
#define _DR Diff      (node->right, var_name)


void                  Input                   (char** buffer, struct Node** manual_formula, struct Node** formula);


struct Node           *New_Node               (int type, double val, struct Node *left, struct Node *right);
struct Node*          Copy_Tree               (const struct Node* node);
void                  Tree_Destroy            (struct Node* node);



int                   ReadFromFile            (const char* name_file, char** buffer);
struct Node           *Load_Tree_From_Buffer  (char **ptr);
struct Node*          Parse_Elem_To_Node      (const char* elem_str);


void                  SkipSpace               (char **ptr);
int                   Is_Zero                 (const struct Node* node);
int                   Is_One                  (const struct Node* node);


struct Node*          Simplify_Tree           (struct Node *node, FILE* Dump_count);
struct Node*          Simplify_Num            (struct Node* node, int op, FILE* Dump_count);
struct Node*          Simplify_Op             (struct Node* node, int op);
struct Node*          Simplify_Add            (struct Node* node);
struct Node*          Simplify_Sub            (struct Node* node);
struct Node*          Simplify_Mul            (struct Node* node);
struct Node*          Simplify_Div            (struct Node* node);





double                Count_Tree              (const struct Node* node, double x_val, double y_val, FILE* Dump_count);
struct Node*          Diff                    (const struct Node* node, char var_name);



void                  Output                  (struct Node *formula, struct Node *d_formula, double res);
void                  Graph_Dump              (const struct Node* root, const char* dot_file, const char* png_file);
void                  Run_Draw                (const struct Node* node, FILE* out);
void                  Html_Dump               (const char* html_file, double eval_res,
                                               const char* img1, const char* img2, const char* pdf_file);

                                               
void                  Tex_Dump                (const struct Node* root, FILE* tex_file, const char* title);
void                  Print_Tex               (const struct Node* node, FILE* out);
void                  Dump_Count_Tree         (FILE* Dump_count, double value_l, double value_r, char* op, double result);

#endif 
