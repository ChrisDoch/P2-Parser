#include "ast.h"

void dummy_print(void* data, FILE* output)
{
    /* print a placeholder (presumably the data itself isn't suitable for printing) */
    fprintf(output, "(...)");
}

void int_attr_print(void* data, FILE* output)
{
    fprintf(output, "%ld", (long)data);
}

void dummy_free(void* data)
{
    /* do nothing (presumably the data is small enough to fit inside a pointer) */
}

const char* NodeType_to_string(NodeType type)
{
    switch (type) {
        case PROGRAM:       return "Program";
        case VARDECL:       return "VarDecl";
        case FUNCDECL:      return "FuncDecl";
        case BLOCK:         return "Block";
        case ASSIGNMENT:    return "Assignment";
        case CONDITIONAL:   return "Conditional";
        case WHILELOOP:     return "WhileLoop";
        case RETURNSTMT:    return "Return";
        case BREAKSTMT:     return "Break";
        case CONTINUESTMT:  return "Continue";
        case BINARYOP:      return "BinaryOp";
        case UNARYOP:       return "UnaryOp";
        case LOCATION:      return "Location";
        case FUNCCALL:      return "FuncCall";
        case LITERAL:       return "Literal";
    }
    return "???";
}

/*
 * use macros defined in common.h to implement lists for nodes and parameters
 */
DEF_LIST_IMPL(Node, struct ASTNode*, ASTNode_free)
DEF_LIST_IMPL(Parameter, struct Parameter*, free)

/*
 * this custom add-parameter method handles allocation as well
 */
void ParameterList_add_new (ParameterList* list, const char* name, DecafType type)
{
    Parameter* param = (Parameter*)calloc(1, sizeof(Parameter));
    CHECK_MALLOC_PTR(param)
    snprintf(param->name, MAX_ID_LEN, "%s", name);
    param->type = type;
    ParameterList_add(list, param);
}

ASTNode* ASTNode_new (NodeType type, int source_line)
{
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    CHECK_MALLOC_PTR(node)
    node->type = type;
    node->source_line = source_line;
    node->attributes = NULL;
    node->next = NULL;
    return node;
}

void ASTNode_set_attribute (ASTNode* node, const char* key, void* value, Destructor dtor)
{
    ASTNode_set_printable_attribute(node, key, value, dummy_print, dtor);
}

void ASTNode_set_int_attribute (ASTNode* node, const char* key, int value)
{
    ASTNode_set_printable_attribute(node, key, (void*)(long)value, int_attr_print, dummy_free);
}

void ASTNode_set_printable_attribute (ASTNode* node, const char* key, void* value,
                                      AttributeValueDOTPrinter dot_printer, Destructor dtor)
{
    if (node == NULL) {
        Error_throw_printf("ERROR: Tried to set attribute '%s' without a node pointer\n", key);
    }

    /* allocate new attribute */
    Attribute* attr = (Attribute*)calloc(1, sizeof(Attribute));
    CHECK_MALLOC_PTR(attr)
    attr->key = key;
    attr->value = value;
    attr->dot_printer = dot_printer;
    attr->dtor = dtor;
    attr->next = NULL;

    if (node->attributes == NULL) {
        /* first attribute */
        node->attributes = attr;
    } else {

        /* search existing keys */
        for (Attribute* a = node->attributes; a != NULL; a = a->next) {
            if (strncmp(key, a->key, MAX_ID_LEN) == 0) {

                /* key present; replace with new value */
                a->dtor(a->value);
                a->value = value;
                a->dtor = dtor;
                free(attr);
                return;
            }
        }

        /* key not present; insert at beginning */
        attr->next = node->attributes;
        node->attributes = attr;
    }
}

bool ASTNode_has_attribute (ASTNode* node, const char* key)
{
    if (node == NULL) {
        Error_throw_printf("ERROR: Tried to get attribute '%s' without a node pointer\n", key);
    }
    for (Attribute* a = node->attributes; a != NULL; a = a->next) {
        if (strncmp(key, a->key, MAX_ID_LEN) == 0) {
            return true;
        }
    }
    return false;
}

