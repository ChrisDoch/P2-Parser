#include "visitor.h"


/*
 * AST TRAVERSAL (VISITOR PATTERN)
 */

void do_nothing (NodeVisitor* visitor, ASTNode* node)
{
    /* do literally nothing (default action for visitors) */
}

NodeVisitor* NodeVisitor_new(void)
{
    NodeVisitor* v = (NodeVisitor*)calloc(1, sizeof(NodeVisitor));
    CHECK_MALLOC_PTR(v)
    v->data = NULL;
    v->dtor = NULL;
    v->previsit_default      = do_nothing;
    v->postvisit_default     = do_nothing;
    v->previsit_program      = NULL;
    v->postvisit_program     = NULL;
    v->previsit_vardecl      = NULL;
    v->postvisit_vardecl     = NULL;
    v->previsit_funcdecl     = NULL;
    v->postvisit_funcdecl    = NULL;
    v->previsit_block        = NULL;
    v->postvisit_block       = NULL;
    v->previsit_assignment   = NULL;
    v->postvisit_assignment  = NULL;
    v->previsit_conditional  = NULL;
    v->postvisit_conditional = NULL;
    v->previsit_whileloop    = NULL;
    v->postvisit_whileloop   = NULL;
    v->previsit_return       = NULL;
    v->postvisit_return      = NULL;
    v->previsit_break        = NULL;
    v->postvisit_break       = NULL;
    v->previsit_continue     = NULL;
    v->postvisit_continue    = NULL;
    v->previsit_binaryop     = NULL;
    v->invisit_binaryop      = NULL;
    v->postvisit_binaryop    = NULL;
    v->previsit_unaryop      = NULL;
    v->postvisit_unaryop     = NULL;
    v->previsit_location     = NULL;
    v->postvisit_location    = NULL;
    v->previsit_funccall     = NULL;
    v->postvisit_funccall    = NULL;
    v->previsit_literal      = NULL;
    v->postvisit_literal     = NULL;
    return v;
}

#define PREVISIT(TYPE)  if (visitor->previsit_ ## TYPE != NULL)  { visitor->previsit_ ## TYPE (visitor, node); } \
                                                           else  { visitor->previsit_default  (visitor, node); }
#define POSTVISIT(TYPE) if (visitor->postvisit_ ## TYPE != NULL) { visitor->postvisit_ ## TYPE(visitor, node); } \
                                                           else  { visitor->postvisit_default (visitor, node); }

void NodeVisitor_traverse (NodeVisitor* visitor, ASTNode* node)
{
    switch (node->type)
    {
        case PROGRAM:
            PREVISIT(program)
            FOR_EACH(ASTNode*, var, node->program.variables) {
                NodeVisitor_traverse(visitor, var);
            }
            FOR_EACH(ASTNode*, func, node->program.functions) {
                NodeVisitor_traverse(visitor, func);
            }
            POSTVISIT(program)
            break;

        case VARDECL:
            PREVISIT(vardecl)
            POSTVISIT(vardecl)
            break;

        case FUNCDECL:
            PREVISIT(funcdecl)
            NodeVisitor_traverse(visitor, node->funcdecl.body);
            POSTVISIT(funcdecl)
            break;

        case BLOCK:
            PREVISIT(block)
            FOR_EACH (ASTNode*, var, node->block.variables) {
                NodeVisitor_traverse(visitor, var);
            }
            FOR_EACH (ASTNode*, stmt, node->block.statements) {
                NodeVisitor_traverse(visitor, stmt);
            }
            POSTVISIT(block)
            break;

        case ASSIGNMENT:
            PREVISIT(assignment)
            NodeVisitor_traverse(visitor, node->assignment.location);
            NodeVisitor_traverse(visitor, node->assignment.value);
            POSTVISIT(assignment)
            break;

        case CONDITIONAL:
            PREVISIT(conditional)
            NodeVisitor_traverse(visitor, node->conditional.condition);
            NodeVisitor_traverse(visitor, node->conditional.if_block);
            if (node->conditional.else_block != NULL) {
                NodeVisitor_traverse(visitor, node->conditional.else_block);
            }
            POSTVISIT(conditional)
            break;

        case WHILELOOP:
            PREVISIT(whileloop)
            NodeVisitor_traverse(visitor, node->whileloop.condition);
            NodeVisitor_traverse(visitor, node->whileloop.body);
            POSTVISIT(whileloop)
            break;

        case RETURNSTMT:
            PREVISIT(return)
            if (node->funcreturn.value != NULL) {
                NodeVisitor_traverse(visitor, node->funcreturn.value);
            }
            POSTVISIT(return)
            break;

        case BREAKSTMT:
            PREVISIT(break)
            POSTVISIT(break)
            break;

        case CONTINUESTMT:
            PREVISIT(continue)
            POSTVISIT(continue)
            break;

        case BINARYOP:
            PREVISIT(binaryop)
            NodeVisitor_traverse(visitor, node->binaryop.left);
            if (visitor->invisit_binaryop != NULL) {
                visitor->invisit_binaryop(visitor, node);
            }
            NodeVisitor_traverse(visitor, node->binaryop.right);
            POSTVISIT(binaryop)
            break;

        case UNARYOP:
            PREVISIT(unaryop)
            NodeVisitor_traverse(visitor, node->unaryop.child);
            POSTVISIT(unaryop)
            break;

        case LOCATION:
            PREVISIT(location)
            if (node->location.index != NULL) {
                NodeVisitor_traverse(visitor, node->location.index);
            }
            POSTVISIT(location)
            break;

        case FUNCCALL:
            PREVISIT(funccall)
            FOR_EACH (ASTNode*, arg, node->funccall.arguments) {
                NodeVisitor_traverse(visitor, arg);
            }
            POSTVISIT(funccall)
            break;

        case LITERAL:
            PREVISIT(literal)
            POSTVISIT(literal)
            break;

        default:
            Error_throw_printf("ERROR: Unhandled node traversal\n");
            break;
    }
}

