/**
 * @file ast.h
 * @brief AST nodes and attributes
 *
 * This module provides declarations of all structures and framework functions
 * for the AST hierarchy. It is a large interface that forms the foundation of
 * several compiler phases and will be necessary for Projects 2 (parsing), 3
 * (static analysis), and 4 (code generation).
 */
#ifndef __AST_H
#define __AST_H

#include "common.h"

/**
 * @brief Function pointer used to store references to custom DOT output routines
 */
typedef void (*AttributeValueDOTPrinter)(void*, FILE*);

/**
 * @brief Function pointer used to store references to custom "free" routines
 */
typedef void (*Destructor)(void*);

/**
 * @brief Fake "free" function that does nothing
 *
 * Useful as a destructor for integral attributes that are smaller than machine
 * pointers (and so can be stored directly in the "value" field of the
 * Attribute struct).
 */
void dummy_free(void*);

/**
 * @brief Fake "print" function that does nothing
 *
 * Useful as a printer for attributes that have no useful representation (e.g.,
 * pointers to other nodes).
 */
void dummy_print(void*, FILE*);

/**
 * @brief Simple "print" function that prints an attribute value as an integer
 *
 * Useful as a printer for integral attributes that are smaller than machine
 * pointers (and so can be stored directly in the "value" field of the
 * Attribute struct).
 */
void int_attr_print(void*, FILE*);


/**
 * @brief AST node type tag
 */
typedef enum NodeType {
    PROGRAM, VARDECL, FUNCDECL, BLOCK,
    ASSIGNMENT, CONDITIONAL, WHILELOOP, RETURNSTMT, BREAKSTMT, CONTINUESTMT,
    BINARYOP, UNARYOP, LOCATION, FUNCCALL, LITERAL
} NodeType;

/**
 * @brief Return a string representation of a node type
 *
 * @param type Type to convert to string
 * @returns String representation
 */
const char* NodeType_to_string(NodeType type);

/**
 * @brief AST root structure
 */
typedef struct ProgramNode {
    struct NodeList* variables; /**< @brief List of global variable declarations */
    struct NodeList* functions; /**< @brief List of function declarations */
} ProgramNode;

/**
 * @brief Allocate a new program AST node
 * 
 * This allocator does not require a source code line because it is assumed
 * that the program begins on line 1.
 * @param vars List of global variable declarations
 * @param funcs List of function declarations
 * @returns Allocated AST node
 */
struct ASTNode* ProgramNode_new (struct NodeList* vars, struct NodeList* funcs);

/**
 * @brief AST variable structure
 */
typedef struct VarDeclNode {
    char name[MAX_ID_LEN];      /**< @brief Variable name */
    DecafType type;             /**< @brief Variable type */
    bool is_array;              /**< @brief True if the variable is an array, false if it's a scalar */
    int array_length;           /**< @brief Length of array (should be 1 if not an array) */
} VarDeclNode;

/**
 * @brief Allocate a new variable declaration AST node
 * 
 * @param name Variable name
 * @param type Variable type
 * @param is_array Whether the variable is an array
 * @param array_length If @c is_array, the length of the array
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* VarDeclNode_new (const char* name, DecafType type,
                                 bool is_array, int array_length, int source_line);

/**
 * @brief AST parameter (used in function declarations)
 */
typedef struct Parameter {
    char name[MAX_ID_LEN];      /**< @brief Parameter formal name */
    DecafType type;             /**< @brief Parameter type */
    struct Parameter* next;     /**< @brief Pointer to next parameter (if in a list) */
} Parameter;

/*
 * Declare ParameterList to be a linked list of Parameter* elements.
 */
DECL_LIST_TYPE(Parameter, struct Parameter*)

/**
 * @brief Allocate and add a new parameter to a list of parameters
 * 
 * @param list List to add the new parameter to
 * @param name Name of new parameter
 * @param type Type of new parameter
 * @returns Allocated parameter
 */
void ParameterList_add_new (ParameterList* list, const char* name, DecafType type);

/**
 * @brief AST function structure
 */
typedef struct FuncDeclNode {
    char name[MAX_ID_LEN];      /**< @brief Function name */
    DecafType return_type;      /**< @brief Function return type */
    ParameterList* parameters;  /**< @brief List of formal parameters */
    struct ASTNode* body;       /**< @brief Function body block */
} FuncDeclNode;

/**
 * @brief Allocate a new function declaration AST node
 * 
 * @param name Function name
 * @param return_type Function return type
 * @param parameters List of function parameters
 * @param body Body of function (block)
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* FuncDeclNode_new (const char* name, DecafType return_type, ParameterList* parameters,
                                  struct ASTNode* body, int source_line);

/**
 * @brief AST block structure
 */
