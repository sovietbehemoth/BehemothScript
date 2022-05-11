


#define IF_STATEMENT 0

typedef struct Ast ast_t;
typedef struct AST_Condition ast_condition_t;
typedef struct AST_Conditional ast_cblock_t;

typedef struct {
    int type;
} symbol_t;

typedef struct AST_Condition {
    symbol_t *value;
    short boolean_expr;
    ast_condition_t *next;
} ast_condition_t;

typedef struct AST_Conditional {
    ast_condition_t *condition;
    ast_t *_if;
    ast_t *_else;
    ast_cblock_t *_elif;
} ast_cblock_t;

typedef struct Ast {
    int type;
    int n;
    ast_t *next;
} ast_t;