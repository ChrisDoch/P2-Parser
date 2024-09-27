#include "common.h"

const char* DecafType_to_string(DecafType type)
{
    switch (type) {
        case UNKNOWN: return "???";
        case INT:     return "int";
        case BOOL:    return "bool";
        case VOID:    return "void";
        case STR:     return "str";
    }
    return "invalid";
}

void print_escaped_string(const char* string, FILE* output)
{
    for (int i = 0; i < strlen(string); i++) {
        /* escape special characters */
        switch (string[i]) {
            case '\n':  fprintf(output, "\\n");  break;
            case '\t':  fprintf(output, "\\t");  break;
            case '\"':  fprintf(output, "\\\""); break;
            case '\\':  fprintf(output, "\\\\"); break;
            default:    fprintf(output, "%c", string[i]); break;
        }
    }
}

void print_doubly_escaped_string(const char* string, FILE* output)
{
    for (int i = 0; i < strlen(string); i++) {
        /* escape special characters */
        switch (string[i]) {
            case '\n':  fprintf(output, "\\\\n");  break;
            case '\t':  fprintf(output, "\\\\t");  break;
            case '\"':  fprintf(output, "\\\\\\\""); break;
            case '\\':  fprintf(output, "\\\\\\\\"); break;
            default:    fprintf(output, "%c", string[i]); break;
        }
    }
}

