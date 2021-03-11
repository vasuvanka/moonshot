#include "./internal.h"
#include <stdlib.h>
#include <string.h>

// Interface and Class function checks
static List* get_interface_ancestor_methods(AstNode* node){
  char* name;
  List* ls=new_default_list();
  if(node->type==AST_CLASS){
    ClassNode* c=(ClassNode*)(node->data);
    AstNode* inode=new_node(AST_INTERFACE,NULL);
    while(c){
      for(int a=0;a<c->interfaces->n;a++){
        name=(char*)get_from_list(c->interfaces,a);
        inode->data=interface_exists(name);
        if(inode->data){
          List* ls1=get_interface_ancestor_methods(inode);
          append_all(ls,ls1);
          dealloc_list(ls1);
        }
      }
      c=class_exists(c->parent);
    }
    free(inode);
  }else if(node->type==AST_INTERFACE){
    InterfaceNode* i=(InterfaceNode*)(node->data);
    while(i){
      for(int a=0;a<i->ls->n;a++) add_to_list(ls,((AstNode*)get_from_list(i->ls,a))->data);
      i=interface_exists(i->parent);
    }
  }
  return ls;
}
static List* get_class_ancestor_methods(ClassNode* node){
  List* ls=new_default_list();
  while(node){
    for(int a=0;a<node->ls->n;a++){
      AstNode* e=(AstNode*)get_from_list(node->ls,a);
      if(e->type==AST_FUNCTION) add_to_list(ls,e->data);
    }
    node=class_exists(node->parent);
  }
  return ls;
}
List* get_missing_class_methods(ClassNode* c){
  AstNode* node=new_node(AST_CLASS,c);
  List* missing=get_interface_ancestor_methods(node);
  List* found=get_class_ancestor_methods(c);
  free(node);
  int a=0;
  while(a<missing->n){
    int removed=0;
    for(int b=0;b<found->n;b++){
      FunctionNode* f1=(FunctionNode*)get_from_list(missing,a);
      FunctionNode* f2=(FunctionNode*)get_from_list(found,b);
      if(methods_equivalent(f1,f2)){
        remove_from_list(missing,a);
        removed=1;
        break;
      }
    }
    if(!removed) a++;
  }
  dealloc_list(found);
  return missing;
}
int methods_equivalent(FunctionNode* f1,FunctionNode* f2){
  // Assumes the two methods belong to clases (name nodes are of type AST_ID)
  AstNode* node=new_node(AST_FUNCTION,f1);
  AstNode* type1=get_type(node);
  free(node);
  node=new_node(AST_FUNCTION,f2);
  AstNode* type2=get_type(node);
  free(node);
  return !strcmp((char*)(f1->name->data),(char*)(f2->name->data)) && typed_match(type1,type2);
}
