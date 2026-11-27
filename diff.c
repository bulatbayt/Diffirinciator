#include "diff.h"

/*
Написать верификатор

добавить в структуре нода родителя, чтобы легче дебажить

поделить на функции упрощалку                                    ------achieved

функция skip space                                               ------achieved

побольше функций для диффиринцирования (sinx, cosx, lnx, степени) --- achieved

можно поддерживать разные переменные (x,y,z .... массив из 26 элементов, подумать)

дебажный дамп дерева , по шагам (сделать массив строк с фразами, которые будут случайно выбираться и выводиться как дополнение к шагам + такой же массив с соединительными союзами)
*/

int main ()
{
    struct Node* manual_formula = _DIV (_ADD (_NUM (2), _VAR ('x')), _SUB (_NUM (5), _VAR ('y')));
    char* buffer         = NULL;
    struct Node* formula = NULL;

    FILE* Dump_count = fopen ("Dump_count.txt", "w");
    srand(time(NULL)); 

    Input (&buffer, &manual_formula, &formula);
    
    double x = 3, y = 1;
    fprintf (Dump_count, "x = %lg; y = %lg\n\n", x, y);
    double res = Count_Tree (formula, x, y, Dump_count);
    printf ("Result (x=3, y=1): %lg\n", res);

    struct Node* d_formula = Diff (formula, 'x');
    d_formula = Simplify_Tree (d_formula, Dump_count);
    Output (formula, d_formula, res);


    fclose (Dump_count);
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



void                  Input                   (char** buffer, struct Node** manual_formula, struct Node** formula)
{
    if (ReadFromFile ("formula.txt", buffer) > 0 && *buffer != NULL)
    {
        char* ptr = *buffer;
        *formula = Load_Tree_From_Buffer (&ptr);
        free (*buffer);
    }
    else
    {
        *formula = *manual_formula;
        *manual_formula = NULL;
    }

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
    if (strcmp (elem_str, "^")   == 0) return New_Node (OP, POW, NULL, NULL);
    if (strcmp (elem_str, "sin") == 0) return New_Node (OP, SIN, NULL, NULL);
    if (strcmp (elem_str, "cos") == 0) return New_Node (OP, COS, NULL, NULL);
    if (strcmp (elem_str, "ln")  == 0) return New_Node (OP, LN,  NULL, NULL);


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
    SkipSpace (ptr);

    if (**ptr == '\0')
        return NULL;

    if (**ptr == '(')
    {
        (*ptr)++;
        SkipSpace (ptr);

        if (strncmp (*ptr, "sin", 3) == 0 || strncmp (*ptr, "cos", 3) == 0 || strncmp (*ptr, "ln", 2) == 0)
        {
            char func_name[16]= { 0 };
            int k = 0;
            while (isalpha ((unsigned char)**ptr) && k < 15)
            {
                func_name[k++] = *(*ptr)++;
            }
            func_name[k] = '\0';

            struct Node* func_node = Parse_Elem_To_Node (func_name);
            if (func_node == NULL)
                return NULL;

            func_node->left  = NULL;
            func_node->right = Load_Tree_From_Buffer (ptr);

            SkipSpace (ptr);
            if (**ptr == ')')
                (*ptr)++;

            return func_node;
        }

        
        struct Node* left = Load_Tree_From_Buffer (ptr);

        SkipSpace (ptr);

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

        SkipSpace (ptr);

        if (**ptr == ')')
            (*ptr)++;

        return op_node;
    }

    
    
    char elem_str[128] = { 0 };
    int i = 0;
    while (!isspace ((unsigned char)**ptr) &&
           **ptr != '(' && **ptr != ')' &&
           **ptr != '\0' && i < 127)
    {
        elem_str[i++] = **ptr;
        (*ptr)++;
    }
    elem_str[i] = '\0';

    struct Node* atom = Parse_Elem_To_Node (elem_str);
    if (atom == NULL)
        return NULL;

    SkipSpace (ptr);


    if (atom->type == OP && (atom->val == SIN || atom->val == COS || atom->val == LN))
    {
        atom->left  = NULL;
        atom->right = Load_Tree_From_Buffer (ptr);
    }

    return atom;


    return Parse_Elem_To_Node (elem_str);
}




void                  SkipSpace               (char **ptr)
{
    while (isspace((unsigned char)**ptr))
        (*ptr)++;
}

int                   Is_Zero                 (const struct Node* node)
{
    return (node != NULL && node->type == NUM && fabs (node->val) < 1e-9);
}

int                   Is_One                  (const struct Node* node)
{
    return (node != NULL && node->type == NUM && fabs (node->val - 1) < 1e-9);
}



struct Node*          Simplify_Tree           (struct Node* node, FILE* Dump_count) 
{
    if (node == NULL)
        return NULL;

    if (node->type != OP)
        return node;

    node->left  = Simplify_Tree (node->left, Dump_count);
    node->right = Simplify_Tree (node->right, Dump_count);

    int op = (int)node->val;

    struct Node* res_return = Simplify_Num (node, op, Dump_count);
    if (res_return != NULL)
    {
        return res_return;
    }

    res_return = NULL;

    res_return = Simplify_Op (node, op);
    if (res_return != NULL)
    {
        return res_return;
    }
    

    return node;
}

struct Node*          Simplify_Num            (struct Node* node, int op, FILE* Dump_count)
{
    if (node->left == NULL || node->right == NULL)
        return NULL;

    if (node->left->type == NUM && node->right->type == NUM)
    {
        double l   = node->left->val;
        double r   = node->right->val;
        double res = 0;

        switch (op)
        {
            case ADD: 
            {
                Dump_Count_Tree (Dump_count, l, r, "+", l+r);
                res = l + r; 
                break;
            }

            case SUB:
            { 
                Dump_Count_Tree (Dump_count, l, r, "-", l-r);
                res = l - r; 
                break;
            }

            case MUL:
            {
                Dump_Count_Tree (Dump_count, l, r, "*", l*r);
                res = l * r; 
                break;
            }

            case DIV:
            {
                if (fabs (r) < 1e-9) 
                    return node;

                Dump_Count_Tree (Dump_count, l, r, "/", l/r);
                res = l / r;
                break;
            }

            default:
                return node;
        }

        Tree_Destroy (node);
        return _NUM (res);
    }

    return NULL;
}

struct Node*          Simplify_Op             (struct Node* node, int op)
{
    struct Node* res_return = NULL;
    
    switch (op)
    {
        case ADD:
        {
            res_return = Simplify_Add (node);
            if (res_return != NULL)
            {
                return res_return;
            }

            break;
        }

        case SUB:
        {
            res_return = Simplify_Sub (node);
            if (res_return != NULL)
            {
                return res_return;
            }
            
            break;
        }

        case MUL:
        {
            res_return = Simplify_Mul (node);
            if (res_return != NULL)
            {
                return res_return;
            }
            
            break;
        }

        case DIV:
        {
            res_return = Simplify_Div (node);
            if (res_return != NULL)
            {
                return res_return;
            }
            
            break;
        }
    }

    return NULL;
}

struct Node*          Simplify_Add             (struct Node* node)
{
    if (Is_Zero (node->left))  
    {
        struct Node* keep = node->right;
        free (node->left);
        free (node);
        return keep;
    }

    if (Is_Zero (node->right)) 
    {
        struct Node* keep = node->left;
        free (node->right);
        free (node);
        return keep;
    }

    return NULL;
}

struct Node*          Simplify_Sub             (struct Node* node)
{
    if (Is_Zero (node->right)) 
    {
        struct Node* keep = node->left;
        free (node->right);
        free (node);
        return keep;
    }

    return NULL;
}

struct Node*          Simplify_Mul             (struct Node* node)
{
    if (Is_Zero (node->left) || Is_Zero (node->right)) 
    {
        Tree_Destroy (node);
        return _NUM (0);
    }
    if (Is_One (node->left))  
    {
        struct Node* keep = node->right;
        free (node->left);
        free (node);
        return keep;
    }
    if (Is_One (node->right))  
    {
        struct Node* keep = node->left;
        free (node->right);
        free (node);
        return keep;
    }

    return NULL;
}

struct Node*          Simplify_Div             (struct Node* node)
{
    if (Is_Zero (node->left))  
    {
        Tree_Destroy (node);
        return _NUM (0);
    }
    if (Is_One (node->right))  
    {
        struct Node* keep = node->left;
        free (node->right);
        free (node);
        return keep;
    }

    return NULL;
}



double                Count_Tree              (const struct Node* node, double x_val, double y_val, FILE* Dump_count)
{
    assert (node != NULL);

    if (node->type == NUM)
        return node->val;

    if (node->type == VAR)
    {
        if ((char)node->val == 'x') return x_val;
        if ((char)node->val == 'y') return y_val;
        return 0;
    }

    double l = 0;
    double r = 0;

    if ( node->left != NULL)
    {
        l = Count_Tree (node->left, x_val, y_val,  Dump_count);
    }


    if ( node->right != NULL)
    {
        r = Count_Tree (node->right, x_val, y_val, Dump_count);
    }


    switch ((int)node->val)
    {
        case ADD: 
        {
            Dump_Count_Tree (Dump_count, l, r, "+", l+r);
            return l + r;
        }
        case SUB: 
        {
            Dump_Count_Tree (Dump_count, l, r, "-", l-r);
            return l - r;
        }
        case MUL:
        {
            Dump_Count_Tree (Dump_count, l, r, "*", l*r);
            return l * r;
        }
        case DIV:
        {
            if (r != 0)
            {
                Dump_Count_Tree (Dump_count, l, r, "/", l/r);
                return (l / r);
            }
            else
            {
                fprintf (Dump_count, " Очевидно, собрались делить на ноль\n");
                return 0;
            }
        }

        case POW: 
        {
            return pow (l, r);
        }

        case SIN: 
        {
            return sin (r);
        }

        case COS: 
        {
            return cos (r);
        }

        case LN:  
        {   
            if ( r > 0)
                return log(r);
            
            else // что делать с этой ошибкой 
            {
                return 0;
            }
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
        {
            return _NUM (0);
        }
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

                case SIN:
                    return _MUL (_COS (_CR), _DR);

                case COS:
                    return _MUL (_SUB (_NUM (0), _SIN (_CR)), _DR);

                case LN:
                    return _DIV (_DR, _CR);

                case POW:
                    if (node->right->type == NUM)
                    {
                        double n = node->right->val;
                        return _MUL (_MUL (_NUM (n), _POW (_CL, _NUM (n - 1))), _DL);
                    }
                    return _MUL (_POW (_CL, _CR),
                                _ADD (_MUL (_DR, _LN (_CL)),
                                    _DIV (_MUL (_CR, _DL), _CL)));

                default:
                    return NULL;
            }

        
        default:
            return NULL;
    }
}



void                  Output                  (struct Node *formula, struct Node *d_formula, double res)
{
    //Graph_Dump(formula, "formula.dot", "formula.png");
    Graph_Dump(d_formula, "diff.dot", "diff.png");

    FILE *tex = fopen("dump.tex", "w");
    if (tex != NULL)
    {
        fprintf(tex, "\\documentclass{article}\n"
                     "\\usepackage{fontspec}\n"
                     "\\setmainfont{Times New Roman}\n"
                     "\\begin{document}\n");
        Tex_Dump(formula, tex, "Исходное выражение:");
        Tex_Dump(d_formula, tex, "Производная по x:");
        fprintf(tex, "\\end{document}\n");
        fclose(tex);

        system("tectonic dump.tex");
    }

    Html_Dump("dump.html", res, "formula.png", "diff.png", "dump.pdf");
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

}

void                  Run_Draw                (const struct Node* node, FILE* out)
{
    if (node == NULL)
        return;

    if (node->type == NUM)
    {
        fprintf (out, "  N%p [label = \"{ NUM | %lg }\", fillcolor = \"#C0E0FF\"];\n",
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
        const char* op_name = " ";
        const char* color   = "#FFB6C1";
        switch ((int)node->val)
        {
            case ADD: op_name = "ADD (+)"; color = "#FFD700"; break;
            case SUB: op_name = "SUB (-)"; color = "#FFA07A"; break;
            case MUL: op_name = "MUL (*)"; color = "#DDA0DD"; break;
            case DIV: op_name = "DIV (/)"; color = "#87CEFA"; break;
            case POW: op_name = "POW (^)"; color = "#F0E68C"; break;
            case SIN: op_name = "SIN";     color = "#90EE90"; break;
            case COS: op_name = "COS";     color = "#AFEEEE"; break;
            case LN:  op_name = "LN";      color = "#D8BFD8"; break;
        }
        fprintf (out, "  N%p [label = \"{ OP | %s }\", fillcolor = \"%s\"];\n",
                (void*)node, op_name, color);
    }

    if (node->left != NULL)
    {
        fprintf (out, "  N%p -> N%p [dir=both; color = \"#00bc7e\"; arrowsize = 0.7];\n", (void*)node, (void*)node->left);
        Run_Draw (node->left, out);
    }

    if (node->right != NULL)
    {
        fprintf (out, "  N%p -> N%p [dir=both; color = \"#05875c\"; arrowsize = 0.7];\n", (void*)node, (void*)node->right);
        Run_Draw (node->right, out);
    }
}

void                  Html_Dump               (const char* html_file, double eval_res,
                                               const char* img1, const char* img2, const char* pdf_file)
{
    assert (html_file != NULL);

    FILE* html = fopen (html_file, "w");
    if (html == NULL)
        return;

    fprintf (html, "<body style=\"background-color: #222; color: #ff0000;\">\n");

    
   // fprintf (html, "<p style=\"text-align: center; color: red; font-size: 30px;\">============================================================</p>\n");
    fprintf (html, "<p style=\"text-align: center; color: red; font-size: 30px;\">DIFFERENTIATOR DUMP</p>\n");
   // fprintf (html, "<p style=\"text-align: center; color: red; font-size: 30px;\">============================================================</p>\n");

    fprintf (html, " <p style=\"text-align: center; color: red; font-size: 30px;\"> Result: %lg </p>\n\n", eval_res);

    /*if (img1 != NULL)
    {
        fprintf (html, "<img src=\"%s\" alt=\"Original Tree\">\n\n", img1);
    }*/

    /*if (img2 != NULL)
    {
        fprintf (html, "<img src=\"%s\" alt=\"Derivative Tree\">\n\n", img2);
    }*/

    if (pdf_file != NULL && img2 != NULL)
    {
        fprintf (html," <div align=\"center\"><img src=\"FRKT_logo.png\" width=\"400\" height=\"200\" > </div>");
        fprintf (html,"<p> \n </p>");        
        fprintf (html, "<img src=\"%s\" width=\"600\" height=\"400\" \" align=\"left\"> <iframe src=\"%s#toolbar=0\" width=\"600\" height=\"400\" \" align=\"right\"></iframe>\n\n",img2, pdf_file);
    }

    fprintf (html, "</body>\n");
    fclose (html);

    char cmd[256] = { 0 };
    snprintf (cmd, sizeof (cmd), "start %s", html_file);
    system   (cmd);
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
            fprintf (out, "(%lg)", node->val);
        else
            fprintf (out, "%lg", node->val);
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

        if (op == POW)
        {
            fprintf (out, "{");
            Print_Tex (node->left, out);
            fprintf (out, "}^{");
            Print_Tex (node->right, out);
            fprintf (out, "}");
            return;
        }
        
        if (op == SIN || op == COS || op == LN)
        {
            const char* name = (op == SIN) ? "\\sin" : (op == COS) ? "\\cos" : "\\ln";
            fprintf (out, "%s(", name);
            Print_Tex (node->right, out);
            fprintf (out, ")");
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

void                  Dump_Count_Tree         (FILE* Dump_count, double value_l, double value_r, char* op, double result) // создается массивы каждый раз --- вынести за функции, или чтитать из файла?
{
    static const char *imbulki[] = {
        "очевидно, что…",
        "легко заметить, что…",
        "методом пристального взгляда нетрудно понять, что…",
        "не нужно быть семи пядей во лбу, чтобы увидеть, что…",
        "любому внимательному читателю сразу ясно, что…",
        "из соображений здравого смысла очевидно, что…",
        "достаточно взглянуть на это одним глазом, чтобы убедиться, что…",
        "невооружённым глазом видно, что…",
        "как показывает простейший мысленный эксперимент…",
        "интуитивно совершенно прозрачно, что…",
        "нетрудно догадаться, что…",
        "всякий, кто хоть немного знаком с предметом, скажет, что…",
        "само собой разумеется, что…",
        "тривиальным образом получаем, что…",
        "не составляет большого труда убедиться, что…",
        "из общих соображений немедленно следует, что…",
        "уже ребёнку понятно, что…",
        "остаётся лишь кивнуть в сторону очевидного факта, что…",
        "при некоторой доле внимательности становится ясно, что…",
        "читатель без труда проверит, что…",
        "не прибегая к сложным выкладкам, заключаем, что…",
        "в порядке самоочевидности отметим, что…",
        "всё это настолько прозрачно, что и доказывать нечего:…",
        "достаточно перечитать предыдущий абзац, чтобы согласиться, что…",
        "как и следовало ожидать, элементарная проверка показывает, что…"
    };

    static const char *link[] = {
        "Поскольку",
        "Так как",
        "Потому что",
        "Ибо",
        "В силу того что",
        "Ввиду того что",
        "Вследствие чего",
        "Откуда следует, что",
        "А значит,",
        "Следовательно,",
        "Стало быть,",
        "Таким образом,",
        "В связи с чем",
        "При условии что",
        "Если учесть, что",
        "Учитывая, что",
        "Несмотря на то что",
        "Хотя",
        "Тогда как",
        "Между тем как",
        "В то время как",
        "Наряду с тем что",
        "К тому же",
        "Более того,",
        "Сверх того,"
    };

    int r = rand()% (sizeof(imbulki) / sizeof (imbulki[0])); 

    fprintf (Dump_count, "%s ", link[r]);
    fprintf (Dump_count, "%s : %lg %s %lg = %lg\n", imbulki[r], value_l, op, value_r, result);
    
    
} 