void NodeVisitor_traverse_and_free (NodeVisitor* visitor, ASTNode* node)
{
    NodeVisitor_traverse(visitor, node);
    NodeVisitor_free(visitor);
}

void NodeVisitor_free (NodeVisitor* visitor)
{
    /* deallocate any state/data associated with the visitor */
    if (visitor->dtor != NULL) {
        visitor->dtor(visitor->data);
    }
    free(visitor);
}


/*
 * AST VISITOR: PRETTY PRINTING
 */

#define OUTFILE ((FILE*)visitor->data)

#define PRINT_INDENT    long depth = (long)ASTNode_get_attribute(node, "depth"); \
                        for (long i = 0; i < depth; i++) { \
                            fprintf(OUTFILE, "  "); \
                        }

void PrintVisitor_visit_program (NodeVisitor* visitor, ASTNode* node)
{
    fprintf(OUTFILE, "Program [line %d]\n", node->source_line);
}

void PrintVisitor_visit_vardecl (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "VarDecl name=\"%s\" type=%s is_array=%s array_length=%d [line %d]\n",
            node->vardecl.name,
            DecafType_to_string(node->vardecl.type),
            (node->vardecl.is_array ? "yes" : "no"),
            node->vardecl.array_length,
            node->source_line);
}

void PrintVisitor_visit_funcdecl (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "FuncDecl name=\"%s\" return_type=%s parameters={",
            node->funcdecl.name,
            DecafType_to_string(node->funcdecl.return_type));
    bool first = true;
    FOR_EACH (Parameter*, param, node->funcdecl.parameters) {
        fprintf(OUTFILE, "%s%s:%s", (first ? "" : ","), param->name, DecafType_to_string(param->type));
        first = false;
    }
    fprintf(OUTFILE, "} [line %d]\n", node->source_line);
}

void PrintVisitor_visit_assignment (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Assignment [line %d]\n", node->source_line);
}

void PrintVisitor_visit_conditional (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Conditional [line %d]\n", node->source_line);
}

void PrintVisitor_visit_whileloop (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Whileloop [line %d]\n", node->source_line);
}

void PrintVisitor_visit_return (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Return [line %d]\n", node->source_line);
}

void PrintVisitor_visit_block (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Block [line %d]\n", node->source_line);
}

void PrintVisitor_visit_break (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Break [line %d]\n", node->source_line);
}

void PrintVisitor_visit_continue (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Continue [line %d]\n", node->source_line);
}

