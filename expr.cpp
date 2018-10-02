
#include "parser.h"

using namespace std;

// bool Parser::Expr(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
// {
//     if (idx < tok_size)
//     {
//         if (tokens[idx].type == T_True) //       true
//         {
//             auto n_true = new AST_node("true");
//             node->appendChild(n_true);
//             ret_idx = idx + 1;
//             return true;
//         }
//         else if (tokens[idx].type == T_False) //       false
//         {
//             auto n_false = new AST_node("false");
//             node->appendChild(n_false);
//             ret_idx = idx + 1;
//             return true;
//         }
//         else if (tokens[idx].type == T_IntConstant) //       -?[0-9]+
//         {
//             auto n_constant = new AST_node("constant");
//             node->appendChild(n_constant);
//             ret_idx = idx + 1;
//             return true;
//         }
//         else if (tokens[idx].type == T_Identifier) //       ident
//         {
//             auto n_exprs = new AST_node("exprs");
//             auto n_expr = new AST_node("expr");
//             node->appendChild(new AST_node("n_ident"));
//             if (idx + 1 < tok_size)
//             {
//                 if (tokens[idx + 1].literal == "[" && Expr(idx + 2, ret_idx, n_expr) && tokens[ret_idx].literal == "]") //      ident\[expr\]
//                 {
//                     node->appendChild(new AST_node("["));
//                     node->appendChild(n_expr);
//                     node->appendChild(new AST_node("]"));
//                     ret_idx = ret_idx + 1;
//                 }
//                 else if (tokens[idx + 1].literal == "(" && Exprs(idx + 2, ret_idx, n_exprs) && tokens[ret_idx].literal == ")") //      ident(<, expr>*)
//                 {
//                     node->appendChild(new AST_node("("));
//                     node->appendChild(n_exprs);
//                     node->appendChild(new AST_node(")"));
//                     ret_idx = ret_idx + 1;
//                 }
//                 else
//                 {
//                     ret_idx = idx + 1;
//                 }
//             }
//             else
//             {
//                 ret_idx = idx + 1;
//             }
//             return true;
//         }
//         else if (tokens[idx].type == T_SizeOf && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && idx + 2 < tok_size && tokens[idx + 2].type == T_Identifier && idx + 3 < tok_size && tokens[idx + 3].literal == ")") //      sizeof(ident)
//         {
//             node->appendChild(new AST_node("n_sizeof"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(new AST_node("n_identifier"));
//             node->appendChild(new AST_node(")"));
//             ret_idx = idx + 4;
//             return true;
//         }
//         else if (
//             tokens[idx].type == T_Input && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && idx + 2 < tok_size && tokens[idx + 2].literal == ")") //      input()
//         {
//             node->appendChild(new AST_node("n_input"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(new AST_node(")"));
//             ret_idx = idx + 3;
//             return true;
//         }
//         else if (tokens[idx].literal == "(") // (
//         {
//             auto n_lexpr = new AST_node("expr");
//             auto n_binop = new AST_node("binop");
//             auto n_rexpr = new AST_node("expr");

//             auto n_expr = new AST_node("expr");
//             auto n_unaryop = new AST_node("unaryop");

