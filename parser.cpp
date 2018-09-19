#include <string>
#include <iostream>
#include "scanner.h"
#include <vector>
#include <queue>
#include <tuple>
#include <iomanip>

using namespace std;

extern int yylineno;
extern char *yytext;
void yyrestart(FILE *fp);
int yylex();

struct token_t
{
    int lineno;
    string literal;
    token type;
};

class AST_node
{
  public:
    string tok;
    vector<AST_node *> children;
    AST_node(string token) : tok(token) {}
    void appendChild(AST_node *child)
    {
        children.push_back(child);
    }
};

class Parser
{
  private:
    AST_node root;
    vector<token_t> tokens;
    vector<token_t>::size_type tok_size;

    std::vector<token_t> scanner(string path)
    {
        std::cout << "Parsing file: " << path << std::endl;

        FILE *fp = nullptr;
        fp = fopen(path.c_str(), "r");

        yyrestart(fp);

        std::vector<token_t> tokens;
        int tok = -1;

        while ((tok = yylex()))
        {
            token_t _tok = {yylineno = yylineno + 1, string(yytext), static_cast<token>(tok)};
            tokens.push_back(_tok);
        }
        token_t eof = {yylineno, "EOF", T_EOF};
        tokens.push_back(eof);

        return tokens;
    }

    // newlines := newline _newlines
    //              |newline
    bool Newlines(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_newlines = new AST_node("newline");

        if (idx < tok_size)
        {
            if (tokens[idx].type == T_NewLine)
            {
                if(idx + 1 < tok_size 
                && Newlines(idx + 1, ret_idx, n_newlines))
                {
                    return true;
                }
                else
                {
                    ret_idx = idx + 1;
                    return true;
                }
            }
            else
            {
                ret_idx = idx;
                return false;
            }
                
        }

        else
        {
            ret_idx = idx;
            return false;
        }
    }

    // program :=
    //      <function \n>+ (NOTE: angular braces are used for grouping)
    // program :=  function newlines program
    //         | function
    bool Prog(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_func = new AST_node("func");
        auto n_newlines = new AST_node("n_newlines");
        auto n_prog = new AST_node("n_prog");

        if (idx < tok_size)
        {
            if (Func(idx, ret_idx, n_func))
            {
                if(Newlines(ret_idx, ret_idx, n_newlines)
                && Prog(ret_idx, ret_idx, n_prog))
                {
                    node->appendChild(n_func);
                    node->appendChild(n_newlines);
                    node->appendChild(n_prog);
                    return true;
                }
                else
                {
                    node->appendChild(n_func);
                    return true;
                }
            } 
            else
            {
                ret_idx = idx;
                return false;
            }
        }
        else
        {
            ret_idx = idx;
            return false;
        }


        // vector<token_t>::size_type ret_idx = 0;
        // auto n_func = new AST_node("func");
        // while (Func(idx, ret_idx, n_func) && ret_idx < tok_size)
        // {
        //     root.appendChild(n_func);
        //     if (tokens[ret_idx].type == T_EOF)
        //         return true;
        //     if (tokens[ret_idx].type != T_NewLine)
        //         return false;

        //     idx = ret_idx + 1;
        // }
        //return true;
    }

    // function :=
    //      type ident(<, type ident>*) stmtblock
    bool Func(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_type = new AST_node("type");
        auto n_ident = new AST_node("ident");
        auto n_lbracket = new AST_node("(");
        auto n_args = new AST_node("args");
        auto n_rbracket = new AST_node(")");
        auto n_stmtblock = new AST_node("stmtblock");

        if (idx < tok_size && Type(idx, ret_idx, n_type) 
        && Ident(ret_idx, ret_idx, n_ident) 
        && tokens[ret_idx].literal == "(" && ret_idx + 1 < tok_size 
        && Args(ret_idx + 1, ret_idx, n_args) 
        && tokens[ret_idx].literal == ")" && ret_idx + 1 < tok_size 
        && Stmtblock(ret_idx + 1, ret_idx, n_stmtblock))
        {
            cout << "Func matched!" << endl;
            node->appendChild(n_type);
            node->appendChild(n_ident);
            node->appendChild(n_lbracket);
            node->appendChild(n_args);
            node->appendChild(n_rbracket);
            node->appendChild(n_stmtblock);
            return true;
        }
        else
        {
            ret_idx = idx;
            return false;
        }
    }

