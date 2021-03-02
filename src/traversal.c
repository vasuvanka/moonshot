#include "./internal.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define ERROR(cond,msg,...) if(cond){add_error(-1,msg,__VA_ARGS__);return;}
static int validate;

// Traversal interface
void traverse(AstNode* root,int valid){
  validate=valid;
  init_types();
  init_scopes();
  push_scope();
  process_stmt(root);
  pop_scope();
  dealloc_scopes();
  dealloc_types();
}

// Output
static void write(const char* msg,...){
  if(validate) return;
  va_list args;
  int n=strlen(msg);
  va_start(args,msg);
  for(int a=0;a<n;a++){
    if(a<n-1 && msg[a]=='%'){
      if(msg[a+1]=='s'){
        printf("%s",va_arg(args,char*));
        a++;
        continue;
      }
    }
    printf("%c",msg[a]);
  }
  va_end(args);
}

// Node to function switch
void process_node(AstNode* node){
  int type=node->type;
  switch(node->type){
    case AST_STMT: process_stmt(node); return;
    case AST_PRIMITIVE: process_primitive(node); return;
    case AST_INTERFACE: process_interface(node); return;
    case AST_FUNCTION: process_function(node); return;
    case AST_TYPEDEF: process_typedef(node); return;
    case AST_DEFINE: process_define(node); return;
    case AST_REPEAT: process_repeat(node); return;
    case AST_LTUPLE: process_ltuple(node); return;
    case AST_RETURN: process_return(node); return;
    case AST_BINARY: process_binary(node); return;
    case AST_FORNUM: process_fornum(node); return;
    case AST_CLASS: process_class(node); return;
    case AST_BREAK: process_break(node); return;
    case AST_FORIN: process_forin(node); return;
    case AST_PAREN: process_paren(node); return;
    case AST_UNARY: process_unary(node); return;
    case AST_TUPLE: process_tuple(node); return;
    case AST_TABLE: process_table(node); return;
    case AST_LOCAL: process_local(node); return;
    case AST_WHILE: process_while(node); return;
    case AST_FIELD: process_field(node); return;
    case AST_LABEL: process_label(node); return;
    case AST_GOTO: process_goto(node); return;
    case AST_CALL: process_call(node); return;
    case AST_SET: process_set(node); return;
    case AST_SUB: process_sub(node); return;
    case AST_IF: process_if(node); return;
    case AST_DO: process_do(node); return;
    case AST_ID: process_id(node); return;
    default: printf("Uh oh, you shouldn't be here (%i)\n",node->type);
  }
}

// Extended grammar
void process_define(AstNode* node){
  BinaryNode* data=(BinaryNode*)(node->data);
  if(validate){
    ERROR(!compound_type_exists(data->l),"reference to nonexistent type %t",data->l);
    if(data->r){
      AstNode* tr=get_type(data->r);
      ERROR(!typed_match(data->l,tr),"expression of type %t cannot be assigned to variable of type %t",tr,data->l);
    }
    ERROR(!add_scoped_var(data),"variable %s was already declared in this scope",data->text);
  }
  write("local %s=",data->text);
  if(data->r) process_node(data->r);
  else write("nil");
  write("\n");
}
void process_typedef(AstNode* node){
  StringAstNode* data=(StringAstNode*)(node->data);
  if(validate){
    ERROR(type_exists(data->text),"type %s is already declared",data->text);
    ERROR(!compound_type_exists(data->node),"type %t does not exist",data->node);
    ERROR(!add_type_equivalence(data->text,data->node),"co-dependent typedef %s detected",data->text);
    register_type(data->text);
  }
  char* type1=stringify_type(data->node);
  write("-- typedef %s -> %s\n",data->text,type1);
  free(type1);
}
void process_interface(AstNode* node){
  InterfaceNode* data=(InterfaceNode*)(node->data);
  if(validate){
    if(strlen(data->parent)>0){
      ERROR(!interface_exists(data->parent),"parent interface %s does not exist",data->parent);
      ERROR(!add_child_type(data->name,data->parent),"co-dependent interface %s detected",data->name);
    }
    ERROR(type_exists(data->name),"type %s is already declared",data->name);
    register_type(data->name);
    register_interface(data);
  }
}
void process_class(AstNode* node){
  ClassNode* data=(ClassNode*)(node->data);
  if(validate){
    if(strlen(data->parent)>0){
      ERROR(!class_exists(data->parent),"parent class %s does not exist",data->parent);
      ERROR(!add_child_type(data->name,data->parent),"co-dependent class %s detected",data->name);
    }
    for(int a=0;a<data->interfaces->n;a++){
      char* interface=(char*)get_from_list(data->interfaces,a);
      ERROR(!interface_exists(data->parent),"interface %s does not exist",interface);
      add_child_type(data->name,interface);
    }
    ERROR(type_exists(data->name),"type %s is already declared",data->name);
    register_type(data->name);
    register_class(data);
  }
  push_scope();
  for(int a=0;a<data->ls->n;a++){
    process_node((AstNode*)get_from_list(data->ls,a));
  }
  pop_scope();
}

