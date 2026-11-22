#include "diff.h"

int main ()
{
    struct Node* manual_formula = _DIV (_ADD (_NUM (2), _VAR ('x')), _SUB (_NUM (5), _VAR ('y')));

    char* buffer      = NULL;
    struct Node* formula = NULL;

    if (ReadFromFile ("formula.txt", &buffer) > 0 && buffer != NULL)
    {
        char* ptr = buffer;
        formula = Load_Tree_From_Buffer (&ptr);
        free (buffer);
    }
    else
    {
        formula = manual_formula;
        manual_formula = NULL;
    }

    double res = Count_Tree (formula, 3.0, 1.0);
    printf ("Result (x=3, y=1): %g\n", res);

    struct Node* d_formula = Diff (formula, 'x');

    Graph_Dump (formula, "formula.dot", "formula.png");

    printf ("Enter to continue..........\n");
    getchar ();

    Graph_Dump (d_formula, "diff.dot", "diff.png");

    FILE* tex = fopen ("dump.tex", "w");
    if (tex != NULL)
    {
        fprintf (tex, "\\documentclass{article}\n"
                    "\\usepackage{fontspec}\n"
                    "\\setmainfont{Times New Roman}\n"
                    "\\begin{document}\n");

        Tex_Dump (formula,   tex, "Исходное выражение:");
        Tex_Dump (d_formula, tex, "Производная по x:");

        fprintf (tex, "\\end{document}\n");
        fclose (tex); // <--- ОБЯЗАТЕЛЬНО закрыть файл ДО запуска tectonic!

        system ("tectonic dump.tex");
        system ("start dump.pdf");
    }


    Tree_Destroy (formula);
    Tree_Destroy (d_formula);
    Tree_Destroy (manual_formula);

    return 0;
}


struct Node*          New_Node                (int type, double val, struct Node* left, struct Node* right)
{
    struct Node* node = (struct Node*) calloc (1, sizeof (struct Node));
    assert (node != NULL);

    node->type  = type;
    node->val   = val;
    node->left  = left;
    node->right = right;

    return node;
}

struct Node*          Copy_Tree               (const struct Node* node)
{
    if (node == NULL)
        return NULL;

    return New_Node (node->type, node->val, Copy_Tree (node->left), Copy_Tree (node->right));
}

void                  Tree_Destroy            (struct Node* node)
{
    if (node == NULL)
        return;

    Tree_Destroy (node->left);
    Tree_Destroy (node->right);
    free (node);
}


int                   ReadFromFile            (const char* name_file, char** buffer)
{
    FILE* file = fopen (name_file, "r");
    if (file == NULL)
        return -1;

    int fd = fileno (file);
    struct stat st = { 0 };
    fstat (fd, &st);
    size_t size = st.st_size;

    *buffer = (char*) calloc (size + 1, sizeof (char));
    fread (*buffer, sizeof (char), size, file);
    fclose (file);

    return (int)size;
}

struct Node*          Parse_Elem_To_Node      (const char* elem_str)
{
    assert (elem_str != NULL);

    if (strcmp (elem_str, "+") == 0) return New_Node (OP, ADD, NULL, NULL);
    if (strcmp (elem_str, "-") == 0) return New_Node (OP, SUB, NULL, NULL);
    if (strcmp (elem_str, "*") == 0) return New_Node (OP, MUL, NULL, NULL);
    if (strcmp (elem_str, "/") == 0) return New_Node (OP, DIV, NULL, NULL);

    char* endptr = NULL;
    double num = strtod (elem_str, &endptr);
    if (endptr != elem_str && *endptr == '\0')
        return New_Node (NUM, num, NULL, NULL);

    if (isalpha ((unsigned char)elem_str[0]) && elem_str[1] == '\0')
        return New_Node (VAR, (double)elem_str[0], NULL, NULL);


    printf ("Syntax error: unknown element '%s'\n", elem_str);
    return NULL;
}

struct Node*          Load_Tree_From_Buffer   (char** ptr)
{
    while (isspace ((unsigned char)**ptr))
        (*ptr)++;

    if (**ptr == '\0')
        return NULL;


    if (**ptr == '(')
    {
        (*ptr)++;
        while (isspace ((unsigned char)**ptr))
            (*ptr)++;


        struct Node* left = Load_Tree_From_Buffer (ptr);

        while (isspace ((unsigned char)**ptr))
            (*ptr)++;


        if (**ptr == ')')
        {
            (*ptr)++;
            return left;
        }

    
        char op_str[2] = { **ptr, '\0' };
        (*ptr)++;

        struct Node* op_node = Parse_Elem_To_Node (op_str);
        if (op_node == NULL)
        {
            Tree_Destroy (left);
            return NULL;
        }


        op_node->left  = left;
        op_node->right = Load_Tree_From_Buffer (ptr);

        while (isspace ((unsigned char)**ptr))
            (*ptr)++;

        if (**ptr == ')')
            (*ptr)++;

        return op_node;
    }


    char elem_str[128] = { 0 };
    int i = 0;
    while (!isspace ((unsigned char)**ptr) && **ptr != '(' && **ptr != ')' && **ptr != '\0' && i < 127)
    {
        elem_str[i++] = **ptr;
        (*ptr)++;
    }
    elem_str[i] = '\0';

    return Parse_Elem_To_Node (elem_str);
}