int ASTNode_get_int_attribute (ASTNode* node, const char* key)
{
    return (int)(long)ASTNode_get_attribute(node, key);
}

void* ASTNode_get_attribute (ASTNode* node, const char* key)
{
    if (node == NULL) {
        Error_throw_printf("ERROR: Tried to get attribute '%s' without a node pointer\n", key);
    }
    for (Attribute* a = node->attributes; a != NULL; a = a->next) {
        if (strncmp(key, a->key, MAX_ID_LEN) == 0) {
            return a->value;
        }
    }
    printf("ERROR: No '%s' attribute\n", key);
    return NULL;
}

void ASTNode_free (ASTNode* node)
{
    /* clean up attributes */
    Attribute* next = node->attributes;
    while (next != NULL) {
        Attribute* cur = next;
        next = cur->next;
        if (cur->dtor != NULL) {
            cur->dtor(cur->value);
        }
        free(cur);
    }

    /* clean up node-specific data */
    switch (node->type) {
        case PROGRAM:
            NodeList_free(node->program.variables);
            NodeList_free(node->program.functions);
            break;
        case FUNCDECL:
            ParameterList_free(node->funcdecl.parameters);
            ASTNode_free(node->funcdecl.body);
            break;
        case BLOCK:
            NodeList_free(node->block.variables);
            NodeList_free(node->block.statements);
            break;
        case ASSIGNMENT:
            ASTNode_free(node->assignment.location);
            ASTNode_free(node->assignment.value);
            break;
        case CONDITIONAL:
            ASTNode_free(node->conditional.condition);
            ASTNode_free(node->conditional.if_block);
            if (node->conditional.else_block != NULL) {
                ASTNode_free(node->conditional.else_block);
            }
            break;
        case WHILELOOP:
            ASTNode_free(node->whileloop.condition);
            ASTNode_free(node->whileloop.body);
            break;
        case RETURNSTMT:
            if (node->funcreturn.value != NULL) {
                ASTNode_free(node->funcreturn.value);
            }
            break;
        case BINARYOP:
            ASTNode_free(node->binaryop.left);
            ASTNode_free(node->binaryop.right);
            break;
        case UNARYOP:
            ASTNode_free(node->unaryop.child);
            break;
        case LOCATION:
            if (node->location.index != NULL) {
                ASTNode_free(node->location.index);
            }
            break;
        case FUNCCALL:
            NodeList_free(node->funccall.arguments);
            break;
        default:
            break;
    }

    /* clean up node itself */
    free(node);
}

ASTNode* ProgramNode_new (NodeList* vars, NodeList* funcs)
{
    ASTNode* node = ASTNode_new(PROGRAM, 1);    /* programs start at line 1 */
    node->program.variables = vars;
    node->program.functions = funcs;
    return node;
}

ASTNode* VarDeclNode_new (const char* name, DecafType type, bool is_array, int array_length, int source_line)
{
    ASTNode* node = ASTNode_new(VARDECL, source_line);
    snprintf(node->vardecl.name, MAX_ID_LEN, "%s", name);
    node->vardecl.type = type;
    node->vardecl.is_array = is_array;
    node->vardecl.array_length = array_length;
    return node;
}

ASTNode* FuncDeclNode_new (const char* name, DecafType return_type, ParameterList* parameters, ASTNode* body, int source_line)
{
    ASTNode* node = ASTNode_new(FUNCDECL, source_line);
    snprintf(node->funcdecl.name, MAX_ID_LEN, "%s", name);
    node->funcdecl.return_type = return_type;
    node->funcdecl.parameters = parameters;
    node->funcdecl.body = body;
    return node;
}

ASTNode* BlockNode_new (NodeList* vars, NodeList* stmts, int source_line)
{
    ASTNode* node = ASTNode_new(BLOCK, source_line);
    node->block.variables = vars;
    node->block.statements = stmts;
    return node;
}