void PrintVisitor_visit_binaryop (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Binaryop op=\"%s\" [line %d]\n", BinaryOpToString(node->binaryop.operator), node->source_line);
}

void PrintVisitor_visit_unaryop (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Unaryop op=\"%s\" [line %d]\n", UnaryOpToString(node->unaryop.operator), node->source_line);
}

void PrintVisitor_visit_location (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "Location name=\"%s\" [line %d]\n", node->location.name, node->source_line);
}

void PrintVisitor_visit_funccall (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    fprintf(OUTFILE, "FuncCall name=\"%s\" [line %d]\n", node->funccall.name, node->source_line);
}

void PrintVisitor_visit_literal (NodeVisitor* visitor, ASTNode* node)
{
    PRINT_INDENT
    switch (node->literal.type) {
        case INT:
            fprintf(OUTFILE, "Literal type=int value=%d [line %d]", node->literal.integer, node->source_line);
            break;
        case BOOL:
            fprintf(OUTFILE, "Literal type=bool value=%s [line %d]", (node->literal.boolean ? "true" : "false"), node->source_line);
            break;
        case STR:
            fprintf(OUTFILE, "Literal type=string value=\"");
            print_escaped_string(node->literal.string, stdout);
            fprintf(OUTFILE, "\" [line %d]", node->source_line);
            break;
        case VOID:
            fprintf(OUTFILE, "Literal type=void");
            break;
        default: /* UNKNOWN shouldn't be possible as it is an inferred type */
            break;
    }
    fprintf(OUTFILE, "\n");
}

NodeVisitor* PrintVisitor_new (FILE* output)
{
    NodeVisitor* v = NodeVisitor_new();
    /* use "data" field to store FILE* output pointer */
    v->data = (void*)output;
    v->previsit_program     = PrintVisitor_visit_program;
    v->previsit_vardecl     = PrintVisitor_visit_vardecl;
    v->previsit_funcdecl    = PrintVisitor_visit_funcdecl;
    v->previsit_block       = PrintVisitor_visit_block;
    v->previsit_assignment  = PrintVisitor_visit_assignment;
    v->previsit_conditional = PrintVisitor_visit_conditional;
    v->previsit_whileloop   = PrintVisitor_visit_whileloop;
    v->previsit_return      = PrintVisitor_visit_return;
    v->previsit_break       = PrintVisitor_visit_break;
    v->previsit_continue    = PrintVisitor_visit_continue;
    v->previsit_binaryop    = PrintVisitor_visit_binaryop;
    v->previsit_unaryop     = PrintVisitor_visit_unaryop;
    v->previsit_location    = PrintVisitor_visit_location;
    v->previsit_funccall    = PrintVisitor_visit_funccall;
    v->previsit_literal     = PrintVisitor_visit_literal;
    return v;
}


/*
 * AST VISITOR: GRAPH OUTPUT (requires 'dot' utility in GraphViz)
 */

void GenerateASTGraph_assign_dotid (NodeVisitor* visitor, ASTNode* node)
{
    static int next_id = 0;
    ASTNode_set_attribute(node, "dotid", (void*)(long)next_id, dummy_free);
    next_id++;
}

#define GET_ID(NODE) ((int)(long)ASTNode_get_attribute(NODE, "dotid"))
#define GEN_LINK(PARENT,CHILD) fprintf(OUTFILE, "%d -> %d;\n", GET_ID(PARENT), GET_ID(CHILD))

