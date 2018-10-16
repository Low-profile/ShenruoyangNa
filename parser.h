#include "ast.h"
#include "scanner.h"

#include <vector>
#include <string>


struct token_t
{
    int lineno;
    std::string literal;
    token type;
};


class Parser
{
  private:
    AST_node root;
    std::vector<token_t> tokens;
    std::vector<token_t>::size_type tok_size;

    std::vector<token_t> scanner(std::string path);

    // newlines := newline _newlines
    //              |newline
    bool Newlines(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);
    
    // program :=
    //      <function \n>+ (NOTE: angular braces are used for grouping)
    std::unique_ptr<ProgramAST> Prog(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);

    // function :=
    //      type ident(<, type ident>*) stmtblock
    std::unique_ptr<FunctionAST> Func(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // type :=
    //   bool
    //   int
    //   array
    //   void
    bool Type(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node, std::string& node_n);

    // ident :=
    //  [a-zA-Z_][a-zA-Z0-9_]*
    bool Ident(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node, std::string& node_n);


    //string
    bool StringConstant(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node, std::string& name);


    // Args : = <, type ident>*
    bool Args(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node, std::vector<std::tuple<std::string,std::string>>& node_n);


    // _stmts := stmt newlines
    //      stmt newlines _stmts
    std::vector<std::unique_ptr<StmtAST>> Stmts(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // idents :=  ident
    //          ident , idents
    std::vector<std::string> Idents(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // arrayidents := ident\[expr\]
    //              ident\[expr\] , arrayidents
    
    std::vector<std::unique_ptr<ExprAST>> ArrayIdents(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node, std::vector<std::string>& idents_n);


    // printArg = string|expr
    std::unique_ptr<printArg_t> PrintArg(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // printArgs :=  printArg
    //              printArg, printArgs
    std::vector<std::unique_ptr<printArg_t>> printArgs(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    //  stmtblock :=
    //       {\n stmts }
    std::unique_ptr<StmtblockAST> Stmtblock(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    //  stmt :=
    //   stmtblock
    //   type ident <, ident>*
    //   array ident\[expr\] <, ident\[expr\]>*
    //   print(<string|expr> <, <string|expr> >*
    //   if (expr) stmtblock [else stmtblock]
    //   while (expr) stmtblock
    //   for (ident : expr) stmtblock
    //  ident := expr
    //  ident\[expr\] := expr
    //  expr
    //  ;.*
    //  ;[^\n]*
    //  return [expr]
    std::unique_ptr<StmtAST> Stmt(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // expr :=
    //       true
    //       false
    //       -?[0-9]+
    //       ident
    //       (expr ? expr : expr)
    //      sizeof(ident)
    //      input()
    //      ident\[expr\]
    //      ident(<, expr>*)
    //      (expr binop expr)
    //      (unaryop expr)
    std::unique_ptr<ExprAST> Expr(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // Exprs : = empty
    //         | _Exprs
    std::vector<std::unique_ptr<ExprAST>> Exprs(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // _Exprs : = expr
    //         | expr, _Exprs
    //bool _Exprs(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node);


    // binop :=
    //  + - * ^ / % & | == != > >= < <=
    bool Binop(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node,std::string& name);

    // unaryop :=
    //     - !
    //     (bang is Boolean not)
    bool Unaryop(std::vector<token_t>::size_type idx, std::vector<token_t>::size_type &ret_idx, AST_node *node,std::string& name);


  public:
    Parser(std::string path);
    
    bool parse();

    void printTree();
};