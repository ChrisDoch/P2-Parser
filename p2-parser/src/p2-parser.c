/**
 * @file p2-parser.c
 * @brief Compiler phase 2: parser
 */

#include "p2-parser.h"

/*
 * helper functions
 */

/**
 * @brief Look up the source line of the next token in the queue.
 * 
 * @param input Token queue to examine
 * @returns Source line
 */
int get_next_token_line (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input\n");
    }
    return TokenQueue_peek(input)->line;
}

/**
 * @brief Check next token for a particular type and text and discard it
 * 
 * Throws an error if there are no more tokens or if the next token in the
 * queue does not match the given type or text.
 * 
 * @param input Token queue to modify
 * @param type Expected type of next token
 * @param text Expected text of next token
 */
void match_and_discard_next_token (TokenQueue* input, TokenType type, const char* text)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected \'%s\')\n", text);
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != type || !token_str_eq(token->text, text)) {
        Error_throw_printf("Expected \'%s\' but found '%s' on line %d\n",
                text, token->text, get_next_token_line(input));
    }
    Token_free(token);
}

/**
 * @brief Remove next token from the queue
 * 
 * Throws an error if there are no more tokens.
 * 
 * @param input Token queue to modify
 */
void discard_next_token (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input\n");
    }
    Token_free(TokenQueue_remove(input));
}

/**
 * @brief Look ahead at the type of the next token
 * 
 * @param input Token queue to examine
 * @param type Expected type of next token
 * @returns True if the next token is of the expected type, false if not
 */
bool check_next_token_type (TokenQueue* input, TokenType type)
{
    if (TokenQueue_is_empty(input)) {
        return false;
    }
    Token* token = TokenQueue_peek(input);
    return (token->type == type);
}

/**
 * @brief Look ahead at the type and text of the next token
 * 
 * @param input Token queue to examine
 * @param type Expected type of next token
 * @param text Expected text of next token
 * @returns True if the next token is of the expected type and text, false if not
 */
bool check_next_token (TokenQueue* input, TokenType type, const char* text)
{
    if (TokenQueue_is_empty(input)) {
        return false;
    }
    Token* token = TokenQueue_peek(input);
    return (token->type == type) && (token_str_eq(token->text, text));
}

/**
 * @brief Parse and return a Decaf type
 * 
 * @param input Token queue to modify
 * @returns Parsed type (it is also removed from the queue)
 */
