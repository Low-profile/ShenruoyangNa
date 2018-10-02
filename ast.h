#include <string>
#include <memory>
#include <vector>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>
using namespace llvm;



class AST_node
{
public:
  std::string tok;
  std::vector<AST_node *> children;
  AST_node(std::string token) : tok(token) {}
  void appendChild(AST_node *child)
  {
    children.push_back(child);
  }
};


/// ExprAST - Base class for all expression nodes.
class ExprAST
{
public:
  virtual ~ExprAST() = default;

  virtual Value *codegen() = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST
{
  int Val;

public:
  NumberExprAST(int Val) : Val(Val) {}

  Value *codegen() override;
};

/// BoolExprAST - Expression class for numeric literals like "1.0".
class BoolExprAST : public ExprAST
{
  double Val;
public:
  BoolExprAST(double Val) : Val(Val) {}

  Value *codegen() override;
};

class IdentExprAST : public ExprAST
{
  std::string Ident;
public:
  IdentExprAST(std::string Ident) : Ident(Ident) {}

  Value *codegen() override;
};


/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST
{
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}

  Value *codegen() override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST
{
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  Value *codegen() override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST
{
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}

  Value *codegen() override;
};

/// StmtAST
class StmtAST
{

  // public:
  

  // virtual Value *codegen() = 0;

public:
  virtual Value * codegen() = 0;
};

/// ReturnStmtAST
class ReturnStmtAST : public StmtAST
{
  std::unique_ptr<ExprAST> Expr;
public:
  ReturnStmtAST(){}
  ReturnStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}

  Value * codegen() override;
};

/// ReturnStmtAST
class ExprStmtAST : public StmtAST
{
  std::unique_ptr<ExprAST> Expr;
public:
  ExprStmtAST(){}
  ExprStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}

  Value * codegen() override;
};

/// ReturnStmtAST
class AssignmentStmtAST : public StmtAST
{
  std::string varname;
  std::unique_ptr<ExprAST> Expr;
public:
  AssignmentStmtAST(){}
  AssignmentStmtAST(std::string varname,std::unique_ptr<ExprAST> Expr)
      : varname(varname), Expr(std::move(Expr)) {}

  Value * codegen() override;
};

/// BoolExprAST - Expression class for numeric literals like "1.0".
class DeclStmtAST : public StmtAST
{
  std::vector<std::string> idents;
public:
  DeclStmtAST(std::vector<std::string> Idents) : idents(std::move(Idents)) {}

  Value *codegen() override;
};


/// StmtblockAST
class StmtblockAST
{
  std::vector<std::unique_ptr<StmtAST>> Stmts;

public:
  StmtblockAST(
      std::vector<std::unique_ptr<StmtAST>> Stmts)
      : Stmts(std::move(Stmts)) {}

  Value * codegen();
};

/// IfExprAST - Expression class for if/then/else.
class IfStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtblockAST> Then, Else;

public:
  IfStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtblockAST> Then,
            std::unique_ptr<StmtblockAST> Else)
    : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  Value *codegen() override;
};

/// ForExprAST - Expression class for for/in.
class ForStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtblockAST> Body;

public:
  ForStmtAST(std::unique_ptr<ExprAST> Cond,
             std::unique_ptr<StmtblockAST> Body)
    :Cond(std::move(Cond)), Body(std::move(Body)) {}

  Value *codegen() override;
};



/// FunctionAST - This class represents a function definition itself.
class FunctionAST
{
  std::string Name;
  std::vector<std::tuple<std::string,std::string>> Args;
  std::unique_ptr<StmtblockAST> Body;

public:
  FunctionAST(const std::string &Name, std::vector<std::tuple<std::string,std::string>> Args, std::unique_ptr<StmtblockAST> Body)
      : Name(Name), Args(std::move(Args)), Body(std::move(Body)) {}

  Function *codegen();

  const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class ProgramAST
{
  std::vector<std::unique_ptr<FunctionAST>> Funcs;

public:
  ProgramAST(std::vector<std::unique_ptr<FunctionAST>> Funcs)
      : Funcs(std::move(Funcs)) {}

  void codegen();
};