typedef struct BlockNode {
    struct NodeList* variables; /**< @brief List of local variable declarations in the block */
    struct NodeList* statements;/**< @brief List of statements in the block */
} BlockNode;

/**
 * @brief Allocate a new block declaration AST node
 * 
 * @param vars List of local variable declarations
 * @param stmts List of body statements
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* BlockNode_new (struct NodeList* vars, struct NodeList* stmts, int source_line);

/**
 * @brief AST assignment structure
 */
typedef struct AssignmentNode {
    struct ASTNode* location;   /**< @brief Left-hand side of assignment */ 
    struct ASTNode* value;      /**< @brief Right-hand side of assignment */
} AssignmentNode;

/**
 * @brief Allocate a new assignment statement AST node
 * 
 * @param location Left-hand side of assignment
 * @param value Right-hand side of assignment
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* AssignmentNode_new (struct ASTNode* location, struct ASTNode* value, int source_line);

/**
 * @brief AST conditional structure
 * 
 * @c else_block can be @c NULL for @c if statements with no @c else clause.
 */
typedef struct ConditionalNode {
    struct ASTNode* condition;  /**< @brief Guard condition (expression) */
    struct ASTNode* if_block;   /**< @brief Block to be executed if the condition is true */
    struct ASTNode* else_block; /**< @brief Block to be executed if the condition is false
                                            (can be @c NULL if there is no else-block) */
} ConditionalNode;

/**
 * @brief Allocate a new conditional statement AST node
 * 
 * @param condition Conditional expression
 * @param if_block Block to be executed if conditional is true
 * @param else_block Block to be executed if conditional is false (may be @c
 * NULL if there is no such block)
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* ConditionalNode_new (struct ASTNode* condition, struct ASTNode* if_block,
                                     struct ASTNode* else_block, int source_line);

/**
 * @brief AST while loop structure
 */
typedef struct WhileLoopNode {
    struct ASTNode* condition;  /**< @brief Guard condition (expression) */
    struct ASTNode* body;       /**< @brief Block to be executed while the condition is true */
} WhileLoopNode;

/**
 * @brief Allocate a new while loop statement AST node
 * 
 * @param condition Conditional expression
 * @param body Block to be executed as long as condition is true
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* WhileLoopNode_new (struct ASTNode* condition, struct ASTNode* body, int source_line);

/**
 * @brief AST return statement structure
 * 
 * @c value can be @c NULL for return statements with no value (i.e., in @c void functions).
 */
typedef struct ReturnNode {
    struct ASTNode* value;      /**< @brief Return value (can be @c NULL for void returns) */ 
} ReturnNode;

/**
 * @brief Allocate a new return statement AST node
 * 
 * @param value Return expression (can be @c NULL)
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* ReturnNode_new (struct ASTNode* value, int source_line);

/**
 * @brief Allocate a new break statement AST node
 * 
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* BreakNode_new (int source_line);

/**
 * @brief Allocate a new continue statement AST node
 * 
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* ContinueNode_new (int source_line);

/**
 * @brief Binary operator
 */
typedef enum BinaryOpType {
    OROP, ANDOP, EQOP, NEQOP, LTOP, LEOP, GEOP, GTOP,
    ADDOP, SUBOP, MULOP, DIVOP, MODOP
} BinaryOpType;

/**
 * @brief Convert a @ref BinaryOpType to a string
 * 
 * @param op Operand to convert
 * @returns String representation of binary operator
 */
const char* BinaryOpToString(BinaryOpType op);

/**
 * @brief AST binary operation expression structure
 */
typedef struct BinaryOpNode {
    BinaryOpType operator;      /**< @brief Operator type */
    struct ASTNode* left;       /**< @brief Left operand expression */
    struct ASTNode* right;      /**< @brief Right operand expression */
} BinaryOpNode;

/**
 * @brief Allocate a new binary operation expression AST node
 * 
 * @param operator @ref BinaryOpType flag
 * @param left Left-hand operand
 * @param right Right-hand operand
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* BinaryOpNode_new (BinaryOpType operator,
                                  struct ASTNode* left, struct ASTNode* right,
                                  int source_line);

/**
 * @brief Unary operator
 */
typedef enum UnaryOpType {
    NEGOP, NOTOP
} UnaryOpType;

/**
 * @brief Convert a @ref UnaryOpType to a string
 * 
 * @param op Operand to convert
 * @returns String representation of unary operator
 */
const char* UnaryOpToString(UnaryOpType op);

/**
 * @brief AST unary operation expression structure
 */
typedef struct UnaryOpNode {
    UnaryOpType operator;       /**< @brief Operator type */
    struct ASTNode* child;      /**< @brief Child operand expression */
} UnaryOpNode;

/**
 * @brief Allocate a new unary operation expression AST node
 * 
 * @param operator @ref UnaryOpType flag
 * @param child Child operand
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* UnaryOpNode_new (UnaryOpType operator, struct ASTNode* child, int source_line);

/**
 * @brief AST location expression structure
 * 
 * @c index can be @c NULL for non-array locations.
 */
