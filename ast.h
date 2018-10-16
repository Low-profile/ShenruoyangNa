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
  std::string getName(){return "";}
  virtual Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) = 0;
};

class printArg_t{
public:
  std::string str;
  std::unique_ptr<ExprAST> expr;

  printArg_t()
  {
    str = "";
    expr = nullptr;
  }
  printArg_t(std::string str, std::unique_ptr<ExprAST> expr):
    str(str),expr(std::move(expr))
  {
  }

  bool isEmpty()
  {
    return (str == "") && (expr == nullptr);
  }
};


/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST
{
  int Val;

public:
  NumberExprAST(int Val) : Val(Val) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// BoolExprAST - Expression class for numeric literals like "1.0".
class BoolExprAST : public ExprAST
{
  bool Val;
public:
  BoolExprAST(bool Val) : Val(Val) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

class inputExprAST : public ExprAST
{
public:
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

class SizeofExprAST : public ExprAST
{
  std::string ident;
public:
  SizeofExprAST(std::string ident) : ident(ident) {}
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};


/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST
{
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
  std::string getName()
  {
    return Name;
  }
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class ArrExprAST : public ExprAST
{
  std::string Name;
  std::unique_ptr<ExprAST> index;

public:
  ArrExprAST(const std::string &Name, std::unique_ptr<ExprAST> index) 
  : Name(Name),index(move(index)) {}
  std::string getName()
  {
    return Name;
  }
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST
{
  std::string Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(std::string Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// BinaryExprAST - Expression class for a binary operator.
class UnaryExprAST : public ExprAST
{
  std::string Op;
  std::unique_ptr<ExprAST> expr;

public:
  UnaryExprAST(std::string Op, std::unique_ptr<ExprAST> expr)
      : Op(Op), expr(std::move(expr)) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
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

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};



/// StmtAST
class StmtAST
{

  // public:
  

  // virtual Value *codegen() = 0;

public:
  virtual Value * codegen(std::map<std::string, AllocaInst *>& NamedValues) = 0;
};

/// ReturnStmtAST
class ReturnStmtAST : public StmtAST
{
  std::unique_ptr<ExprAST> Expr;
public:
  ReturnStmtAST(){}
  ReturnStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}

  Value * codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// ReturnStmtAST
class ExprStmtAST : public StmtAST
{
  std::unique_ptr<ExprAST> Expr;
public:
  ExprStmtAST(){}
  ExprStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}

  Value * codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
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

  Value * codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// ReturnStmtAST
class ArrAssignmentStmtAST : public StmtAST
{
  std::string varname;
  std::unique_ptr<ExprAST> Expr,arridx;
public:
  ArrAssignmentStmtAST(std::string varname,std::unique_ptr<ExprAST> Expr, std::unique_ptr<ExprAST> arridx)
      : varname(varname), Expr(std::move(Expr)),arridx(std::move(arridx)) {}

  Value * codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// BoolExprAST - Expression class for numeric literals like "1.0".
class DeclStmtAST : public StmtAST
{
  std::string type;
  std::vector<std::string> idents;
public:
  DeclStmtAST(std::string type, std::vector<std::string> Idents) : 
    type(type), idents(std::move(Idents)) {}

  void printIdents()
  {
    for(auto s : idents)
    {
      std::cout << s << std::endl;
    }
  }
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};


/// BoolExprAST - Expression class for numeric literals like "1.0".
class ArrDeclStmtAST : public StmtAST
{
  std::vector<std::string> idents;
  std::vector<std::unique_ptr<ExprAST>> arr_size;
public:
  ArrDeclStmtAST(std::vector<std::string> Idents, std::vector<std::unique_ptr<ExprAST>> size) : 
   idents(std::move(Idents)),arr_size(std::move(size)) {}

  void printIdents()
  {
    for(auto s : idents)
    {
      std::cout << s << std::endl;
    }
  }
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};


/// StmtblockAST
class StmtblockAST : public StmtAST
{
  std::vector<std::unique_ptr<StmtAST>> Stmts;

public:
  StmtblockAST(
      std::vector<std::unique_ptr<StmtAST>> Stmts)
      : Stmts(std::move(Stmts)) {}
  
  void appendStmt(std::unique_ptr<StmtAST> stmt)
  {
    Stmts.push_back(move(stmt));
  }

  Value * codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// IfExprAST - Expression class for if/then/else.
class IfStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtblockAST> Then, Else;
public:
  IfStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtblockAST> Then,
            std::unique_ptr<StmtblockAST> Else)
    : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

/// ForExprAST - Expression class for for/in.
class ForStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtblockAST> Body;

public:
  ForStmtAST(std::unique_ptr<ExprAST> Cond,
             std::unique_ptr<StmtblockAST> Body)
    :Cond(std::move(Cond)), Body(std::move(Body)) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

class forarrStmtAST : public StmtAST {
  std::string iterator;
  std::unique_ptr<ExprAST> array;
  std::unique_ptr<StmtblockAST> Body;

public:
  forarrStmtAST(std::string iterator, std::unique_ptr<ExprAST> array,
             std::unique_ptr<StmtblockAST> Body)
    :iterator(iterator), array(std::move(array)), Body(std::move(Body)) {}
  std::string getarrName()
  {
    return array->getName();
  }
  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

class printStmtAST : public StmtAST
{
  std::vector<std::unique_ptr<printArg_t>> args;
public:
  printStmtAST(std::vector<std::unique_ptr<printArg_t>> args) : args(std::move(args)) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};

class abortStmtAST : public StmtAST
{
  std::vector<std::unique_ptr<printArg_t>> args;
public:
  abortStmtAST(std::vector<std::unique_ptr<printArg_t>> args) : args(std::move(args)) {}

  Value *codegen(std::map<std::string, AllocaInst *>& NamedValues) override;
};


/// FunctionAST - This class represents a function definition itself.
class FunctionAST
{
  std::string Name,type;
  std::vector<std::tuple<std::string,std::string>> Args;
  std::unique_ptr<StmtblockAST> Body;

public:
  FunctionAST(const std::string &type, const std::string &Name, std::vector<std::tuple<std::string,std::string>> Args, std::unique_ptr<StmtblockAST> Body)
      : type(type), Name(Name), Args(std::move(Args)), Body(std::move(Body)) {}

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

