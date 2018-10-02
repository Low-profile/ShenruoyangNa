
#include "parser.h"

using namespace std;

// bool Parser::Stmt(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
// {
//     auto n_stmtblcok = new AST_node("stmtblock");
//     auto n_expr = new AST_node("expr");

//     auto n_type = new AST_node("type");
//     auto n_idents = new AST_node("idents");

//     auto n_arrayIdents = new AST_node("arrayIdents");

//     auto n_printArgs = new AST_node("printArgs");

//     auto n_ident = new AST_node("ident");

//     auto n_expr_2 = new AST_node("expr");

//     if (idx < tok_size)
//     {
//         string name;

//         if (Stmtblock(idx, ret_idx, n_stmtblcok)) //   stmtblock
//         {
//             node->appendChild(n_stmtblcok);
//             return true;
//         }
//         else if (
//             tokens[idx].type == T_Array && ArrayIdents(idx + 1, ret_idx, n_arrayIdents)) //   array ident\[expr\] <, ident\[expr\]>*
//         {
//             node->appendChild(new AST_node("array"));
//             node->appendChild(n_arrayIdents);
//             return true;
//         }
//         else if (
//             Type(idx, ret_idx, n_type, name) && Idents(ret_idx, ret_idx, n_idents)) //   type ident <, ident>*
//         {
//             node->appendChild(n_type);
//             node->appendChild(n_idents);
//             return true;
//         }

//         else if (
//             tokens[idx].type == T_Print && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && printArgs(idx + 2, ret_idx, n_printArgs) && ret_idx < tok_size && tokens[ret_idx].literal == ")") //   print(<string|expr> <, <string|expr> >*
//         {
//             node->appendChild(new AST_node("print"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(n_printArgs);
//             node->appendChild(new AST_node(")"));
//             ret_idx += 1;
//             return true;
//         }
//         else if (
//             tokens[idx].type == T_Abort && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && printArgs(idx + 2, ret_idx, n_printArgs) && ret_idx < tok_size && tokens[ret_idx].literal == ")") //   abort(<string|expr> <, <string|expr> >*
//         {
//             node->appendChild(new AST_node("abort"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(n_printArgs);
//             node->appendChild(new AST_node(")"));
//             ret_idx += 1;
//             return true;
//         }
//         else if (
//             tokens[idx].type == T_If && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && Expr(idx + 2, ret_idx, n_expr) && ret_idx < tok_size && tokens[ret_idx].literal == ")" && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok)) //   if (expr) stmtblock [else stmtblock]
//         {
//             auto tmp_idx = ret_idx;
//             node->appendChild(new AST_node("if"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(n_expr);
//             node->appendChild(new AST_node(")"));
//             node->appendChild(n_stmtblcok);

//             auto n_stmtblcok_ = new AST_node("stmtblock");
//             if (ret_idx < tok_size && tokens[ret_idx].type == T_Else && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok_))
//             {
//                 node->appendChild(new AST_node("else"));
//                 node->appendChild(n_stmtblcok_);
//             }
//             else
//                 ret_idx = tmp_idx;

