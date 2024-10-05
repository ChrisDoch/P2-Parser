/**
 * @file p2-parser.c
 * @brief Compiler phase 2: parser
 *
 * Names : Christopher Docherty, Annalise Lang
 * AI STATEMENT : we did not use AI for any part of this project.
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

ASTNode* parse_expr(TokenQueue* input); // for use in location and args

ASTNode* parse_lit(TokenQueue* input)
{
  int curline = get_next_token_line(input);
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  Token* t = TokenQueue_peek(input);
  ASTNode* lit = NULL;
  if (t->type == HEXLIT) { // TODO hex
    int num = (int)strtol(t->text, NULL, 16);
    lit = LiteralNode_new_int(num, curline);
  } else if (t->type == STRLIT) { // TODO string 
    int i = 1;
    int j = 0;
    char text[MAX_TOKEN_LEN];
    while (i < sizeof(t->text) && (t->text[i]) && (t->text[i] != '"')) {
      if (t->text[i] == '\\') {
        if (t->text[i + 1] == 'n') {
          text[j++] = '\n';
        } else if (t->text[i + 1] == 't') {
          text[j++] = '\t';
        } else if (t->text[i + 1] == 'r') {
          text[j++] = '\r';
        }
        i += 2;
      } else {
        text[j++] = t->text[i];
        i += 1;
      }
    }
    text[j] = '\0';
    lit = LiteralNode_new_string(text, curline);
  } else if (t->type == DECLIT) { // int
    int num = atoi(t->text);
    lit = LiteralNode_new_int(num, curline);
  } else if (token_str_eq(t->text, "true")) { // true bool
    lit = LiteralNode_new_bool(true, curline);
  } else if (token_str_eq(t->text, "false")) { // false bool
    lit = LiteralNode_new_bool(false, curline);
  } else {
    Error_throw_printf("Unexpected literal\n");
  }
  discard_next_token(input);
  return lit;
}

ASTNode* parse_vardecl(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int line = get_next_token_line(input);
  DecafType t = parse_type(input);
  char NAME[MAX_TOKEN_LEN];
  parse_id(input, NAME);
  int arraylen = 1;
  bool isarray = false;
  if (check_next_token(input, SYM, "[")) { // parse arrays
    isarray = true;
    match_and_discard_next_token (input, SYM, "[");
    while (!check_next_token (input, SYM, "]")) {
      discard_next_token(input);
      arraylen += 1;
    }
    match_and_discard_next_token (input, SYM, "]");
  }
  match_and_discard_next_token(input, SYM, ";");

  ASTNode* val = VarDeclNode_new(NAME, t, isarray, arraylen, line);
  return val;
}

NodeList* parse_args(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  NodeList* args = NodeList_new();
  if (check_next_token(input, SYM, ")")) { // returns if no arguments
    return args;
  }
  NodeList_add(args, parse_expr(input));
  while (token_str_eq(TokenQueue_peek(input)->text, ",")) { // checks for multiple arguments
    match_and_discard_next_token(input, SYM, ",");
    NodeList_add(args, parse_expr(input));
  }
  return args; // returns node list
}

ASTNode* parse_funccall(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int curline = get_next_token_line(input);
  char FUNCNAME[MAX_TOKEN_LEN];
  parse_id(input, FUNCNAME);
  match_and_discard_next_token(input, SYM, "(");
  return FuncCallNode_new(FUNCNAME, parse_args(input), curline);
}

ASTNode* parse_loc(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int curline = get_next_token_line(input);
  ASTNode* array_expr = NULL;
  char LOCNAME[MAX_TOKEN_LEN];
  parse_id(input, LOCNAME);
  if (check_next_token(input, SYM, "[")) { // parse arrays
    match_and_discard_next_token(input, SYM, "[");
    array_expr = parse_expr(input);
    match_and_discard_next_token(input, SYM, "]");
  }
  return LocationNode_new(LOCNAME, array_expr, curline);
}

ASTNode* parse_baseexpr(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  ASTNode* base = NULL;
  Token* t = TokenQueue_peek(input);
  if (token_str_eq(t->text, "(")) { // looks for nexted expression
    base = parse_expr(input);
  } else if (token_str_eq(t->next->text, "(")) { // checks if not nested expession for funccall
    base = parse_funccall(input);
  } else if (t->type == DECLIT || t->type == HEXLIT || t->type == STRLIT || token_str_eq(t->text, "true") || token_str_eq(t->text, "false")) {
    base = parse_lit(input);
  } else if (t->type == ID) { // checks if not funccall if ID it is a loc
    base = parse_loc(input);
  } else {
    Error_throw_printf("Unidentifiable base expression\n");
  }
  return base;
}

const UnaryOpType StringToUnaryOp(char* op)
{
  if (strcmp(op, "-") == 0) {
    return NEGOP;
  } else if (strcmp(op, "!") == 0) {
    return NOTOP;
  } else {
    Error_throw_printf("Unidentifiable unary operator\n");
  }
  return NEGOP;
}

ASTNode* parse_unaryexpr(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int curline = get_next_token_line(input);
  UnaryOpType op;
  bool opisnull = true;
  char *unop[] = {"-", "!"};
  for (int i = 0; i < sizeof(unop)/sizeof(unop[0]); i++) {
    if (token_str_eq(unop[i], TokenQueue_peek(input)->text)) {
      op = StringToUnaryOp(unop[i]);
      discard_next_token(input);
      opisnull = false;
    }
  }
  ASTNode* child = parse_baseexpr(input);
  if (opisnull) {
    return child;
  } else {
    return UnaryOpNode_new(op, child, curline);
  }
}

const BinaryOpType StringToBinaryOp(char* op)
{
  if (strcmp(op, "||") == 0) {
    return OROP;
  } else if (strcmp(op, "&&") == 0) {
    return ANDOP;
  } else if (strcmp(op, "==") == 0) {
    return EQOP;
  } else if (strcmp(op, "!=") == 0) {
    return NEQOP;
  } else if (strcmp(op, "<") == 0) {
    return LTOP;
  } else if (strcmp(op, "<=") == 0) {
    return LEOP;
  } else if (strcmp(op, ">=") == 0) {
    return GEOP;
  } else if (strcmp(op, ">") == 0) {
    return GTOP;
  } else if (strcmp(op, "+") == 0) {
    return ADDOP;
  } else if (strcmp(op, "-") == 0) {
    return SUBOP;
  } else if (strcmp(op, "*") == 0) {
    return MULOP;
  } else if (strcmp(op, "/") == 0) {
    return DIVOP;
  } else if (strcmp(op, "%") == 0) {
    return MODOP;
  } else {
    Error_throw_printf("Unidentifiable binary operator\n");
  }
  return OROP;
}

ASTNode* parse_binexpr(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int curline = get_next_token_line(input);
  BinaryOpType op;
  bool opisnull = true;
  ASTNode* left = parse_unaryexpr(input);
  ASTNode* right = NULL;
  char *binop[] = {"||", "&&", "==", "!=", "<", "<=", ">=", ">", "+", "-", "*", "/", "%"};
  for (int i = 0; i < sizeof(binop)/sizeof(binop[0]); i++) {
    if (token_str_eq(binop[i], TokenQueue_peek(input)->text)) {
      op = StringToBinaryOp(binop[i]);
      opisnull = false;
      discard_next_token(input);
      right = parse_unaryexpr(input);
    }
  }
  if (opisnull) {
    return left;
  } else {
    return BinaryOpNode_new(op, left, right, curline);
  }
}

ASTNode* parse_expr(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  return parse_binexpr(input);
}

ASTNode* parse_block(TokenQueue* input); // declare so can be referenced by parse_stmt

ASTNode* parse_stmt(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  int curline = get_next_token_line(input);
  ASTNode* stmt = NULL;
  Token* token = TokenQueue_peek(input);
  if (token_str_eq((token->next)->text, "=") || token_str_eq((token->next)->text, "[")) { // assignment or array assignment
    ASTNode* lookup = parse_loc(input);
    match_and_discard_next_token(input, SYM, "=");
    ASTNode* expr = parse_expr(input);
    stmt = AssignmentNode_new(lookup, expr, curline);
    match_and_discard_next_token(input, SYM, ";");
  } else if (token_str_eq(token->text, "if")) { // if condition
    match_and_discard_next_token(input, KEY, "if");
    match_and_discard_next_token(input, SYM, "(");
    ASTNode* expr = parse_expr(input);
    match_and_discard_next_token(input, SYM, ")");
    ASTNode* body = parse_block(input);
    ASTNode* body_else = NULL;
    if (token_str_eq(TokenQueue_peek(input)->text, "else")) { // else condition
      match_and_discard_next_token(input, KEY, "else");
      body_else = parse_block(input);
    }
    stmt = ConditionalNode_new(expr, body, body_else, curline);
  } else if (token_str_eq(token->text, "while")) { // while loop
    match_and_discard_next_token(input, KEY, "while");
    match_and_discard_next_token(input, SYM, "(");
    ASTNode* expr = parse_expr(input);
    match_and_discard_next_token(input, SYM, ")");
    ASTNode* body = parse_block(input);
    stmt = WhileLoopNode_new(expr, body, curline);
  } else if (token_str_eq(token->text, "return")) { // return
    ASTNode* type = NULL;
    match_and_discard_next_token(input, KEY, "return");
    if (!check_next_token(input, SYM, ";")) {
      type = parse_expr(input);
    }
    stmt = ReturnNode_new(type, curline);
    match_and_discard_next_token(input, SYM, ";");
  } else if (token_str_eq(token->text, "break")) { // break
    stmt = BreakNode_new(curline);
    match_and_discard_next_token(input, KEY, "break");
    match_and_discard_next_token(input, SYM, ";");
  } else if (token_str_eq(token->text, "continue")) { // continue
    stmt = ContinueNode_new(curline);
    match_and_discard_next_token(input, KEY, "continue");
    match_and_discard_next_token(input, SYM, ";");
  } else if (token_str_eq((token->next)->text, "(")) { // checks if function name
    stmt = parse_funccall(input);
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
  int curline = get_next_token_line(input);
  match_and_discard_next_token(input, SYM, "{");
  NodeList* vars = NodeList_new();
  NodeList* stmts = NodeList_new();
  ASTNode* val = NULL;
  if (check_next_token(input, SYM, "}")) { // checks for empty block
    match_and_discard_next_token(input, SYM, "}");
    return val;
  }
  while (check_next_token(input, KEY, "int") || check_next_token(input, KEY, "bool") || check_next_token(input, KEY, "void")) {
    NodeList_add(vars, parse_vardecl(input)); // checks for variables and adds them to a node list
  }
  while (check_next_token_type(input, KEY) || check_next_token_type(input, ID)) { // checks for lookups and statments
    NodeList_add(stmts, parse_stmt(input)); // checks for variables and adds them to a node list
  }
  val = BlockNode_new(vars, stmts, curline);
  match_and_discard_next_token(input, SYM, "}");
  return val;
}

ParameterList* parse_param(TokenQueue* input)
{
  ParameterList* params = ParameterList_new();
  while (!check_next_token(input, SYM, ")")) {
    if (!check_next_token(input, KEY, "int") && !check_next_token(input, KEY, "bool") && !check_next_token(input, KEY, "void")) {
      Error_throw_printf("invalid parameter type\n");
    }
    DecafType paramt = parse_type(input);
    char NAME[MAX_TOKEN_LEN];
    parse_id(input, NAME);
    ParameterList_add_new(params, NAME, paramt);
    if (!check_next_token(input, SYM, ")")) { // check if not last param
      match_and_discard_next_token(input, SYM, ",");
    }
  }
  return params;
}

ASTNode* parse_funcdecl(TokenQueue* input)
{
  if (TokenQueue_is_empty(input)) {
    Error_throw_printf("Unexpected end of input (expected identifier)\n");
  }
  ParameterList* params = ParameterList_new();
  int line = get_next_token_line(input);
  discard_next_token(input); // discard def
  DecafType t = parse_type(input); // return type
  char FUNCNAME[MAX_TOKEN_LEN];
  parse_id(input, FUNCNAME);
  match_and_discard_next_token(input, SYM, "("); // start of params
  if (!check_next_token(input, SYM, ")")) { // check if not empty
    params = parse_param(input);
  }
  match_and_discard_next_token(input, SYM, ")");
  ASTNode* block = parse_block(input);
  ASTNode* val = FuncDeclNode_new(FUNCNAME, t, params, block, line);
  return val;
}

/*
 * node-level parsing functions
 */

ASTNode* parse_program (TokenQueue* input) // reject invalid programs and func parameters statements are an issue because of loops
{
    NodeList* vars = NodeList_new();
    NodeList* funcs = NodeList_new();

    if (input == NULL) {
      Error_throw_printf("NULL token queue\n");
      ASTNode* error = NULL;
      return error;
    }
    
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
