#include "../include/BehemothScript.h"

#define TK_ASSIGN 0         // =
#define TK_EQU 1            // ==
#define TK_ADD 2            // +
#define TK_SUB 3            // -
#define TK_UNARY 4          // *
#define TK_DIV 5            // /
#define TK_WH 6             // ' '
#define TK_LP 7             // (
#define TK_RP 8             // )
#define TK_LCB 9            // {
#define TK_RCB 10           // }
#define TK_LSB 11           // [
#define TK_RSB 12           // ]

#define LEX_DYN_CAP 10
#define LEX_CAP 20
#define LEX_DYN_INC 5
#define LEX_INC 10


/**
 * @brief Determine if character is to be handled specially.
 * 
 * @param buffer Raw buffer.
 * @param n Position in iteration.
 * @return int (-1 if not special)
 */
static int lexer_is_special(char *buffer, int n) {
    char token = buffer[n];

    switch (token) {
        case '=':
            if (strlen(buffer) >= n+1) {
                if (buffer[n+1] == '=') {
                    return TK_EQU;
                }
            } return TK_ASSIGN;
        break;

        case '+': return TK_ADD; break;
        case '-': return TK_SUB; break;
        case '*': return TK_UNARY; break;
        case '/': return TK_DIV; break;

        //Some common empty space ASCII codes.
        case '\n':
        case 9:
        case 13:
        case -31:
        case ' ': return TK_WH; break;

        case '(': return TK_LP; break;
        case ')': return TK_RP; break;
        case '{': return TK_LCB; break;
        case '}': return TK_RCB; break;
        case '[': return TK_LSB; break;
        case ']': return TK_RSB; break;

        default: return -1; break;
    }
}

/**
 * @brief Consumes the current token, pushing it to the index and continuing to the next node.
 * 
 * @param lex The lex stack.
 * @param buffer The raw text buffer.
 * @param n Position in text buffer.
 * @param dyn_capacity How large the current node is allocated.
 * @param lex_head The head of the lex stack.
 */
static int lex_eat_token(char **lex, char *buffer, int n, int *dyn_capacity, int *lex_head) {
    const int len = strlen(lex[*lex_head]);

    //The node should fit the block, trim size.
    if (*dyn_capacity != len) {
        lex[*lex_head] = realloc(lex[*lex_head], CHAR * len);
        *dyn_capacity = 2;
    }

    //0 length node should be used instead of skipped.
    if (len > 0) {
        (*lex_head)++;
    }

    lex[*lex_head] = (char*) calloc(*dyn_capacity, CHAR);

    int retval = 0;

    if (buffer[n] != ' ') {
        if (strlen(buffer) >= n+1) { //'==' is different because it has two characters.
            if (buffer[n+1] == '=' && buffer[n] == '=') {
                strcat(lex[*lex_head], "==");
                retval = 1;
            } else goto regular;
        } else {
            regular:;
            lex[*lex_head][0] = buffer[n];
        }

        //Reset dynamic capacity, increment stack pointer, and allocate it.

        *dyn_capacity = LEX_DYN_CAP;
        (*lex_head)++;
        lex[*lex_head] = (char*) calloc(*dyn_capacity, CHAR);

    } else *dyn_capacity = LEX_DYN_CAP;

    return retval;
}

/**
 * @brief Compile all characters into a consumable stream.
 * 
 * @param buffer Raw file buffer.
 * @param recursion 
 * @return char** 
 */
char **lexer_init(char *buffer) {

    //This structure is defined as a growable heap. The size of the stack is always larger than the
    //actual length, this prevents frequent realloc() calls; the same goes for the individual node
    //allocation protocol.

    //Dynamic capacity (dyn_capacity) is defined as the space allocated for the current node, this space
    //may grow if the node becomes too large, it will also get reverted upon next iteration.

    int lex_capacity = LEX_CAP;            //Capacity of stack.
    int lex_head = 0;                      //Stack pointer, effectively length.
    int dyn_capacity = LEX_DYN_CAP;        //Dynamic capacity.

    //Stack base allocation.
    char **lex = (char**) calloc(lex_capacity, sizeof(char*));

    //Allocate top.
    lex[lex_head] = (char*) calloc(dyn_capacity, CHAR);

    int line = 1;

    for (int i = 0; i < strlen(buffer); i++) {

        if (buffer[i] == '\n') {
            line++;
        }

        // printf("LSIZE=%d, DSIZE=%d, DLEN=%ld LEX[line %d] %d: %c\n", lex_capacity, dyn_capacity, strlen(lex[lex_head]), line, buffer[i], buffer[i]);

        //In case the stack gets too large.
        if (lex_capacity == lex_head) {
            lex_capacity += LEX_INC;
            lex = realloc(lex, sizeof(char*) * lex_capacity);
        }

        const int len = strlen( lex[lex_head] );
        const char cur = buffer[i];

        //Determine if the token should be handled separately.
        int special_token = lexer_is_special(buffer, i);

        if (special_token != -1) {
            if (special_token == TK_WH && i+1 <= strlen(buffer)) {
                //If the next character is special, and the current is whitespace, we can't
                //eat it because then an empty node will be pushed.
                if (lexer_is_special(buffer, i+1) != -1) {
                    continue;
                }
            }

            //Eat token. 'i' will only increment upon special control operators that
            //are more than 1 byte, then we need to increment extra.
            i += lex_eat_token(lex, buffer, i, &dyn_capacity, &lex_head, line);
            continue;
        }


        //The node has reached its capacity.
        if (len >= dyn_capacity) {
            dyn_capacity += LEX_DYN_INC;
            lex[lex_head] = realloc(lex[lex_head], CHAR * dyn_capacity);
        }

        lex[lex_head][len] = cur;

    }

    //It's important to NULL terminate the stack.

    lex_head++;
    lex[lex_head] = NULL;

    return lex;
}