double                Count_Tree               (const struct Node* node, double x_val, double y_val)
{
    assert (node != NULL);

    if (node->type == NUM)
        return node->val;

    if (node->type == VAR)
    {
        if ((char)node->val == 'x') return x_val;
        if ((char)node->val == 'y') return y_val;
        return 0.0;
    }

    double l = Count_Tree (node->left, x_val, y_val);
    double r = Count_Tree (node->right, x_val, y_val);

    switch ((int)node->val)
    {
        case ADD: return l + r;
        case SUB: return l - r;
        case MUL: return l * r;
        case DIV:
        {
            if (r != 0)
                return (l / r);
            else
                return 0;
        }
        default:  return 0;
    }
}

struct Node*          Diff                    (const struct Node* node, char var_name)
{
    assert (node != NULL);

    switch (node->type)
    {
        case NUM:
            return _NUM (0);

        case VAR:
            if ( (char) node->val == var_name)
            {
                return _NUM (1);
            }

            else 
            {
                return _NUM (0);
            }

        case OP:
            switch ((int)node->val)
            {
                case ADD:
                    return _ADD (_DL, _DR);

                case SUB:
                    return _SUB (_DL, _DR);

                case MUL:
                    return _ADD (_MUL (_DL, _CR), _MUL (_CL, _DR));

                case DIV:
                    return _DIV (_SUB (_MUL (_DL, _CR), _MUL (_CL, _DR)), _MUL (_CR, _CR));

                default:
                    return NULL;
            }

        default:
            return NULL;
    }
}


void                  Graph_Dump              (const struct Node* root, const char* dot_file, const char* png_file)
{
    if (root == NULL)
        return;

    FILE* out = fopen (dot_file, "w");
    if (out == NULL)
        return;

    fprintf (out, "digraph G {\n");
    fprintf (out, "  rankdir = TB;\n");
    fprintf (out, "  node [shape = Mrecord, style = filled, fontname = \"Arial\"];\n\n");

    Run_Draw (root, out);

    fprintf (out, "}\n");
    fclose (out);

    char cmd[256] = { 0 };
    snprintf (cmd, sizeof (cmd), "dot -Tpng %s -o %s", dot_file, png_file);
    system   (cmd);

    snprintf (cmd, sizeof (cmd), "start %s", png_file);
    system   (cmd);
}

void                  Run_Draw                (const struct Node* node, FILE* out)
{
    if (node == NULL)
        return;

    if (node->type == NUM)
    {
        fprintf (out, "  N%p [label = \"{ NUM | %g }\", fillcolor = \"#C0E0FF\"];\n",
                (void*)node, node->val);
    }
    else if (node->type == VAR)
    {
        const char* color = "#FFE4B5";

        if ((char)node->val == 'x')
        {
            color = "#98FB98";
        }

        fprintf (out, "  N%p [label = \"{ VAR | %c }\", fillcolor = \"%s\"];\n",
                (void*)node, (char)node->val, color);
    }
    else if (node->type == OP)
    {
        const char* op_name = "?";
        const char* color   = "#FFB6C1";
        switch ((int)node->val)
        {
            case ADD: op_name = "ADD (+)"; color = "#FFD700"; break;
            case SUB: op_name = "SUB (-)"; color = "#FFA07A"; break;
            case MUL: op_name = "MUL (*)"; color = "#DDA0DD"; break;
            case DIV: op_name = "DIV (/)"; color = "#87CEFA"; break;
        }
        fprintf (out, "  N%p [label = \"{ OP | %s }\", fillcolor = \"%s\"];\n",
                (void*)node, op_name, color);
    }

    if (node->left != NULL)
    {
        fprintf (out, "  N%p -> N%p;\n", (void*)node, (void*)node->left);
        Run_Draw (node->left, out);
    }
    if (node->right != NULL)
    {
        fprintf (out, "  N%p -> N%p;\n", (void*)node, (void*)node->right);
        Run_Draw (node->right, out);
    }
}

void                  Tex_Dump                (const struct Node* root, FILE* tex_file, const char* title)
{
    if (root == NULL || tex_file == NULL)
        return;

    fprintf (tex_file, "\\section*{%s}\n$$\n", title);
    Print_Tex (root, tex_file);
    fprintf (tex_file, "\n$$\n\n");
}

void                  Print_Tex               (const struct Node* node, FILE* out)
{
    if (node == NULL)
        return;

    if (node->type == NUM)
    {
        if (node->val < 0)
            fprintf (out, "(%g)", node->val);
        else
            fprintf (out, "%g", node->val);
        return;
    }

    if (node->type == VAR)
    {
        fprintf (out, "%c", (char)node->val);
        return;
    }

    if (node->type == OP)
    {
        int op = (int)node->val;

        if (op == DIV)
        {
            fprintf (out, "\\frac{");
            Print_Tex (node->left, out);
            fprintf (out, "}{");
            Print_Tex (node->right, out);
            fprintf (out, "}");
            return;
        }

        int need_left_brackets  = (op == MUL && node->left->type == OP && ((int)node->left->val == ADD || (int)node->left->val == SUB));
        int need_right_brackets = (op == MUL && node->right->type == OP && ((int)node->right->val == ADD || (int)node->right->val == SUB)) ||
                                  (op == SUB && node->right->type == OP && ((int)node->right->val == ADD || (int)node->right->val == SUB));

        if (need_left_brackets)
        {
            fprintf (out, "(");
        }

        Print_Tex (node->left, out);

        if (need_left_brackets)
        {
            fprintf (out, ")");
        }

        switch (op)
        {
            case ADD:
            {
                fprintf (out, " + "); 
                break;
            }

            case SUB: 
            {
                fprintf (out, " - "); 
                break;
            }

            case MUL: 
            {
                fprintf (out, " \\cdot "); 
                break;
            }

        }
            if (need_right_brackets)
            {
                fprintf (out, "(");
            }
            Print_Tex (node->right, out);
            if (need_right_brackets)
            {
            fprintf (out, ")");
            }
    }
}