typedef struct LocationNode {
    char name[MAX_ID_LEN];      /**< @brief Location/variable name */
    struct ASTNode* index;      /**< @brief Index expression (can be @c NULL for non-array locations) */
} LocationNode;

/**
 * @brief Allocate a new location expression AST node
 * 
 * @param name Name of location
 * @param index Index expression (or @c NULL if not an array location)
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* LocationNode_new (const char* name, struct ASTNode* index, int source_line);

/**
 * @brief AST function call expression structure
 */
typedef struct FuncCallNode {
    char name[MAX_ID_LEN];      /**< @brief Function name */
    struct NodeList* arguments; /**< @brief List of actual parameters/arguments */
} FuncCallNode;

/**
 * @brief Allocate a new function call expression AST node
 * 
 * @param name Name of function
 * @param args List of actual arguments
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* FuncCallNode_new (const char* name, struct NodeList* args, int source_line);

/**
 * @brief AST literal expression structure
 */
typedef struct LiteralNode {
    DecafType type;                 /**< @brief Literal type (discriminator/tag for the anonymous union) */
    union {
        int integer;                /**< @brief Integer value (if @c type is @c INT) */
        bool boolean;               /**< @brief Boolean value (if @c type is @c BOOL) */
        char string[MAX_LINE_LEN];  /**< @brief String value (if @c type is @c STR) */
    };
} LiteralNode;

/**
 * @brief Allocate a new integer literal expression AST node
 * 
 * @param value Literal value
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* LiteralNode_new_int (int value, int source_line);

/**
 * @brief Allocate a new boolean literal expression AST node
 * 
 * @param value Literal value
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* LiteralNode_new_bool (bool value, int source_line);

/**
 * @brief Allocate a new string literal expression AST node
 * 
 * @param value Literal value
 * @param source_line Source code line where code begins
 * @returns Allocated AST node
 */
struct ASTNode* LiteralNode_new_string (const char* value, int source_line);

/**
 * @brief AST attribute (basically a key-value store for nodes)
 */
typedef struct Attribute
{
    const char* key;        /**< @brief Attribute key */
    void* value;            /**< @brief Attribute value (integral value or pointer to heap) */
    AttributeValueDOTPrinter dot_printer;   /**< @brief Pointer to DOT-printing function
                                                        (can be @c NULL if not printable) */
    Destructor dtor;        /**< @brief Pointer to destructor function that should
                                        be called to deallocate the attribute value
                                        (should be @c NULL if it's an integral value) */
    struct Attribute* next; /**< @brief Next attribute (if stored in a list) */
} Attribute;

/**
 * @brief Main AST node structure
 *
 * Provides some basic definitions used across many nodes, such as source code
 * info and attribute management. Storage of type-specific node data is managed
 * using a tagged union of the other *Node structures declared earlier in this
 * file.
 *
 * AST nodes are designed to be semi-mutable even after parsing by means of the
 * @c attributes key-value mapping that is stored in every node. List of
 * potential attributes (not exhaustive, and most are irrelevant to Project 2):
 *
 * <table border="1">
 * <tr><th>Key</th><th>Description</th></tr>
 * <tr><td>@c parent</td><td>Uptree parent @ref ASTNode reference</td></tr>
 * <tr><td>@c depth</td><td>Tree depth (@c int)</td></tr>
 * <tr><td>@c symbolTable</td><td>Symbol table reference (only in program, function, and block nodes)</td></tr>
 * <tr><td>@c type</td><td>@ref DecafType of node (only in expression nodes)</td></tr>
 * <tr><td>@c staticSize</td><td>Size (in bytes as @c int) of global variables (only in program node)</td></tr>
 * <tr><td>@c localSize</td><td>Size (in bytes as @c int) of local variables (only in function nodes)</td></tr>
 * <tr><td>@c code</td><td>ILOC instructions generated from the subtree rooted at this node</td></tr>
 * <tr><td>@c reg</td><td>Register storing the result of the expression rooted at this node (only in expression nodes)</td></tr>
 * </table>
 * 
 * Generally, the node-type-specific allocators (e.g., @ref ProgramNode_new)
 * should be used to ensure that all of the node-specific data members are
 * initialized correctly. Node structures must be explicitly freed using @ref
 * ASTNode_free.
 * 
 * Methods:
 * - @ref ASTNode_set_attribute
 * - @ref ASTNode_set_int_attribute
 * - @ref ASTNode_set_printable_attribute
 * - @ref ASTNode_has_attribute
 * - @ref ASTNode_get_attribute
 */