void GenerateASTGraph_generate_dot (NodeVisitor* visitor, ASTNode* node)
{
    /*
     * This is a bit of an anti-pattern here; rather than properly use the
     * visitor functions, we just have a generic print function. That's ok here
     * because we have to handle every kind of node anyway and not much of what
     * we do is type-specific.
     */

    /* generate label */
    int id = GET_ID(node);
    fprintf(OUTFILE, "%d [shape=box, label=\"%s", id, NodeType_to_string(node->type));
    switch (node->type) {
        case VARDECL:  fprintf(OUTFILE, " name='%s'", node->vardecl.name);  break;
        case FUNCDECL: fprintf(OUTFILE, " name='%s'", node->funcdecl.name); break;
        case FUNCCALL: fprintf(OUTFILE, " name='%s'", node->funccall.name); break;
        case BINARYOP: fprintf(OUTFILE, " op='%s'",   BinaryOpToString(node->binaryop.operator)); break;
        case UNARYOP:  fprintf(OUTFILE, " op='%s'",   UnaryOpToString (node->unaryop.operator));  break;
        case LOCATION: fprintf(OUTFILE, " name='%s'", node->location.name); break;
        case LITERAL: {
            switch (node->literal.type) {
                case INT:  fprintf(OUTFILE, " value=%d", node->literal.integer); break;
                case BOOL: fprintf(OUTFILE, " value=%s", (node->literal.boolean ? "true" : "false")); break;
                case STR:  fprintf(OUTFILE, " value='"); print_doubly_escaped_string(node->literal.string, OUTFILE); fprintf(OUTFILE, "'"); break;
                default:   break;
            } break;
        }
        default: break;
    }
    for (Attribute* attr = node->attributes; attr != NULL; attr = attr->next) {
        if (strncmp(attr->key, "dotid", 10) != 0 &&
            strncmp(attr->key, "depth", 10) != 0 &&
            strncmp(attr->key, "parent", 10) != 0) {
            fprintf(OUTFILE, "\\n%s: ", attr->key);
            attr->dot_printer(attr->value, OUTFILE);
        }
    }
    fprintf(OUTFILE, "\"];\n");

    /* create any edges */
    switch (node->type)
    {
        case PROGRAM:
            FOR_EACH(ASTNode*, var,  node->program.variables) { GEN_LINK(node, var); }
            FOR_EACH(ASTNode*, func, node->program.functions) { GEN_LINK(node, func); } break;
        case FUNCDECL:
            GEN_LINK(node, node->funcdecl.body); break;
        case BLOCK:
            FOR_EACH(ASTNode*, var,  node->block.variables)  { GEN_LINK(node, var); }
            FOR_EACH(ASTNode*, stmt, node->block.statements) { GEN_LINK(node, stmt); } break;
        case ASSIGNMENT:
            GEN_LINK(node, node->assignment.location);
            GEN_LINK(node, node->assignment.value); break;
        case CONDITIONAL:
            GEN_LINK(node, node->conditional.condition);
            GEN_LINK(node, node->conditional.if_block);
            if (node->conditional.else_block != NULL) { GEN_LINK(node, node->conditional.else_block); } break;
        case WHILELOOP:
            GEN_LINK(node, node->whileloop.condition);
            GEN_LINK(node, node->whileloop.body); break;
        case RETURNSTMT:
            if (node->funcreturn.value != NULL) { GEN_LINK(node, node->funcreturn.value); } break;
        case BINARYOP:
            GEN_LINK(node, node->binaryop.left);
            GEN_LINK(node, node->binaryop.right); break;
        case UNARYOP:
            GEN_LINK(node, node->unaryop.child); break;
        case FUNCCALL:
            FOR_EACH(ASTNode*, arg, node->funccall.arguments) { GEN_LINK(node, arg); } break;
        case LOCATION:
            if (node->location.index != NULL) { GEN_LINK(node, node->location.index); } break;
        default:
            break;
    }
}

void GenerateASTGraph_initialize (NodeVisitor* visitor, ASTNode* node)
{
    fprintf(OUTFILE, "digraph AST {\n");
    GenerateASTGraph_assign_dotid(visitor, node);
}

void GenerateASTGraph_finalize (NodeVisitor* visitor, ASTNode* node)
{
    GenerateASTGraph_generate_dot(visitor, node);
    fprintf(OUTFILE, "}\n");
}

NodeVisitor* GenerateASTGraph_new (FILE* output)
{
    NodeVisitor* v = NodeVisitor_new();
    /* use "data" field to store FILE* output pointer */
    v->data = (void*)output;
    v->previsit_default      = GenerateASTGraph_assign_dotid;
    v->postvisit_default     = GenerateASTGraph_generate_dot;
    v->previsit_program      = GenerateASTGraph_initialize;
    v->postvisit_program     = GenerateASTGraph_finalize;
    return v;
}


/*
 * AST VISITOR: PARENT POINTER SETUP
 */

void SetParentVisitor_visit_program (NodeVisitor* visitor, ASTNode* node)
{
    FOR_EACH(ASTNode*, var, node->program.variables) {
        ASTNode_set_attribute(var, "parent", (void*)node, NULL);
    }
    FOR_EACH(ASTNode*, func, node->program.functions) {
        ASTNode_set_attribute(func, "parent", (void*)node, NULL);
    }
}