//             return true;
//         }
//         else if (
//             tokens[idx].type == T_While && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && Expr(idx + 2, ret_idx, n_expr) && ret_idx < tok_size && tokens[ret_idx].literal == ")" && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok)) //   while (expr) stmtblock
//         {
//             node->appendChild(new AST_node("while"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(n_expr);
//             node->appendChild(new AST_node(")"));
//             node->appendChild(n_stmtblcok);
//             return true;
//         }
//         else if (
//             tokens[idx].type == T_For && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && Ident(idx + 2, ret_idx, n_ident, name) && ret_idx < tok_size && tokens[ret_idx].literal == ":" && Expr(ret_idx + 1, ret_idx, n_expr) && ret_idx < tok_size && tokens[ret_idx].literal == ")" && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok)) //   for (ident : expr) stmtblock
//         {
//             node->appendChild(new AST_node("for"));
//             node->appendChild(new AST_node("("));
//             node->appendChild(n_ident);
//             node->appendChild(new AST_node(":"));
//             node->appendChild(n_expr);
//             node->appendChild(new AST_node(")"));
//             node->appendChild(n_stmtblcok);
//             return true;
//         }
//         else if (
//             Ident(idx, ret_idx, n_ident, name) && ret_idx < tok_size && tokens[ret_idx].literal == ":" && ret_idx + 1 < tok_size && tokens[ret_idx + 1].literal == "=" && Expr(ret_idx + 2, ret_idx, n_expr)) //   ident := expr
//         {
//             node->appendChild(n_ident);
//             node->appendChild(new AST_node(":"));
//             node->appendChild(new AST_node("="));
//             node->appendChild(n_expr);
//             return true;
//         }
//         else if (
//             Ident(idx, ret_idx, n_ident, name) && ret_idx < tok_size && tokens[ret_idx].literal == "[" && Expr(ret_idx + 1, ret_idx, n_expr) && ret_idx < tok_size && tokens[ret_idx].literal == "]" && ret_idx + 1 < tok_size && tokens[ret_idx + 1].literal == ":" && ret_idx + 2 < tok_size && tokens[ret_idx + 2].literal == "=" && Expr(ret_idx + 3, ret_idx, n_expr_2)) //   ident\[expr\] := expr
//         {
//             node->appendChild(n_ident);
//             node->appendChild(new AST_node("["));
//             node->appendChild(n_expr);
//             node->appendChild(new AST_node("]"));
//             node->appendChild(new AST_node(":"));
//             node->appendChild(new AST_node("="));
//             node->appendChild(n_expr_2);
//             return true;
//         }
//         else if (Expr(idx, ret_idx, n_expr)) //   expr
//         {
//             node->appendChild(n_expr);

//             return true;
//         }
//         // // else if () //   ;[^\n]*
//         // // {
//         // //     return true;
//         // // }
//         else if (tokens[idx].type == T_Return) //   return [expr]
//         {
//             node->appendChild(new AST_node("return"));
//             if (Expr(idx + 1, ret_idx, n_expr))
//             {
//                 node->appendChild(n_expr);
//             }
//             else
//             {
//                 ret_idx = idx + 1;
//             }
//             return true;
//         }
//         else
//         {
//             return false;
//         }
//     }
//     else
//     {
//         ret_idx = idx;
//         return false;
//     }
// }