// Statement group
void process_stmt(AstNode* node){
  List* ls=(List*)(node->data);
  for(int a=0;a<ls->n;a++){
    AstNode* e=(AstNode*)get_from_list(ls,a);
    process_node(e);
    if(e->type==AST_CALL) write("\n");
    if(e->type==AST_FUNCTION) write("\n");
  }
}
void process_do(AstNode* node){
  List* ls=(List*)(node->data);
  write("do\n");
  push_scope();
  for(int a=0;a<ls->n;a++){
    process_node((AstNode*)get_from_list(ls,a));
  }
  write("end\n");
  pop_scope();
}

// Statement
void process_call(AstNode* node){
  AstAstNode* data=(AstAstNode*)(node->data);
  if(validate){
    char* name=NULL;
    AstNode* functype=NULL;
    if(data->l->type==AST_ID){
      name=(char*)(data->l->data);
      FunctionNode* func=function_exists(name);
      if(func){
        AstNode* funcnode=new_node(AST_FUNCTION,func);
        functype=get_type(funcnode);
        free(funcnode);
      }
    }else if(data->l->type==AST_FIELD){
      name=((StringAstNode*)(data->l->data))->text;
      functype=get_type(data->l);
    }
    if(functype){
      List* funcargs=((AstListNode*)(functype->data))->list;
      if(data->r){
        List* args=(List*)(data->r->data);
        ERROR(funcargs->n!=args->n,"invalid number of arguments for function %s",name);
        for(int a=0;a<args->n;a++){
          AstNode* type1=get_type((AstNode*)get_from_list(args,a));
          AstNode* type2=(AstNode*)get_from_list(funcargs,a);
          ERROR(!typed_match(type1,type2),"invalid argument provided for function %s",name);
        }
      }else{
        ERROR(funcargs->n!=0,"not enough arguments for function %s",name);
      }
    }
  }
  process_node(data->l);
  write("(");
  if(data->r) process_node(data->r);
  write(")");
}
void process_set(AstNode* node){
  AstAstNode* data=(AstAstNode*)(node->data);
  if(validate){
    AstNode* tl=get_type(data->l);
    AstNode* tr=get_type(data->r);
    if(tr->type==AST_TYPE_TUPLE){
      List* ls=(List*)(tr->data);
      if(ls->n==1) tr=(AstNode*)get_from_list(ls,0);
    }
    ERROR(!typed_match(tl,tr),"expression of type %t cannot be assigned to variable of type %t",tr,tl);
  }
  process_node(data->l);
  write("=");
  process_node(data->r);
  write("\n");
}
void process_return(AstNode* node){
  write("return");
  if(node->data){
    write(" ");
    process_node((AstNode*)(node->data));
  }
  write("\n");
}
void process_ltuple(AstNode* node){
  List* ls=(List*)(node->data);
  for(int a=0;a<ls->n;a++){
    if(a) write(",");
    write("%s",(char*)get_from_list(ls,a));
  }
}
void process_field(AstNode* node){
  StringAstNode* data=(StringAstNode*)(node->data);
  process_node(data->node);
  write(".%s",data->text);
}
void process_sub(AstNode* node){
  AstAstNode* data=(AstAstNode*)(node->data);
  process_node(data->l);
  write("[");
  process_node(data->r);
  write("]");
}
void process_id(AstNode* node){
  write("%s",(char*)(node->data));
}
void process_local(AstNode* node){
  StringAstNode* data=(StringAstNode*)(node->data);
  write("local %s",data->text);
  if(data->node){
    write("=");
    process_node(data->node);
  }
  write("\n");
}