//             auto n_expr2 = new AST_node("expr");
//             auto n_expr3 = new AST_node("expr");
//             if (Expr(idx + 1, ret_idx, n_lexpr)) // (expr
//             {
//                 if (Binop(ret_idx, ret_idx, n_binop) && Expr(ret_idx, ret_idx, n_rexpr) && tokens[ret_idx].literal == ")") // (expr binop expr)
//                 {
//                     node->appendChild(new AST_node("("));
//                     node->appendChild(n_lexpr);
//                     node->appendChild(n_binop);
//                     node->appendChild(n_rexpr);
//                     node->appendChild(new AST_node(")"));
//                     ret_idx = ret_idx + 1;
//                     return true;
//                 }
//                 else if (
//                     tokens[ret_idx].literal == "?" && Expr(ret_idx + 1, ret_idx, n_expr2) && tokens[ret_idx].literal == ":" && Expr(ret_idx + 1, ret_idx, n_expr3) && tokens[ret_idx].literal == ")") //       (expr ? expr : expr)
//                 {
//                     node->appendChild(new AST_node("("));
//                     node->appendChild(n_lexpr);
//                     node->appendChild(new AST_node("?"));
//                     node->appendChild(n_expr2);
//                     node->appendChild(new AST_node(":"));
//                     node->appendChild(n_expr3);
//                     node->appendChild(new AST_node(")"));
//                     ret_idx = ret_idx + 1;
//                     return true;
//                 }
//                 else
//                     ret_idx = idx;
//                 return false;
//             }
//             else if (Unaryop(idx + 1, ret_idx, n_unaryop) && Expr(ret_idx, ret_idx, n_expr) && tokens[ret_idx].literal == ")") //      (unaryop expr)
//             {
//                 node->appendChild(new AST_node("("));
//                 node->appendChild(n_unaryop);
//                 node->appendChild(n_expr);
//                 node->appendChild(new AST_node(")"));
//                 ret_idx = ret_idx + 1;
//                 return true;
//             }
//             else
//             {
//                 ret_idx = idx;
//                 return false;
//             }
//         }
//         else
//         {
//             ret_idx = idx;
//             return false;
//         }
//     }
//     else
//     {
//         ret_idx = idx;
//         return false;
//     }
// }

// bool Parser::Exprs(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
// {
//     if (!_Exprs(idx, ret_idx, node))
//     {
//         ret_idx = idx;
//     }
//     return true;
// }