void SetParentVisitor_visit_funcdecl (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_attribute(node->funcdecl.body, "parent", (void*)node, NULL);
}

void SetParentVisitor_visit_block (NodeVisitor* visitor, ASTNode* node)
{
    FOR_EACH(ASTNode*, var, node->block.variables) {
        ASTNode_set_attribute(var, "parent", (void*)node, NULL);
    }
    FOR_EACH(ASTNode*, stmt, node->block.statements) {
        ASTNode_set_attribute(stmt, "parent", (void*)node, NULL);
    }
}

void SetParentVisitor_visit_assignment (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_attribute(node->assignment.location, "parent", (void*)node, NULL);
    ASTNode_set_attribute(node->assignment.value, "parent", (void*)node, NULL);
}

void SetParentVisitor_visit_conditional (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_attribute(node->conditional.condition, "parent", (void*)node, NULL);
    ASTNode_set_attribute(node->conditional.if_block, "parent", (void*)node, NULL);
    if (node->conditional.else_block != NULL) {
        ASTNode_set_attribute(node->conditional.else_block, "parent", (void*)node, NULL);
    }
}

void SetParentVisitor_visit_whileloop (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_attribute(node->whileloop.condition, "parent", (void*)node, NULL);
    ASTNode_set_attribute(node->whileloop.body, "parent", (void*)node, NULL);
}

void SetParentVisitor_visit_return (NodeVisitor* visitor, ASTNode* node)
{
    if (node->funcreturn.value != NULL) {
        ASTNode_set_attribute(node->funcreturn.value, "parent", (void*)node, NULL);
    }
}

void SetParentVisitor_visit_binaryop (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_attribute(node->binaryop.left, "parent", (void*)node, NULL);
    ASTNode_set_attribute(node->binaryop.right, "parent", (void*)node, NULL);
}

void SetParentVisitor_visit_unaryop (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_attribute(node->unaryop.child, "parent", (void*)node, NULL);
}

void SetParentVisitor_visit_location (NodeVisitor* visitor, ASTNode* node)
{
    if (node->location.index != NULL) {
        ASTNode_set_attribute(node->location.index, "parent", (void*)node, NULL);
    }
}

void SetParentVisitor_visit_funccall (NodeVisitor* visitor, ASTNode* node)
{
    FOR_EACH(ASTNode*, arg, node->funccall.arguments) {
        ASTNode_set_attribute(arg, "parent", (void*)node, NULL);
    }
}

NodeVisitor* SetParentVisitor_new(void)
{
    NodeVisitor* v = NodeVisitor_new();
    v->previsit_program = SetParentVisitor_visit_program;
    v->previsit_funcdecl = SetParentVisitor_visit_funcdecl;
    v->previsit_block = SetParentVisitor_visit_block;
    v->previsit_assignment = SetParentVisitor_visit_assignment;
    v->previsit_conditional = SetParentVisitor_visit_conditional;
    v->previsit_whileloop = SetParentVisitor_visit_whileloop;
    v->previsit_return = SetParentVisitor_visit_return;
    v->previsit_binaryop = SetParentVisitor_visit_binaryop;
    v->previsit_unaryop = SetParentVisitor_visit_unaryop;
    v->previsit_location = SetParentVisitor_visit_location;
    v->previsit_funccall = SetParentVisitor_visit_funccall;
    return v;
}


/*
 * AST VISITOR: DEPTH CALCULATION
 */

void CalcDepthVisitor_visit_program (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode_set_int_attribute(node, "depth", 0);
}

void CalcDepthVisitor_visit_nonprogram (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode* parent = (ASTNode*)ASTNode_get_attribute(node, "parent");
    long pdepth = (long)ASTNode_get_int_attribute(parent, "depth");
    ASTNode_set_int_attribute(node, "depth", pdepth + 1);
}

NodeVisitor* CalcDepthVisitor_new (void)
{
    NodeVisitor* v = NodeVisitor_new();
    v->previsit_program  = CalcDepthVisitor_visit_program;
    v->previsit_default  = CalcDepthVisitor_visit_nonprogram;
    return v;
}