vector<unique_ptr<StmtAST>> Parser::Stmts(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{

    auto n_stmt = new AST_node("stmt");
    auto n_newlines = new AST_node("newlines");
    vector<unique_ptr<StmtAST>> stmts_ast;
    auto tmp_idx = idx;

    if (idx < tok_size)
    {
        int count = 0;
        while(true)
        {
            auto n_stmt = new AST_node("_stmt");
            if(auto stmt_n = Stmt(idx, ret_idx, n_stmt) )
            {
                if(Newlines(ret_idx, ret_idx, n_newlines))
                {
                    count ++;
                    node->appendChild(n_stmt);
                    stmts_ast.push_back(move(stmt_n));
                    idx = ret_idx;
                    continue;
                }
                else
                {
                    ret_idx = tmp_idx;
                    return vector<unique_ptr<StmtAST>>();
                }
            }
            else
            {
                if(tokens[ret_idx].literal != "}")
                {
                    ret_idx = tmp_idx;
                    stmts_ast = vector<unique_ptr<StmtAST>>();
                }
                    
                return stmts_ast;
            }
                
        }
    }
    else
    {
        ret_idx = idx;
        return stmts_ast;
    }
}


vector<string> Parser::Idents(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    auto tmp_idx = idx;
    vector<string> node_n;
    if (idx < tok_size)
    {
        while(true)
        {
            auto n_ident = new AST_node("ident");
            string name;
            if(!Ident(idx, ret_idx, n_ident, name))
            {
                ret_idx = tmp_idx;
                return node_n;
            }
            else if(ret_idx < tok_size && tokens[ret_idx].literal == ",")
            {
                node_n.push_back(name);
                idx = ret_idx + 1;
                continue;
            }
            node_n.push_back(name);
            return node_n;
        }
        // if (Ident(idx, ret_idx, n_ident, name))
        // {
        //     auto tmp_idx = ret_idx;
        //     node->appendChild(n_ident);
        //     auto n_idents = new AST_node("idents");
        //     if (tokens[ret_idx].literal == "," && Idents(ret_idx + 1, ret_idx, n_idents))
        //     {
        //         node->appendChild(new AST_node(","));
        //         for (auto child : n_idents->children)
        //         {
        //             node->appendChild(child);
        //         }
        //         delete n_idents;
        //     }
        //     else
        //     {
        //         ret_idx = tmp_idx;
        //     }

        //     return true;
        // }
        // else
        // {
        //     ret_idx = idx;
        //     return false;
        // }
    }
    else
    {
        ret_idx = idx;
        return node_n;
    }
}

unique_ptr<StmtAST> Parser::Stmt(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    auto n_stmtblcok = new AST_node("stmtblock");
    auto n_expr = new AST_node("expr");

    auto n_type = new AST_node("type");
    auto n_idents = new AST_node("idents");

    auto n_arrayIdents = new AST_node("arrayIdents");

    auto n_printArgs = new AST_node("printArgs");

    auto n_ident = new AST_node("ident");

    auto n_expr_2 = new AST_node("expr");

    if (idx < tok_size)
    {
        string name;

        if (Stmtblock(idx, ret_idx, n_stmtblcok)) //   stmtblock
        {
            node->appendChild(n_stmtblcok);
            return llvm::make_unique<ReturnStmtAST>();
        }
        else if (
            tokens[idx].type == T_Array && ArrayIdents(idx + 1, ret_idx, n_arrayIdents)) //   array ident\[expr\] <, ident\[expr\]>*
        {
            node->appendChild(new AST_node("array"));
            node->appendChild(n_arrayIdents);
            return llvm::make_unique<ReturnStmtAST>();
        }
        else if (
            Type(idx, ret_idx, n_type, name )) //   type ident <, ident>*
        {
            auto decls = Idents(ret_idx, ret_idx, n_idents);
            if(decls.empty())
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_type);
            node->appendChild(n_idents);
            return llvm::make_unique<DeclStmtAST>(move(decls));
        }

        else if (
            tokens[idx].type == T_Print && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && printArgs(idx + 2, ret_idx, n_printArgs) && ret_idx < tok_size && tokens[ret_idx].literal == ")") //   print(<string|expr> <, <string|expr> >*
        {
            node->appendChild(new AST_node("print"));
            node->appendChild(n_printArgs);
            ret_idx += 1;
            return llvm::make_unique<ReturnStmtAST>();
        }
        else if (
            tokens[idx].type == T_Abort && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && printArgs(idx + 2, ret_idx, n_printArgs) && ret_idx < tok_size && tokens[ret_idx].literal == ")") //   abort(<string|expr> <, <string|expr> >*
        {
            node->appendChild(new AST_node("abort"));
            node->appendChild(n_printArgs);
            ret_idx += 1;
            return llvm::make_unique<ReturnStmtAST>();
        }
        else if (
            tokens[idx].type == T_If 
            && idx + 1 < tok_size && tokens[idx + 1].literal == "(" ) //   if (expr) stmtblock [else stmtblock]
        {
            if(auto cond_n = Expr(idx + 2, ret_idx, n_expr ))
            {
                if(ret_idx >= tok_size || tokens[ret_idx].literal != ")")
                {
                    ret_idx = idx;
                    return nullptr;
                } 
                if(auto then_n = Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok))
                {

                    auto tmp_idx = ret_idx;
                    node->appendChild(new AST_node("if"));
                    node->appendChild(n_expr);
                    node->appendChild(n_stmtblcok);
                    auto n_stmtblcok_ = new AST_node("stmtblock");

                    cout << tokens[ret_idx].literal <<endl;
                    if (ret_idx < tok_size && tokens[ret_idx].type == T_Else )
                    {
                        if(auto else_n = Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok_))
                        {
                            node->appendChild(new AST_node("else"));
                            node->appendChild(n_stmtblcok_);
                            return llvm::make_unique<IfStmtAST>(move(cond_n),move(then_n),move(else_n));
                        }
                        else
                        {
                            ret_idx = tmp_idx;
                            return nullptr;
                        }

                    }
                    else
                        ret_idx = tmp_idx;
                        

                    return llvm::make_unique<IfStmtAST>(move(cond_n),move(then_n),nullptr);
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
        else if (
            tokens[idx].type == T_While 
            && idx + 1 < tok_size 
            && tokens[idx + 1].literal == "(" 
            // && Expr(idx + 2, ret_idx, n_expr) 
            // && ret_idx < tok_size && tokens[ret_idx].literal == ")" 
            // && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok)
            ) //   while (expr) stmtblock
        {
            auto expr_n = Expr(idx + 2, ret_idx, n_expr);
            if(!expr_n)
            {
                ret_idx = idx;
                return nullptr;
            }
            if(ret_idx >= tok_size || tokens[ret_idx].literal != ")")
            {
                ret_idx = idx;
                return nullptr;
            }
            auto stmtblock_n = Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok);
            if(!expr_n)
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_expr);
            node->appendChild(n_stmtblcok);
            return llvm::make_unique<ForStmtAST>(move(expr_n),move(stmtblock_n));
        }
        else if (
            tokens[idx].type == T_For && idx + 1 < tok_size && tokens[idx + 1].literal == "(" && Ident(idx + 2, ret_idx, n_ident, name) && ret_idx < tok_size && tokens[ret_idx].literal == ":" && Expr(ret_idx + 1, ret_idx, n_expr) && ret_idx < tok_size && tokens[ret_idx].literal == ")" && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok)) //   for (ident : expr) stmtblock
        {
            node->appendChild(new AST_node("for"));
            node->appendChild(n_ident);
            node->appendChild(n_expr);
            node->appendChild(n_stmtblcok);
            return llvm::make_unique<ReturnStmtAST>();
        }
        else if (
            Ident(idx, ret_idx, n_ident, name) 
            && ret_idx < tok_size && tokens[ret_idx].literal == ":" 
            && ret_idx + 1 < tok_size && tokens[ret_idx + 1].literal == "=" 
            ) //   ident := expr
        {
            auto expr_n = Expr(ret_idx + 2, ret_idx, n_expr);
            if(!expr_n)
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_ident);
            node->appendChild(n_expr);
            return llvm::make_unique<AssignmentStmtAST>(name, move(expr_n));
        }
        else if (
            Ident(idx, ret_idx, n_ident, name) && ret_idx < tok_size && tokens[ret_idx].literal == "[" && Expr(ret_idx + 1, ret_idx, n_expr) && ret_idx < tok_size && tokens[ret_idx].literal == "]" && ret_idx + 1 < tok_size && tokens[ret_idx + 1].literal == ":" && ret_idx + 2 < tok_size && tokens[ret_idx + 2].literal == "=" && Expr(ret_idx + 3, ret_idx, n_expr_2)) //   ident\[expr\] := expr
        {
            node->appendChild(n_ident);
            node->appendChild(n_expr);
            node->appendChild(n_expr_2);
            return llvm::make_unique<ReturnStmtAST>();
        }
        else if (auto expr_n = Expr(idx, ret_idx, n_expr)) //   expr
        {
            node->appendChild(n_expr);

            return llvm::make_unique<ExprStmtAST>(move(expr_n));
        }
        // // else if () //   ;[^\n]*
        // // {
        // //     return true;
        // // }
        else if (tokens[idx].type == T_Return) //   return [expr]
        {
            node->appendChild(new AST_node("return"));
            if (auto expr_n = Expr(idx + 1, ret_idx, n_expr))
            {
                node->appendChild(n_expr);
                return llvm::make_unique<ReturnStmtAST>(ReturnStmtAST(move(expr_n)));
            }
            else
            {
                ret_idx = idx + 1;
                return llvm::make_unique<ReturnStmtAST>(ReturnStmtAST());
            }
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        ret_idx = idx;
        return nullptr;
    }
}