ASTNode* AssignmentNode_new (struct ASTNode* location, struct ASTNode* value, int source_line)
{
    ASTNode* node = ASTNode_new(ASSIGNMENT, source_line);
    node->assignment.location = location;
    node->assignment.value = value;
    return node;
}

ASTNode* ConditionalNode_new (struct ASTNode* condition, struct ASTNode* if_block, struct ASTNode* else_block, int source_line)
{
    ASTNode* node = ASTNode_new(CONDITIONAL, source_line);
    node->conditional.condition = condition;
    node->conditional.if_block = if_block;
    node->conditional.else_block = else_block;
    return node;
}

ASTNode* WhileLoopNode_new (struct ASTNode* condition, struct ASTNode* body, int source_line)
{
    ASTNode* node = ASTNode_new(WHILELOOP, source_line);
    node->whileloop.condition = condition;
    node->whileloop.body = body;
    return node;
}

ASTNode* ReturnNode_new (struct ASTNode* value, int source_line)
{
    ASTNode* node = ASTNode_new(RETURNSTMT, source_line);
    node->funcreturn.value = value;
    return node;
}

ASTNode* BreakNode_new (int source_line)
{
    return ASTNode_new(BREAKSTMT, source_line);
}

ASTNode* ContinueNode_new (int source_line)
{
    return ASTNode_new(CONTINUESTMT, source_line);
}

const char* BinaryOpToString(BinaryOpType op)
{
    switch (op) {
        case OROP:     return "||";
        case ANDOP:    return "&&";
        case EQOP:     return "==";
        case NEQOP:    return "!=";
        case LTOP:     return "<";
        case LEOP:     return "<=";
        case GEOP:     return ">=";
        case GTOP:     return ">";
        case ADDOP:    return "+";
        case SUBOP:    return "-";
        case MULOP:    return "*";
        case DIVOP:    return "/";
        case MODOP:    return "%";
        default:       return "INVALID";
    }
}

ASTNode* BinaryOpNode_new (BinaryOpType operator, struct ASTNode* left, struct ASTNode* right, int source_line)
{
    ASTNode* node = ASTNode_new(BINARYOP, source_line);
    node->binaryop.operator = operator;
    node->binaryop.left = left;
    node->binaryop.right = right;
    return node;
}

const char* UnaryOpToString(UnaryOpType op)
{
    switch (op) {
        case NEGOP:     return "-";
        case NOTOP:     return "!";
        default:        return "INVALID";
    }
}

ASTNode* UnaryOpNode_new (UnaryOpType operator, struct ASTNode* child, int source_line)
{
    ASTNode* node = ASTNode_new(UNARYOP, source_line);
    node->unaryop.operator = operator;
    node->unaryop.child = child;
    return node;
}

ASTNode* LocationNode_new (const char* name, struct ASTNode* index, int source_line)
{
    ASTNode* node = ASTNode_new(LOCATION, source_line);
    snprintf(node->location.name, MAX_ID_LEN, "%s", name);
    node->location.index = index;
    return node;
}

ASTNode* FuncCallNode_new (const char* name, NodeList* args, int source_line)
{
    ASTNode* node = ASTNode_new(FUNCCALL, source_line);
    snprintf(node->funccall.name, MAX_ID_LEN, "%s", name);
    node->funccall.arguments = args;
    return node;
}

ASTNode* LiteralNode_new_int (int value, int source_line)
{
    ASTNode* node = ASTNode_new(LITERAL, source_line);
    node->literal.type = INT;
    node->literal.integer = value;
    return node;
}

ASTNode* LiteralNode_new_bool (bool value, int source_line)
{
    ASTNode* node = ASTNode_new(LITERAL, source_line);
    node->literal.type = BOOL;
    node->literal.boolean = value;
    return node;
}

ASTNode* LiteralNode_new_string (const char* value, int source_line)
{
    ASTNode* node = ASTNode_new(LITERAL, source_line);
    node->literal.type = STR;
    snprintf(node->literal.string, MAX_LINE_LEN, "%s", value);
    return node;
}
