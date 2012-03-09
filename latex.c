#include "latex.h"

#include <stdio.h>

static u8 depth = 2;
static void indent()
{
  for (u8 i = 0; i < depth; i++)
    putchar('\t');
}
static void beginNode(const char* name)
{
  indent();printf("child\n");
  indent();printf("{\n");
  depth++;
  indent();printf("node{%s}\n", name);
}
static void beginNodeInt(s32 s)
{
  indent();printf("child\n");
  indent();printf("{\n");
  depth++;
  indent();printf("node{%ld}\n", s);
}
static void endNode()
{
  indent();printf("edge from parent node[below]{}\n");
  depth--;
  indent();printf("}\n");
}

static void printName(const char* str);
static void printExprList(ExprList* l);
static void printExpr(Expr* e);
static void printType(Type* t);
static void printStmtList(StmtList* l);
static void printStmt(Stmt* s);
static void printParam(Param* p);
static void printParams(ParamList* l);
static void printFunDecl(FunDecl* f);

static void printName(const char* str)
{
  beginNode(str);
  endNode();
}

static void printExprList(ExprList* l)
{
  if(!l){printName("NULL");return;}
  do
  {
    printExpr(l->head);
  }
  while ((l = l->tail));
}

#define PRINT_BINOP(CODE, NAME) \
    case EXPR_##CODE: \
      beginNode(#NAME); \
      printExpr(e->v.bin_op.left); \
      printExpr(e->v.bin_op.right); \
      endNode(); \
      break;
static void printExpr(Expr* e)
{
  if(!e){printName("NULL");return;}
  switch(e->type)
  {
    case EXPR_INTEGER:
      beginNodeInt(e->v.i);
      endNode();
      break;
    case EXPR_FUN_CALL:
      beginNode("FunCall");
      printName(e->v.call.name);
      printExprList(e->v.call.params);
      endNode();
      break;
    case EXPR_AFF:
      beginNode("Aff");
      printName(e->v.aff.name);
      printExpr(e->v.aff.expr);
      endNode();
      break;
    case EXPR_VAR:
      beginNode("Var");
      printName(e->v.var.name);
      endNode();
      break;
    PRINT_BINOP(EQ,  Eq );
    PRINT_BINOP(NEQ, Neq);
    PRINT_BINOP(LE,  Le );
    PRINT_BINOP(LT,  Lt );
    PRINT_BINOP(GE,  Ge );
    PRINT_BINOP(GT,  Gt );
    PRINT_BINOP(ADD, Add);
    PRINT_BINOP(SUB, Sub);
    PRINT_BINOP(MUL, Mul);
    PRINT_BINOP(DIV, Div);
    PRINT_BINOP(MOD, Mod);
    case EXPR_MINUS:
      beginNode("Minus");
      printExpr(e->v.uni_op);
      endNode();
      break;
    case EXPR_IFTE:
      beginNode("Ifte");
      printExpr(e->v.tern_op.op1);
      printExpr(e->v.tern_op.op2);
      printExpr(e->v.tern_op.op3);
      endNode();
    default:
      break;
  }
}

static void printType(Type* t)
{
  if(!t){printName("NULL");return;}
  switch (t->type)
  {
    case TYPE_VOID:
      printName("void");
      break;
    case TYPE_CHAR:
      printName("char");
      break;
    case TYPE_INT:
      printName("int");
      break;
    case TYPE_PTR:
      printType(t->v.ptr);
      printName("*");
      break;
    default:
      break;
  }
}

static void printStmtList(StmtList* l)
{
  do
  {
    printStmt(l->head);
  }
  while ((l = l->tail));
}

static void printStmt(Stmt* s)
{
  switch (s->type)
  {
    case STMT_NOTHING:
      beginNode("Nothing");
      endNode();
      break;
    case STMT_DECL:
      beginNode("Decl");
      printType(s->v.decl.t);
      printName(s->v.decl.name);
      printExpr(s->v.decl.val);
      endNode();
      break;
    case STMT_EXPR:
      printExpr(s->v.expr);
      break;
    case STMT_WHILE:
      beginNode("While");
      printExpr(s->v.whilez.cond);
      printStmt(s->v.whilez.stmt);
      endNode();
      break;
    case STMT_DO:
      beginNode("Do");
      printStmt(s->v.doz.stmt);
      printExpr(s->v.doz.cond);
      endNode();
    case STMT_FOR:
      beginNode("For");
      printStmt(s->v.forz.a);
      printStmt(s->v.forz.b);
      printStmt(s->v.forz.c);
      printStmt(s->v.forz.stmt);
      endNode();
      break;
    case STMT_IF:
      beginNode("If");
      printExpr(s->v.ifz.cond);
      printStmt(s->v.ifz.iftrue);
      if (s->v.ifz.iffalse)
        printStmt(s->v.ifz.iffalse);
      endNode();
      break;
    case STMT_RETURN:
      beginNode("Return");
      printExpr(s->v.expr);
      endNode();
      break;
    case STMT_BLOCK:
      beginNode("Block");
      printStmtList(s->v.block);
      endNode();
      break;

    default:
      break;
  }
}

static void printParam(Param* p)
{
  if(!p){printName("NULL");return;}
  beginNode("Param");
  printType(p->type);
  printName(p->name);
  endNode();
}

static void printParams(ParamList* l)
{
  if(!l){printName("NULL");return;}
  beginNode("ParamList");
  do
  {
    printParam(l->head);
  }
  while ((l = l->tail));
  endNode();
}

static void printFunDecl(FunDecl* f)
{
  if(!f){printName("NULL");return;}
  beginNode("FunDecl");
  printType(f->type);
  printName(f->name);
  printParams(f->params);
  printStmt(f->stmt);
  endNode();
}

void Program_Latex(Program* p)
{
  if (!p)return;
  
  printf
  (
    "\\documentclass{article}\n"
    "\\usepackage{tikz}\n"
    "\\usetikzlibrary{trees}\n"
    "\\begin{document}\n"
    "\t\\tikzstyle{level 1}=[level distance=3.5cm, sibling distance=2cm]\n"
    "\t\\tikzstyle{level 2}=[level distance=3.5cm, sibling distance=2cm]\n"
    "\t\\begin{tikzpicture}\n"
  );
  
  printf("\t\t\\node{Program}\n");
  
  do
  {
    printFunDecl(p->head);
  }
  while ((p = p->tail));
  
  printf("\t\t;\n");
  printf("\t\\end{tikzpicture}\n");
  printf("\\end{document}\n");
}