    // type :=
    //   bool
    //   int
    //   array
    //   void
    bool Type(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        if (idx < tok_size &&
                tokens[idx].type == T_Int ||
            tokens[idx].type == T_Void || tokens[idx].type == T_Bool || tokens[idx].type == T_Array)
        {
            ret_idx = idx + 1;
            return true;
        }
        else
            return false;
    }

    // ident :=
    //  [a-zA-Z_][a-zA-Z0-9_]*
    bool Ident(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        if (idx < tok_size)
        {
            if (tokens[idx].type == T_Identifier)
            {
                ret_idx = idx + 1;
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }

    // Args : = empty
    //         | _Args
    bool Args(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_pargs = new AST_node("_args");
        if (idx < tok_size && _Args(idx, ret_idx, node))
        {

            //node->appendChild(n_pargs);
            return true;
        }
        else
        {
            ret_idx = idx;
            return true;
        }
    }

    // _Args : = type ident
    //         | type ident, _Args
    bool _Args(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {

        auto n_type = new AST_node("type");
        auto n_comma = new AST_node(",");
        auto n_ident = new AST_node("ident");
        auto n_pargs = new AST_node("_args");

        if (idx < tok_size)
        {
            if (Type(idx, ret_idx, n_type) && Ident(ret_idx, ret_idx, n_ident))
            {
                node->appendChild(n_type);
                node->appendChild(n_ident);
                if(tokens[ret_idx].literal == "," && ret_idx + 1 < tok_size 
                && _Args(ret_idx + 1, ret_idx, n_pargs))
                {
                    node->appendChild(n_comma);
                    node->appendChild(n_pargs);
                }
                return true;
            }
            else
            {
                ret_idx = idx;
                return false;
            }
        }
        else
        {
            ret_idx = idx;
            return false;
        }
    }

    // stmts := empty
    //      _stmts
    bool Stmts(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_stmts = new AST_node("_stmts");
        if (idx < tok_size && _Stmts(idx, ret_idx, node))
        {
            //node->appendChild(n_stmts);
            return true;
        }
        else
        {
            ret_idx = idx;
            return true;
        }
    }
    
    // _stmts := stmt newlines
    //      stmt newlines _stmts 
    bool _Stmts(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {

        auto n_stmt = new AST_node("stmt");
        auto n_newlines= new AST_node("newlines");
        auto n_ptmts = new AST_node("_stmts");

        if (idx < tok_size)
        {
            if (Stmt(idx, ret_idx, n_stmt) && Newlines(ret_idx, ret_idx, n_newlines))
            {
                node->appendChild(n_stmt);
                node->appendChild(n_newlines);
                _Stmts(ret_idx, ret_idx, node);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            ret_idx = idx;
            return false;
        }
    }
    
    //  stmtblock :=
    //       {\n <stmt \n>* }
    //  stmtblock :=
    //       {\n stmts }
    
    bool Stmtblock(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_lcbracket = new AST_node("{");
        auto n_newlines = new AST_node("newlines");
        auto n_stmts = new AST_node("stmts");
        auto n_rcbrakcet = new AST_node("}");
        if (idx < tok_size)
        {
            if (tokens[idx].literal == "{" && idx + 1 < tok_size
            && Newlines(idx+1,ret_idx,n_newlines)
            && Stmts(ret_idx,ret_idx,n_newlines)
            && tokens[ret_idx].literal == "}")
            {
                node->appendChild(n_lcbracket);
                node->appendChild(n_newlines);
                node->appendChild(n_stmts);
                node->appendChild(n_rcbrakcet);
                ret_idx = ret_idx + 1;
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }

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
    bool Stmt(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        auto n_type = new AST_node("type");
        auto n_comma = new AST_node(",");
        auto n_ident = new AST_node("ident");
        auto n_pargs = new AST_node("_args");
        auto n_expr = new AST_node("expr");

        if (idx < tok_size)
        {
            
            if (Stmtblock(idx, ret_idx, n_type)) //   stmtblock
            {
                return true;
            }
            // else if (Type(idx, ret_idx, n_type)) //   type ident <, ident>*
            // {

            // }
            // else if () //   array ident\[expr\] <, ident\[expr\]>*
            // {
            //     return true;
            // }
            // else if () //   print(<string|expr> <, <string|expr> >*
            // {
            //     return true;
            // }
            // else if () //   if (expr) stmtblock [else stmtblock]
            // {
            //     return true;
            // }
            // else if () //   while (expr) stmtblock
            // {
            //     return true;
            // }
            // else if () //   for (ident : expr) stmtblock
            // {
            //     return true;
            // }
            // else if () //   ident := expr
            // {
            //     return true;
            // }
            // else if () //   ident\[expr\] := expr
            // {
            //     return true;
            // }
            else if (Expr(idx,ret_idx,n_expr)) //   expr
            {
                node->appendChild(n_expr);

                return true;
            }
            // // else if () //   ;[^\n]*
            // // {
            // //     return true;
            // // }
            // else if () //   return [expr]
            // {
            //     return true;
            // }
            else
            {
                return false;
            }
        }
        else
        {
            //cout << "_args matched!" << endl;
            ret_idx = idx;
            return false;
        }
    }

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
    bool Expr(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        if (idx < tok_size)
        {
            if (tokens[idx].type == T_True) //       true
            {
                auto n_true = new AST_node("true");
                node->appendChild(n_true);
                ret_idx = idx + 1;
                return true;
            }
            else if (tokens[idx].type == T_False)  //       false
            {                
                auto n_false = new AST_node("false");
                node->appendChild(n_false);
                ret_idx = idx + 1;
                return true;
            }
            else if (tokens[idx].type == T_IntConstant)   //       -?[0-9]+
            {                
                auto n_constant = new AST_node("constant");
                node->appendChild(n_constant);
                ret_idx = idx + 1;
                return true;
            }
            else if (tokens[idx].type == T_Identifier) //       ident
            {
                auto n_exprs = new AST_node("exprs");
                auto n_expr = new AST_node("expr");
                node->appendChild(new AST_node("n_ident"));
                if (idx + 1 < tok_size)
                {
                    if(tokens[idx+1].literal == "["
                    && Expr(idx+2,ret_idx, n_expr)
                    && tokens[ret_idx].literal == "]" ) //      ident\[expr\]
                    {
                        node->appendChild(new AST_node("["));
                        node->appendChild(n_expr);
                        node->appendChild(new AST_node("]"));
                        ret_idx = ret_idx + 1;
                    }
                    else if (tokens[idx+1].literal == "("
                    && Exprs(idx+2,ret_idx,n_exprs)
                    && tokens[ret_idx].literal == ")") //      ident(<, expr>*)
                    {
                        node->appendChild(new AST_node("("));
                        node->appendChild(n_exprs);
                        node->appendChild(new AST_node(")"));
                        ret_idx = ret_idx + 1;
                    }
                    else
                    {
                        ret_idx = idx + 1;
                    }
                    
                } 
                else
                {
                    ret_idx = idx + 1;
                }
                return true;

            }
            // else if () //       (expr ? expr : expr)
            // {
            //     return true;
            // }
            else if (tokens[idx].type == T_SizeOf && idx + 1 < tok_size 
            && tokens[idx+1].literal == "(" && idx + 2 < tok_size
            && tokens[idx+2].type == T_Identifier && idx + 3 < tok_size
            && tokens[idx+3].literal == ")" )          //      sizeof(ident)
            { 
                node->appendChild(new AST_node("n_sizeof"));
                node->appendChild(new AST_node("("));
                node->appendChild(new AST_node("n_identifier"));
                node->appendChild(new AST_node(")"));
                ret_idx = idx + 4;
                return true;
            }
            else if (
            tokens[idx].type == T_Input && idx + 1 < tok_size 
            && tokens[idx+1].literal == "(" && idx + 2 < tok_size
            && tokens[idx+2].literal == ")" 
            ) //      input()
            {
                node->appendChild(new AST_node("n_input"));
                node->appendChild(new AST_node("("));
                node->appendChild(new AST_node(")"));
                ret_idx = idx + 3;
                return true;
            }
            else if (tokens[idx].literal == "(" )
            {
                auto n_lexpr = new AST_node("expr");
                auto n_binop = new AST_node("binop");
                auto n_rexpr = new AST_node("expr");
                auto n_expr = new AST_node("expr");
                auto n_unaryop = new AST_node("unaryop");
                if( Expr(idx+1, ret_idx, n_lexpr) && Binop(ret_idx,ret_idx,n_binop) 
                    && Expr(ret_idx,ret_idx,n_rexpr)
                    && tokens[ret_idx].literal == ")" 
                )   // (expr binop expr)
                {
                    node->appendChild(new AST_node("("));
                    node->appendChild(n_lexpr);
                    node->appendChild(n_binop);
                    node->appendChild(n_rexpr);
                    node->appendChild(new AST_node(")"));
                    ret_idx = ret_idx + 1;
                    return true;
                }
                else if (Unaryop(idx+1,ret_idx,n_unaryop) 
                    && Expr(ret_idx,ret_idx,n_expr)
                    && tokens[ret_idx].literal == ")" )  //      (unaryop expr)
                {
                    node->appendChild(new AST_node("("));
                    node->appendChild(n_unaryop);
                    node->appendChild(n_expr);
                    node->appendChild(new AST_node(")"));
                    ret_idx = ret_idx + 1;
                    return true;
                }
                else
                {
                    ret_idx = idx;
                    return false;
                }
            }
            else
            {
                ret_idx = idx;
                return false;
            }
        }
        else
        {
            //cout << "_args matched!" << endl;
            ret_idx = idx;
            return false;
        }
    }

    // Exprs : = empty
    //         | _Exprs
    bool Exprs(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        if (!_Exprs(idx, ret_idx, node))
        {
            ret_idx = idx;
        }
        return true;
    }

    // _Exprs : = expr
    //         | expr, _Exprs
    bool _Exprs(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {

        auto n_expr = new AST_node("expr");
        auto n_comma = new AST_node(",");
        auto n_pexprs = new AST_node("_exprs");

        if (idx < tok_size)
        {
            if (Expr(idx, ret_idx, n_expr))
            {
                node->appendChild(n_expr);
                if(tokens[ret_idx].literal == ","
                && _Exprs(ret_idx + 1, ret_idx, n_pexprs))
                {
                    node->appendChild(n_comma);
                    node->appendChild(n_pexprs);
                }
                return true;
            }
            else
            {
                ret_idx = idx;
                return false;
            }
        }
        else
        {
            ret_idx = idx;
            return false;
        }
    }

    // binop :=
    //  + - * ^ / % & | == != > >= < <=
    bool Binop(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        if (idx < tok_size && tokens[idx].type == T_Binop 
            ||tokens[idx].type == T_Eq || tokens[idx].type == T_Le 
            || tokens[idx].type == T_Ne || tokens[idx].type == T_Ge)
        {
            node->appendChild(new AST_node(tokens[idx].literal));
            ret_idx = idx + 1;
            return true;
        }
        else
            return false;
    }

    // unaryop :=
    //     - !
    //     (bang is Boolean not)
    bool Binop(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
    {
        if (idx < tok_size && tokens[idx].literal == "!" 
            ||tokens[idx].literal == "-")
        {
            node->appendChild(new AST_node(tokens[idx].literal));
            ret_idx = idx + 1;
            return true;
        }
        else
            return false;
    }

  public:
    Parser(string path) : root("Prog")
    {
        tokens = scanner(path);
        tok_size = tokens.size();
        // for (auto s : tokens)
        // {
        //     cout << s.literal << "\t" << s.lineno << endl;
        // }
    }
    bool parse()
    {
        vector<token_t>::size_type ret_idx = 0;
        return Prog(0, ret_idx, &root);
    }
    void printTree()
    {
        queue<tuple<AST_node, int>> s;
        s.push(make_tuple(root, 0));
        while (s.size() > 0)
        {
            cout << setiosflags(ios::left) << std::setw(15) << get<0>(s.front()).tok << "\t" << get<1>(s.front()) << endl;
            for (auto i : get<0>(s.front()).children)
            {
                s.push(make_tuple(*i, get<1>(s.front()) + 1));
            }
            s.pop();
        }
    }
};

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        std::cerr << "Usage: parse <filename>" << std::endl;
        return 1;
    }

    std::string path = std::string(argv[1]);
    Parser p(path);
    if (p.parse())
    {
        p.printTree();
        cout << "success" << endl;
    }
    else
        cout << "failed" << endl;
    return 0;
}