DecafType parse_type (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected type)\n");
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != KEY) {
        Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    DecafType t = VOID;
    if (token_str_eq("int", token->text)) {
        t = INT;
    } else if (token_str_eq("bool", token->text)) {
        t = BOOL;
    } else if (token_str_eq("void", token->text)) {
        t = VOID;
    } else {
        Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    Token_free(token);
    return t;
}

/**
 * @brief Parse and return a Decaf identifier
 * 
 * @param input Token queue to modify
 * @param buffer String buffer for parsed identifier (should be at least
 * @c MAX_TOKEN_LEN characters long)
 */
void parse_id (TokenQueue* input, char* buffer)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected identifier)\n");
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != ID) {
        Error_throw_printf("Invalid ID '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    snprintf(buffer, MAX_ID_LEN, "%s", token->text);
    Token_free(token);
}

ASTNode* parse_vardecl(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int line = get_next_token_line(input);
  DecafType t = parse_type(input);
  char* NAME[MAX_TOKEN_LEN];
  parse_id(input, NAME);
  int arraylen = 0;
  bool isarray = false;
  if (check_next_token(input, SYM, "[")) { // parse arrays
    isarray = true;
    match_and_discard_next_token(input, SYM, "[");
    arraylen = TokenQueue_remove(input);
    match_and_discard_next_token(input, SYM, "]");
  }
  match_and_discard_next_token(input, SYM, ";");

  ASTNode* val = VarDeclNode_new(NAME, t, isarray, arraylen, line);
  return val;
}

ASTNode* parse_loc(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
}

ASTNode* parse_expr(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
}

ASTNode* parse_stmt(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  ASTNode* stmt;
  Token* token = TokenQueue_peek(input);
  token = token->next;
  if (token_str_eq((token->next)->text, "(")) { // checks if function name
      NodeList_add(stmts, parse_func)
  } else if (token_str_eq((token->next)->text, "=")) {
  } else if (token->text == "if") {
  } else if (token->text == "else") {
  } else if (token->text == "while") {
  } else if (token->text == "return") {
    ASTNode* type;
    if (check_next_token(input, SYM, ";")) {
      // nothing
    } else {
      type = parse_expr(input);
    }
    stmt = ReturnNode_new(type, get_next_token_line(input));
    match_and_discard_next_token(input, SYM, ";");
  } else if (token->text == "break") {
    match_and_discard_next_token(input, KEY, "break");
    match_and_discard_next_token(input, SYM, ";");
  } else if (token->text == "continue") {
    match_and_discard_next_token(input, KEY, "continue");
    match_and_discard_next_token(input, SYM, ";");
  } else {
    Error_throw_printf("Unexpected token in block\n");
  }
  return stmt;
}

ASTNode* parse_block(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  match_and_discard_next_token(input, SYM, "{");
  NodeList* vars = NodeList_new();
  NodeList* stmts = NodeList_new();
  ASTNode* val;
  int line = get_next_token_line(input);
  if (check_next_token(input, SYM, "}")) { // checks for empty block
    match_and_discard_next_token(input, SYM, "}");
    return val;
  }
  while (check_next_token(input, KEY, "int") || check_next_token(input, KEY, "bool") || check_next_token(input, KEY, "void")) { // NEEDS CHANGING to account for Decaftypes specifically, not keys
    NodeList_add(vars, parse_vardecl(input)); // checks for variables and adds them to a node list
  }
  while (check_next_token_type(input, KEY) || check_next_token_type(input, ID)) { // checks for lookups and statments
    NodeList_add(stmts, parse_vardecl(input)); // checks for variables and adds them to a node list
  }
  val = BlockNode_new(vars, stmts, line);
  match_and_discard_next_token(input, SYM, "}");
  return val;
}

ASTNode* parse_funcdecl(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  ParameterList* params;
  int line = get_next_token_line(input);
  discard_next_token(input); // discard def
  DecafType t = parse_type(input); // return type
  char* FUNCNAME[MAX_TOKEN_LEN];
  parse_id(input, FUNCNAME);
  match_and_discard_next_token(input, SYM, "("); // start of params
  if (!check_next_token(input, SYM, ")")) { // check if not empty
    while (!check_next_token(input, SYM, ")")) {
        DecafType paramt = parse_type(input);
        char* NAME[MAX_TOKEN_LEN];
        parse_id(input, NAME);
        ParameterList_add_new(params, NAME, paramt);
        if (!check_next_token(input, SYM, ")")) { // check if not last param
            match_and_discard_next_token(input, SYM, ",");
        }
    }
  }
  match_and_discard_next_token(input, SYM, ")");
  ASTNode* block = parse_block(input);
  ASTNode* val = FuncDeclNode_new(FUNCNAME, t, params, block, line);
  return val;
}

/*
 * node-level parsing functions
 */

ASTNode* parse_program (TokenQueue* input)
{
    NodeList* vars = NodeList_new();
    NodeList* funcs = NodeList_new();
    
    while (!TokenQueue_is_empty(input)) {
      Token* start = TokenQueue_peek(input);
      if (strcmp(start->text, "int") == 0 || strcmp(start->text, "bool") == 0 || strcmp(start->text, "void") == 0) {
        NodeList_add(vars, parse_vardecl(input));
      } else if (strcmp(start->text, "def") == 0) {
        NodeList_add(funcs, parse_funcdecl(input));
      } else {
        Error_throw_printf("Unexpected input (expected Variable or Function)\n");
      }
    }

    return ProgramNode_new(vars, funcs);
}

ASTNode* parse (TokenQueue* input)
{
    return parse_program(input);
}