typedef struct ASTNode
{
    NodeType type;          /**< @brief Node type (discriminator/tag for the anonymous union) */
    int source_line;        /**< @brief Source code line number */
    Attribute* attributes;  /**< @brief Attribute list (not a formal list because of the provided accessor methods) */
    struct ASTNode* next;   /**< @brief Next node (if stored in a list) */

    /* anonymous union of type-specific node data (C polymorphism) */
    union {
        struct ProgramNode program;
        struct VarDeclNode vardecl;
        struct FuncDeclNode funcdecl;
        struct BlockNode block;
        struct AssignmentNode assignment;
        struct ConditionalNode conditional;
        struct WhileLoopNode whileloop;
        struct ReturnNode funcreturn;
        struct BinaryOpNode binaryop;
        struct UnaryOpNode unaryop;
        struct LocationNode location;
        struct FuncCallNode funccall;
        struct LiteralNode literal;
    };
} ASTNode;

/*
 * Declare NodeList to be a linked list of ASTNode* elements.
 */
DECL_LIST_TYPE(Node, struct ASTNode*)

/**
 * @brief Allocate a new AST node.
 * 
 * Generally, the node-type-specific allocators (e.g., @ref ProgramNode_new)
 * should be used to ensure that all of the node-specific data members are
 * initialized correctly.
 * 
 * Node structures allocated by this or any other allocator must be explicitly
 * freed using @ref ASTNode_free (note that this will recursively free any
 * children, so it is sufficient to free the root of a tree in order to free
 * the entire tree).
 * 
 * @param type Node type
 * @param line Source line (debug info)
 * @returns Pointer to allocated node structure
 */
ASTNode* ASTNode_new (NodeType type, int line);

/**
 * @brief Add or change an attribute for an AST node
 * 
 * Attributes are node-specific key-value pairs. Keys should be static strings
 * and values should be either integral (i.e., it can fit inside a pointer) or
 * a pointer to some structure on the heap. If the latter, you must provide a
 * pointer to a destructor function that can be used to deallocate the
 * attribute value when the node is deallocated.
 * 
 * @param node Node to add the attribute to
 * @param key Attribute key (used to retrieve it later)
 * @param value Attribute value (may be a pointer)
 * @param dtor Pointer to destructor/deallocator function that should be used
 * to free the attribute value when the node is deallocated
 */
void ASTNode_set_attribute (ASTNode* node, const char* key, void* value, Destructor dtor);

/**
 * @brief Add or change a printable attribute for an AST node
 * 
 * This is an overload of @ref ASTNode_set_attribute that requires an
 * additional function pointer that handles printing the value to the DOT graph
 * format.
 * 
 * @param node Node to add the attribute to
 * @param key Attribute key (used to retrieve it later)
 * @param value Attribute value (may be a pointer)
 * @param dot_printer Pointer to printing function that will be used to include
 * the attribute value in DOT graph output
 * @param dtor Pointer to destructor/deallocator function that should be used
 * to free the attribute value when the node is deallocated
 */
void ASTNode_set_printable_attribute (ASTNode* node, const char* key, void* value,
                                      AttributeValueDOTPrinter dot_printer, Destructor dtor);

/**
 * @brief Add or change an integer attribute for an AST node
 * 
 * This is an overload of @ref ASTNode_set_attribute for attributes that are
 * integral; DOT printing and deallocation are handled automatically if you use
 * this function.
 * 
 * @param node Node to add the attribute to
 * @param key Attribute key (used to retrieve it later)
 * @param value Attribute value (may be a pointer)
 */
void ASTNode_set_int_attribute (ASTNode* node, const char* key, int value);

/**
 * @brief Check to see if a node has a particular attribute
 * 
 * @param node Node to check
 * @param key Key to check for
 * @returns True if the node has the requested attribute, false if not
 */
bool ASTNode_has_attribute (ASTNode* node, const char* key);

/**
 * @brief Retrieve a particular attribute from a node
 * 
 * You should immediately cast the returned @c void* pointer to a pointer of
 * the appropriate type.
 * 
 * @param node Node to access
 * @param key Key to retrieve
 * @returns Attribute value
 */
void* ASTNode_get_attribute (ASTNode* node, const char* key);

/**
 * @brief Retrieve a particular integer attribute from a node
 * 
 * This function makes pointer casting unnecessary.
 * 
 * @param node Node to access
 * @param key Key to retrieve
 * @returns Attribute value
 */
int ASTNode_get_int_attribute (ASTNode* node, const char* key);

/**
 * @brief Deallocate an AST node structure
 * 
 * This will recursively free any children, so it is sufficient to free the
 * root of a tree in order to free the entire tree.
 * 
 * It is highly recommended that you subsequently set the pointer to @c NULL so
 * that you do not unintentionally dereference an invalid pointer.
 * 
 * @param node Pointer to structure to free
 */
void ASTNode_free (ASTNode* node);

#endif