vector<unique_ptr<ExprAST>> Parser::Exprs(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{

    // auto n_expr = new AST_node("expr");
    auto n_pexprs = new AST_node("_exprs");
    auto tmp_idx = idx;
    vector<unique_ptr<ExprAST>> node_n;

    if (idx < tok_size)
    {
        while(true)
        {
            auto n_expr = new AST_node("expr");
            auto args_n = Expr(idx, ret_idx, n_expr);
            if(!args_n)
            {
                ret_idx = tmp_idx;
                return node_n;
            }
            else if(ret_idx < tok_size && tokens[ret_idx].literal == ",")
            {
                node_n.push_back(move(args_n));
                idx = ret_idx + 1;
                continue;
            }
            node_n.push_back(move(args_n));
            return node_n;
        }
    }
    else
    {
        ret_idx = idx;
        return node_n;
    }

}

unique_ptr<ExprAST> Parser::Expr(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    if (idx < tok_size)
    {
        if (tokens[idx].type == T_True) //       true
        {
            auto n_true = new AST_node("true");
            node->appendChild(n_true);
            ret_idx = idx + 1;
            return llvm::make_unique<BoolExprAST>(1);
        }
        else if (tokens[idx].type == T_False) //       false
        {
            auto n_false = new AST_node("false");
            node->appendChild(n_false);
            ret_idx = idx + 1;
            return llvm::make_unique<BoolExprAST>(0);
        }
        else if (tokens[idx].type == T_IntConstant) //       -?[0-9]+
        {
            auto n_constant = new AST_node("constant");
            node->appendChild(n_constant);
            ret_idx = idx + 1;
            return llvm::make_unique<NumberExprAST>(atof(tokens[idx].literal.c_str()));
        }
        else if (tokens[idx].type == T_Identifier) //       ident
        {
            auto n_exprs = new AST_node("exprs");
            auto n_expr = new AST_node("expr");
            node->appendChild(new AST_node("n_ident"));
            if (idx + 1 < tok_size)
            {
                if (tokens[idx + 1].literal == "[" 
                && Expr(idx + 2, ret_idx, n_expr) && tokens[ret_idx].literal == "]") //      ident\[expr\]
                {
                    node->appendChild(n_expr);
                    ret_idx = ret_idx + 1;
                }
                else if (tokens[idx + 1].literal == "(" ) //      ident(<, expr>*)
                {
                    auto exprs_n = Exprs(idx + 2, ret_idx, n_exprs);
                    if(tokens[ret_idx].literal == ")")
                    {
                        node->appendChild(n_exprs);
                        ret_idx = ret_idx + 1;
                        return llvm::make_unique<CallExprAST>(tokens[idx].literal,move(exprs_n));

                    }
                    else
                    {
                        ret_idx = idx + 1;

                        return llvm::make_unique<IdentExprAST>(tokens[idx].literal);
                    }
                }
                else
                {
                    ret_idx = idx + 1;
                    return llvm::make_unique<IdentExprAST>(tokens[idx].literal);
                }
            }
            else
            {
                ret_idx = idx + 1;
                return llvm::make_unique<IdentExprAST>(tokens[idx].literal);
            }
            return llvm::make_unique<IdentExprAST>(tokens[idx].literal);

        }
        else if (tokens[idx].type == T_SizeOf && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && idx + 2 < tok_size && tokens[idx + 2].type == T_Identifier && idx + 3 < tok_size && tokens[idx + 3].literal == ")") //      sizeof(ident)
        {
            node->appendChild(new AST_node("n_sizeof"));
            node->appendChild(new AST_node("("));
            node->appendChild(new AST_node("n_identifier"));
            node->appendChild(new AST_node(")"));
            ret_idx = idx + 4;
            return llvm::make_unique<NumberExprAST>(0);
        }
        else if (
            tokens[idx].type == T_Input && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && idx + 2 < tok_size && tokens[idx + 2].literal == ")") //      input()
        {
            node->appendChild(new AST_node("n_input"));
            node->appendChild(new AST_node("("));
            node->appendChild(new AST_node(")"));
            ret_idx = idx + 3;
            return llvm::make_unique<NumberExprAST>(0);
        }
        else if (tokens[idx].literal == "(") // (
        {
            auto n_lexpr = new AST_node("expr");
            auto n_binop = new AST_node("binop");
            auto n_rexpr = new AST_node("expr");

            auto n_expr = new AST_node("expr");
            auto n_unaryop = new AST_node("unaryop");

            auto n_expr2 = new AST_node("expr");
            auto n_expr3 = new AST_node("expr");
            if (Expr(idx + 1, ret_idx, n_lexpr)) // (expr
            {
                if (Binop(ret_idx, ret_idx, n_binop) && Expr(ret_idx, ret_idx, n_rexpr) && tokens[ret_idx].literal == ")") // (expr binop expr)
                {
                    node->appendChild(new AST_node("("));
                    node->appendChild(n_lexpr);
                    node->appendChild(n_binop);
                    node->appendChild(n_rexpr);
                    node->appendChild(new AST_node(")"));
                    ret_idx = ret_idx + 1;
                    return llvm::make_unique<NumberExprAST>(0);
                }
                else if (
                    tokens[ret_idx].literal == "?" && Expr(ret_idx + 1, ret_idx, n_expr2) && tokens[ret_idx].literal == ":" && Expr(ret_idx + 1, ret_idx, n_expr3) && tokens[ret_idx].literal == ")") //       (expr ? expr : expr)
                {
                    node->appendChild(new AST_node("("));
                    node->appendChild(n_lexpr);
                    node->appendChild(new AST_node("?"));
                    node->appendChild(n_expr2);
                    node->appendChild(new AST_node(":"));
                    node->appendChild(n_expr3);
                    node->appendChild(new AST_node(")"));
                    ret_idx = ret_idx + 1;
                    return llvm::make_unique<NumberExprAST>(0);
                }
                else
                    ret_idx = idx;
                return nullptr;
            }
            else if (Unaryop(idx + 1, ret_idx, n_unaryop) && Expr(ret_idx, ret_idx, n_expr) && tokens[ret_idx].literal == ")") //      (unaryop expr)
            {
                node->appendChild(new AST_node("("));
                node->appendChild(n_unaryop);
                node->appendChild(n_expr);
                node->appendChild(new AST_node(")"));
                ret_idx = ret_idx + 1;
                return llvm::make_unique<NumberExprAST>(0);
            }
            else
            {
                ret_idx = idx;
                return nullptr;
            }
        }
        else
        {
            ret_idx = idx;
            return nullptr;
        }
    }
    else
    {
        ret_idx = idx;
        return nullptr;
    }
}
