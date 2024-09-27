#include "token.h"

Regex* Regex_new (const char* regex)
{
    Regex* r = (Regex*)calloc(1, sizeof(Regex));
    CHECK_MALLOC_PTR(r)
    /* regcomp initializes a regex_t, for which Regex is just a typedef */
    regcomp(r, regex, REG_EXTENDED);
    return r;
}

bool Regex_match (Regex *regex, const char *text, char *match)
{
    /* only save one element becase, we only care about the whole-regex match */
    regmatch_t matches[1];
    if (regexec(regex, text, 1, matches, 0) == 0) {

        /* save the match into the given string buffer */
        snprintf(match, matches[0].rm_eo+1, "%s", text);
        return true;
    }
    return false;
}

void Regex_free (Regex* regex)
{
    regfree(regex); /* clean up regex_t structure */
    free(regex);
}

const char* TokenType_to_string (TokenType type)
{
    switch (type) {
        case ID:        return "ID";
        case DECLIT:    return "DECLIT";
        case HEXLIT:    return "HEXLIT";
        case STRLIT:    return "STRLIT";
        case KEY:       return "KEYWORD";
        case SYM:       return "SYMBOL";
    }
    return "INVALID";
}

bool token_str_eq (const char* str1, const char* str2)
{
    return strncmp(str1, str2, MAX_TOKEN_LEN) == 0;
}

Token* Token_new (TokenType type, const char* text, int line)
{
    Token* token = (Token*)calloc(1, sizeof(Token));
    CHECK_MALLOC_PTR(token)
    token->type = type;
    snprintf(token->text, MAX_TOKEN_LEN, "%s", text);
    token->line = line;
    token->next = NULL;
    return token;
}

void Token_free (Token* token)
{
    free(token);
}

TokenQueue* TokenQueue_new (void)
{
    TokenQueue* queue = calloc(1, sizeof(TokenQueue));
    CHECK_MALLOC_PTR(queue)
    return queue;
}

void TokenQueue_add (TokenQueue* queue, Token* token)
{
    if (queue->head == NULL) {
        /* empty list: new token is both head and tail */
        queue->head = token;
        queue->tail = token;
    } else {
        /* non-empty list: append to tail */
        queue->tail->next = token;
        queue->tail = token;
    }
}

Token* TokenQueue_peek (TokenQueue* queue)
{
    return queue->head;
}

Token* TokenQueue_remove (TokenQueue* queue)
{
    if (queue->head == NULL) {
        /* queue is empty: return NULL */
        return NULL;
    } else {
        /* queue is non-empty: remove a token from head and return it */
        Token* tmp = queue->head;
        queue->head = queue->head->next;
        if (queue->head == NULL) {
            queue->tail = NULL;    /* just removed the last item */
        }
        return tmp;
    }
}

bool TokenQueue_is_empty (TokenQueue* queue)
{
    return queue->head == NULL;
}

size_t TokenQueue_size (TokenQueue* queue)
{
    size_t size = 0;
    for (Token* cur = queue->head; cur != NULL; cur = cur->next) {
        size++;
    }
    return size;
}

void TokenQueue_print (TokenQueue* queue, FILE* out)
{
    for (Token* t = queue->head; t != NULL; t = t->next) {
        fprintf(out, "%-8s [line %03d]  %s\n",
                TokenType_to_string(t->type),
                t->line, t->text);
    }
}

void TokenQueue_free (TokenQueue* queue)
{
    /* clean up any remaining tokens */
    while (!TokenQueue_is_empty(queue)) {
        Token_free(TokenQueue_remove(queue));
    }
    free(queue);
}
