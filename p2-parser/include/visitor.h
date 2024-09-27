/**
 * @file visitor.h
 * @brief AST visitors
 *
 * This module provides declarations of AST visitors. It is not strictly
 * necessary to understand these for Project 2 (parsing), but they are included
 * because the debug output is implemented as a visitor. The declarations in
 * this file will be will be critical for Projects 3 (static analysis) and 4
 * (code generation).
 */
#ifndef __VISITOR_H
#define __VISITOR_H

#include "ast.h"


/*
 * AST TRAVERSAL (VISITOR PATTERN)
 */

/**
 * @brief Node visitor structure
 * 
 * A visitor is basically a collection of function pointers that are invoked as
 * the visitor traverses the AST.
 */
typedef struct NodeVisitor
{
    /**
     * @brief Visitor-specific state information
     */
    void *data;

    /**
     * @brief Pointer to destructor function (used to deallocate the @c data member)
     */
    Destructor dtor;

    /*
     * Traversal routines; each of these is called at the appropriate time as
     * the visitor traverses the AST.
     */

    #ifndef SKIP_IN_DOXYGEN
    void (* previsit_default)     (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_default)     (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_program)     (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_program)     (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_vardecl)     (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_vardecl)     (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_funcdecl)    (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_funcdecl)    (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_block)       (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_block)       (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_assignment)  (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_assignment)  (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_conditional) (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_conditional) (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_whileloop)   (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_whileloop)   (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_return)      (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_return)      (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_break)       (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_break)       (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_continue)    (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_continue)    (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_binaryop)    (struct NodeVisitor* visitor, ASTNode* node);
    void (*  invisit_binaryop)    (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_binaryop)    (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_unaryop)     (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_unaryop)     (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_location)    (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_location)    (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_funccall)    (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_funccall)    (struct NodeVisitor* visitor, ASTNode* node);
    void (* previsit_literal)     (struct NodeVisitor* visitor, ASTNode* node);
    void (*postvisit_literal)     (struct NodeVisitor* visitor, ASTNode* node);
    #endif

} NodeVisitor;

/**
 * @brief Allocate a new generic visitor structure
 * 
 * @returns A pointer to the allocated visitor
 */
NodeVisitor* NodeVisitor_new(void);

/**
 * @brief Perform an AST traversal using the given visitor
 * 
 * @param visitor Visitor structure containing function pointers that will be
 * invoked during the traversal
 * @param node Root of AST structure to traverse
 */
void NodeVisitor_traverse (NodeVisitor* visitor, ASTNode* node);

/**
 * @brief Perform an AST traversal using the given visitor and then deallocate the visitor
 * 
 * This is provided as a shortcut so that traversals can be written as a single
 * line in the compiler driver routine. For example:
 * 
 *     NodeVisitor_traverse_and_free(PrintVisitor_new(stdout), tree);
 * 
 * @param visitor Visitor structure containing function pointers that will be
 * invoked during the traversal
 * @param node Root of AST structure to traverse
 */
void NodeVisitor_traverse_and_free (NodeVisitor* visitor, ASTNode* node);

/**
 * @brief Deallocate a visitor structure
 * 
 * @param visitor Visitor to deallocate
 */
void NodeVisitor_free (NodeVisitor* visitor);


/*
 * VISITORS
 */

/**
 * @brief Create a new AST debug print visitor
 * 
 * @param output File stream for the print output
 * @returns Pointer to visitor structure
 */
NodeVisitor* PrintVisitor_new (FILE* output);

/**
 * @brief Create a new AST debug graph output visitor
 * 
 * The output is in DOT format: https://graphviz.org
 * 
 * To convert the output to a PNG (for example):
 * 
 *     dot -Tpng -o ast.png ast.dot
 * 
 * @param output File stream for the DOT output
 * @returns Pointer to visitor structure
 */
NodeVisitor* GenerateASTGraph_new (FILE* output);

/**
 * @brief Create a new visitor that sets up parent pointers as attributes
 * 
 * These parent pointers are used in other visitors.
 * 
 * @returns Pointer to visitor structure
 */
NodeVisitor* SetParentVisitor_new(void);

/**
 * @brief Create a new visitor that calculates node depths as attributes
 * 
 * These depths are used during debug output.
 * 
 * @returns Pointer to visitor structure
 */
NodeVisitor* CalcDepthVisitor_new (void);

#endif
