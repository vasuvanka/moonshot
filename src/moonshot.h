
/*
  List: a dynamic-length array
*/
typedef struct{
  void** items;
  int max;
  int n;
} List;

List* new_list(int max);
List* new_default_list();
void* get_from_list(List* ls,int i);
void* remove_from_list(List* ls,int i);
int add_to_list(List* ls,void* e);
void dealloc_list(List* ls);

/*
  Token: a symbol from the input code utilized by the parser
*/
typedef struct{
  char* text;
  int type;
  int line;
} Token;

void deallocate_token(Token* token);

// AST node types
typedef struct{
  void* data;
  int type;
} AstNode;

typedef struct{
  AstNode* node;
  List* list;
} AstListNode;

typedef struct{
  char name[256];
  AstNode* type;
  List* body;
  List* args;
} FunctionNode;

typedef struct{
  List* keys;
  List* vals;
} TableNode;

typedef struct{
  AstNode* l;
  AstNode* r;
} AstAstNode;

typedef struct{
  AstNode* node;
  char* text;
} StringAstNode;

typedef struct{
  char name[256];
  AstNode* num1;
  AstNode* num2;
  AstNode* num3;
  List* body;
} FornumNode;

typedef struct{
  AstNode* lhs;
  AstNode* tuple;
  List* body;
} ForinNode;

typedef struct{
  char text[256];
  AstNode* l;
  AstNode* r;
} BinaryNode;

typedef struct{
  char parent[256];
  char name[256];
  List* ls;
} InterfaceNode;

typedef struct{
  List* interfaces;
  char parent[256];
  char name[256];
  List* ls;
} ClassNode;

/*
  List of all possible tokens
*/

enum TOKENS{

  // Lua reserved keywords
  TK_AND, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_END, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GOTO, TK_IF, TK_IN, TK_LOCAL, TK_NIL, TK_NOT, TK_OR, TK_REPEAT,
  TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,

  // Lua other terminal symbols
  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING,

  // New tokens for vanilla lexing
  TK_PAREN, TK_CURLY, TK_SQUARE,
  TK_COMMENT, TK_QUOTE,
  TK_UNARY, TK_BINARY,
  TK_SPACE, TK_MISC,

  // New tokens specific to Moonshot
  TK_NEW, TK_FINAL, TK_TYPEDEF, TK_VAR,
  TK_INTERFACE, TK_CLASS, TK_EXTENDS, TK_IMPLEMENTS,
  TK_WHERE

};

/*
  List of all grammar rules
*/
enum RULES{
  // NULL,
  AST_BREAK, AST_TYPE_ANY,

  // char*
  AST_LABEL, AST_GOTO, AST_ID, AST_TYPE_BASIC,

  // List*
  AST_STMT, AST_DO, AST_LTUPLE, AST_TYPE_TUPLE,

  // AstListNode*
  AST_REPEAT, AST_WHILE, AST_IF, AST_TYPE_FUNC, AST_TUPLE,

  // ClassNode*
  AST_CLASS,

  // InterfaceNode*
  AST_INTERFACE,

  // FunctionNode*
  AST_FUNCTION,

  // TableNode*
  AST_TABLE,

  // AstAstNode*
  AST_SET, AST_CALL, AST_SUB,

  // Binary node
  AST_BINARY, AST_DEFINE,

  // AstNode*
  AST_RETURN, AST_PAREN,

  // StringAstNode*
  AST_FIELD, AST_LOCAL, AST_UNARY, AST_TYPEDEF, AST_PRIMITIVE,

  // FornumNode*
  AST_FORNUM,

  // ForinNode*
  AST_FORIN,

  // Miscellaneous
  AST_UNKNOWN
};

// Variable scope tracking
void init_scopes();
void dealloc_scopes();
void push_scope();
void pop_scope();
void add_scoped_var(BinaryNode* node);
BinaryNode* get_scoped_var(char* name);

// Type nodes
AstNode* get_type(AstNode* node);
int typed_match(AstNode* l,AstNode* r);

// Node constructors
AstNode* new_node(int type,void* data);
FunctionNode* new_function_node(char* name,AstNode* type,List* args,List* body);
AstListNode* new_ast_list_node(AstNode* ast,List* list);
TableNode* new_table_node(List* keys,List* vals);
AstAstNode* new_ast_ast_node(AstNode* l,AstNode* r);
StringAstNode* new_string_ast_node(char* text,AstNode* ast);
StringAstNode* new_primitive_node(char* text,const char* type);
FornumNode* new_fornum_node(char* name,AstNode* num1,AstNode* num2,AstNode* num3,List* body);
ForinNode* new_forin_node(AstNode* lhs,AstNode* tuple,List* body);
BinaryNode* new_binary_node(char* text,AstNode* l,AstNode* r);
InterfaceNode* new_interface_node(char* name,char* parent,List* ls);
ClassNode* new_class_node(char* name,char* parent,List* interfaces,List* ls);
char* new_string_node(char* msg);

// Parsing interface
AstNode* parse(List* ls);
char* get_parse_error();

// Parser functions
AstNode* parse_interface();
AstNode* parse_typedef();
AstNode* parse_define(AstNode* type);
AstNode* parse_class();
AstNode* parse_basic_type();
AstNode* parse_type();
AstNode* parse_stmt();
AstNode* parse_call();
AstNode* parse_set();
AstNode* parse_function(AstNode* type,int include_body);
AstNode* parse_repeat();
AstNode* parse_string();
AstNode* parse_number();
AstNode* parse_return();
AstNode* parse_fornum();
AstNode* parse_tuple();
AstNode* parse_while();
AstNode* parse_local();
AstNode* parse_table();
AstNode* parse_forin();
AstNode* parse_label();
AstNode* parse_break();
AstNode* parse_goto();
AstNode* parse_expr();
AstNode* parse_lhs();
AstNode* parse_if();
AstNode* parse_do();

// Traversal interface
void traverse(AstNode* root);
List* get_traversal_errors();

// Traversal functions
void* process_stmt(AstNode* node);
void* process_type(AstNode* node);
void* process_define(AstNode* node);
void* process_typedef(AstNode* node);
void* process_primitive(AstNode* node);
void* process_interface(AstNode* node);
void* process_class(AstNode* node);
void* process_function(AstNode* node);
void* process_repeat(AstNode* node);
void* process_ltuple(AstNode* node);
void* process_return(AstNode* node);
void* process_binary(AstNode* node);
void* process_fornum(AstNode* node);
void* process_break(AstNode* node);
void* process_paren(AstNode* node);
void* process_forin(AstNode* node);
void* process_unary(AstNode* node);
void* process_tuple(AstNode* node);
void* process_table(AstNode* node);
void* process_local(AstNode* node);
void* process_while(AstNode* node);
void* process_field(AstNode* node);
void* process_label(AstNode* node);
void* process_goto(AstNode* node);
void* process_call(AstNode* node);
void* process_set(AstNode* node);
void* process_sub(AstNode* node);
void* process_if(AstNode* node);
void* process_do(AstNode* node);
void* process_id(AstNode* node);
