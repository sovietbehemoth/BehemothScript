#include "../include/BehemothScript.h"

#define LEFT 0
#define RIGHT 1

#define HAS_T(c) if (contains(input, c))

typedef struct Token {
    int position;
    int token;
    int type;
} token_t;

typedef struct Match {
    int right_position;
    int left_position;
} match_t;



static token_t * lexer_remove_popped(token_t *matches, int length, int target) {
    token_t * matches_copy = (token_t*) _calloc(length, sizeof(token_t));

    int head = 0;

    for (int i = 0; i < length; i++) {
        if (i == target || i == target+1) {
            continue;
        } 

        //Copy from old stack to new stack.
        _memcpy(&matches_copy[head], &matches[i], sizeof(token_t)); 
        head++;
    }

    return matches_copy;
}



bool escaped(char *input, int n) {
    bool escape = false;

    if (n-1 < 0) {
        return false;
    }

    for (int i = n-1; i > 0; i--) {
        if (input[i] == '\\') {
            if (escape) {
                escape = false;
            } else escape = true;
        } else {
            return escape;
        }
    }
}



static match_t *lex_pairs(char *input, char open, char close) {
    int stack_size = 10;
    int stack_head = 0;

    int rcount = 0;
    int lcount = 0;

    bool comment = false;
    bool literal = false;
    char lit_type = '\0';

    token_t * stack = (token_t*) _calloc(stack_size, sizeof(token_t));

    for (int i = 0; i < strlen(input); i++) {

        if (comment) {
            if (input[i] == '\n') {
                comment = false;
                continue;
            } 
        }

        if (i+1 <= strlen(input)) {
            if (input[i] == '/' && input[i+1] == '/') {
                comment = true;
            } 
        }

        if (comment) {
            continue;
        }

        if ((input[i] == '"' || input[i] == '\'') && !escaped(input, i)) {
            if (literal) {
                if (input[i] == lit_type) {
                    literal = false;
                }
            } else {
                literal = true;

                lit_type = input[i];
            }
        } else if (literal) continue;

        if (input[i] == open || input[i] == close) {

            if (stack_head == stack_size) {
                stack_size += 5;
                stack = realloc(stack, stack_size * sizeof(token_t));
            }

            stack[stack_head].position = i;

            if (input[i] == open) {
                stack[stack_head].type = LEFT;
                lcount++;
            } else {
                stack[stack_head].type = RIGHT;
                rcount++;
            }

            stack_head++;
        }
    }

    if (stack_size != stack_head) {
        int rsize = rcount + lcount;
        stack = realloc(stack, rsize * sizeof(token_t));
    }

    if (lcount != rcount) {
        free(stack);

        fprintf(stderr, "\e[0;31mError\e[0;37m: %c and %c instances unmatched.\n", open, close);
        printf("Debug: %d total indexes.\n capacity: %d\n size: %d\n", lcount+rcount, stack_size, stack_head);
        exit(1);
    }

    int iterator = 0;
    int matches_head = 0;
    int stack_head_cpy = stack_head;
    int matches_size = 5;

    int iterations = 0;

    match_t * matches = (match_t*) _calloc(matches_size, sizeof(match_t));

    while (stack_head_cpy > 0) {

        if (iterator >= stack_head_cpy) {
            iterator = 0; iterations++;
            continue;
        }

        if (iterator + 1 <= stack_head) {       

            if (stack[iterator].type == LEFT && stack[iterator+1].type == RIGHT) {

                if (matches_size == matches_head) {
                    matches_size += 5;
                    matches = realloc(matches, matches_size * sizeof(match_t));
                }

                matches[matches_head].left_position = stack[iterator].position;
                matches[matches_head].right_position = stack[iterator+1].position;

                //Remove leading tokens.
                _memcpy(stack, lexer_remove_popped(stack, stack_head, iterator), sizeof(token_t) * (stack_head_cpy-2));

                stack_head_cpy -= 2; 
                matches_head++;
            }
        }

        iterator++; iterations++;
    }

    int terminator = (matches_head == 0 ? 0 : matches_head);

    if (matches_head != matches_size) {
        if (matches_head == 0) {
            matches_head = 8; //In case no parenthesis are present in the expression.
        } else {
            matches = realloc(matches, matches_head * sizeof(match_t));
        }
    } 


    //Last member acts as a terminator for the length checker.
    matches[terminator].left_position = -1;
    matches[terminator].right_position = -1;
    

    free(stack);

    return matches;
}



static bool contains(char *input, char c) {

    bool comment = false;
    bool literal = false;
    char lit_type = '\0';

    for (int i = 0; i < strlen(input); i++) {
        if (comment) {
            if (input[i] == '\n') {
                comment = false;
                continue;
            } 
        }

        if (i+1 <= strlen(input)) {
            if (input[i] == '/' && input[i+1] == '/') {
                comment = true;
            } 
        }

        if (comment) {
            continue;
        }

        if ((input[i] == '"' || input[i] == '\'') && !escaped(input, i)) {
            if (literal) {
                if (input[i] == lit_type) {
                    literal = false;
                }
            } else {
                literal = true;

                lit_type = input[i];
            }
        } else if (literal) continue;

        if (input[i] == c) {
            return true;
        }
    } 

    return false;
}



static int lex_length_check_p(match_t *lex) {
    for (int i = 0;; i++) {
        if (lex[i].left_position == -1) {
            return i;
        }
    }
}



tok_t *tokenizer(char *input) {
    tok_t * tokenizer_output = (tok_t*) calloc(1, sizeof(tok_t));

    HAS_T('{') tokenizer_output->curly_braces = lex_pairs(input, '{', '}');
    HAS_T('(') tokenizer_output->parenthesis = lex_pairs(input, '(', ')');
    HAS_T('[') tokenizer_output->square_braces = lex_pairs(input, '[', ']');

    return tokenizer_output;
}



int find_pair(int index, match_t *matches) {
    int length = lex_length_check_p(matches);

    for (int i = 0; i < length; i++) {
        if (matches[i].left_position == index) {
            return matches[i].right_position;
        }
    }

    return 0;
}