// Control
void process_function(AstNode* node){
  // TODO implement return statement type check
  FunctionNode* data=(FunctionNode*)(node->data);
  write("function");
  if(data->name[0]){
    register_function(data);
    write(" %s",data->name);
  }
  write("(");
  for(int a=0;a<data->args->n;a++){
    if(a) write(",");
    StringAstNode* arg=(StringAstNode*)get_from_list(data->args,a);
    write("%s",arg->text);
  }
  write(")\n");
  if(data->body){
    push_scope();
    for(int a=0;a<data->body->n;a++){
      process_node((AstNode*)get_from_list(data->body,a));
    }
    write("end");
    pop_scope();
  }
}
void process_repeat(AstNode* node){
  AstListNode* data=(AstListNode*)(node->data);
  write("repeat\n");
  push_scope();
  for(int a=0;a<data->list->n;a++){
    process_node((AstNode*)get_from_list(data->list,a));
  }
  pop_scope();
  write("until ");
  process_node(data->node);
  write("\n");
}
void process_while(AstNode* node){
  AstListNode* data=(AstListNode*)(node->data);
  write("while ");
  process_node(data->node);
  write(" do\n");
  push_scope();
  for(int a=0;a<data->list->n;a++){
    process_node((AstNode*)get_from_list(data->list,a));
  }
  pop_scope();
  write("end\n");
}
void process_if(AstNode* node){
  AstListNode* data=(AstListNode*)(node->data);
  write("if ");
  process_node(data->node);
  write(" then\n");
  push_scope();
  for(int a=0;a<data->list->n;a++){
    process_node((AstNode*)get_from_list(data->list,a));
  }
  pop_scope();
  write("end\n");
}
void process_fornum(AstNode* node){
  FornumNode* data=(FornumNode*)(node->data);
  write("for %s=",data->name);
  process_node(data->num1);
  write(",");
  process_node(data->num2);
  if(data->num3){
    write(",");
    process_node(data->num3);
  }
  push_scope();
  write(" do\n");
  for(int a=0;a<data->body->n;a++){
    process_node((AstNode*)get_from_list(data->body,a));
  }
  write("end\n");
  pop_scope();
}
void process_forin(AstNode* node){
  ForinNode* data=(ForinNode*)(node->data);
  write("for ");
  process_node(data->lhs);
  write(" in ");
  process_node(data->tuple);
  write(" do\n");
  push_scope();
  for(int a=0;a<data->body->n;a++){
    process_node((AstNode*)get_from_list(data->body,a));
  }
  write("end\n");
  pop_scope();
}
void process_break(AstNode* node){
  write("break\n");
}
void process_label(AstNode* node){
  write("::%s::\n",(char*)(node->data));
}
void process_goto(AstNode* node){
  write("goto %s\n",(char*)(node->data));
}

// Primitives
void process_primitive(AstNode* node){
  write("%s",((StringAstNode*)(node->data))->text);
}
void process_table(AstNode* node){
  TableNode* data=(TableNode*)(node->data);
  write("{");
  for(int a=0;a<data->keys->n;a++){
    if(a) write(",");
    write("%s=",(char*)get_from_list(data->keys,a));
    process_node((AstNode*)get_from_list(data->vals,a));
  }
  write("}");
}
void process_tuple(AstNode* node){
  List* ls=(List*)(node->data);
  for(int a=0;a<ls->n;a++){
    if(a) write(",");
    process_node((AstNode*)get_from_list(ls,a));
  }
}

// Expressions
void process_unary(AstNode* node){
  // TODO test and then remove parenthesis
  BinaryNode* data=(BinaryNode*)(node->data);
  write("(%s ",data->text);
  process_node(data->l);
  write(")");
}
void process_binary(AstNode* node){
  // TODO test and then remove parenthesis
  BinaryNode* data=(BinaryNode*)(node->data);
  write("(");
  process_node(data->l);
  write(" %s ",data->text);
  process_node(data->r);
  write(")");
}
void process_paren(AstNode* node){
  write("(");
  process_node((AstNode*)(node->data));
  write(")");
}
