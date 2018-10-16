
#include "parser.h"




using namespace std;


bool Parser::StringConstant(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node, string& name)
{
    if (idx < tok_size && tokens[idx].type == T_StringConstant)
    {
        node ->appendChild(new AST_node("stringconstant"));
        ret_idx = idx + 1;
        name = tokens[idx].literal;
        return true;
    }
    else
    {
        ret_idx = idx;
        return false;
    }
}

string string_replace( const string & s, const string & findS, const std::string & replaceS )
{
    string result = s;
    auto pos = s.find( findS );
    if ( pos == string::npos ) {
        return result;
    }
    result.replace( pos, findS.length(), replaceS );
    return string_replace( result, findS, replaceS );
}

string parse_string(const string& s) {
    static vector< pair< string, string > > patterns = {
        { "\\\\" , "\\" },
        { "\\n", "\n" },
        { "\\r", "\r" },
        { "\\t", "\t" }
        //,{ "\"", "" }
    };
    string result = s;
    for ( const auto & p : patterns ) {
        result = string_replace( result, p.first, p.second );
    }
    return result;
}

unique_ptr<printArg_t> Parser::PrintArg(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{


    auto n_expr = new AST_node("expr");
    auto n_string = new AST_node("string");
    string name;
    if (auto expr_n = Expr(idx, ret_idx, n_expr))
    {
        node->appendChild(n_expr);
        return llvm::make_unique<printArg_t>("",move(expr_n));
    }
    else if (StringConstant(idx, ret_idx, n_string, name))
    {
        name.erase( 0, 1 ); // erase the first character
        name.erase( name.size() - 1 ); // erase the last character
        // replace(name.begin(), name.end(),'\\\\','\\');
        name = parse_string(name);
        node->appendChild(n_string);
        // cout << "name " << name <<endl;
        return llvm::make_unique<printArg_t>(name,nullptr);
    }
    else
    {
        ret_idx = idx;
        return nullptr;
    }
}

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
    }
    else
    {
        ret_idx = idx;
        return node_n;
    }
}

vector<unique_ptr<ExprAST>> Parser::ArrayIdents(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node, vector<string>& idents_n)
{
    auto n_ident = new AST_node("ident");
    auto n_expr = new AST_node("expr");
    vector<unique_ptr<ExprAST>> expr_v;
    auto tmp_idx = idx;

    if (idx < tok_size)
    {
        while(true)
        {
            auto n_ident = new AST_node("ident");
            string name;
            if(!Ident(idx, ret_idx, n_ident, name) || tokens[ret_idx].literal != "[")
            {
                ret_idx = tmp_idx;
                return expr_v;
            }
            auto expr_n = Expr(ret_idx + 1, ret_idx, n_expr);
            if(!expr_n || tokens[ret_idx].literal != "]")
            {
                ret_idx = tmp_idx;
                return expr_v;
            }
            else if(ret_idx + 1 < tok_size && tokens[ret_idx + 1].literal == ",")
            {
                idents_n.push_back(name);
                expr_v.push_back(move(expr_n));
                idx = ret_idx + 2;
                continue;
            }
            idents_n.push_back(name);
            expr_v.push_back(move(expr_n));
            ret_idx = ret_idx + 1;
            return expr_v;
        }
    }
    else
    {
        ret_idx = idx;
        return expr_v;
    }
}


vector<unique_ptr<printArg_t>> Parser::printArgs(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    //auto n_printArg = new AST_node("printArg");
    auto tmp_idx = idx;
    vector<unique_ptr<printArg_t>> printArg_n;
    if (idx < tok_size)
    {
        while(true)
        {
            auto arg = PrintArg(idx, ret_idx, node);
            if(!arg)
            {
                ret_idx = tmp_idx;
                return printArg_n;
            }
            else if(ret_idx < tok_size && tokens[ret_idx].literal == ",")
            {
                printArg_n.push_back(move(arg));
                idx = ret_idx + 1;
                continue;
            }
            printArg_n.push_back(move(arg));
            return printArg_n;
        }
    }
    else
    {
        ret_idx = idx;
        return printArg_n;
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

        if (auto stmtblock_n = Stmtblock(idx, ret_idx, n_stmtblcok)) //   stmtblock
        {
            node->appendChild(n_stmtblcok);
            return move(stmtblock_n);
        }
        else if (
            tokens[idx].type == T_Array ) //   array ident\[expr\] <, ident\[expr\]>*
        {
            vector<string> arrdecl_n;
            auto arrdeclsize_n = ArrayIdents(idx + 1, ret_idx, n_arrayIdents, arrdecl_n);
            if (arrdeclsize_n.empty())
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(new AST_node("array"));
            node->appendChild(n_arrayIdents);
            return llvm::make_unique<ArrDeclStmtAST>(move(arrdecl_n),move(arrdeclsize_n));
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
            return llvm::make_unique<DeclStmtAST>(name, move(decls));
        }

        else if (
            tokens[idx].type == T_Print && idx + 1 < tok_size && tokens[idx + 1].literal == "(" 
            // && printArgs(idx + 2, ret_idx, n_printArgs) 
            // && ret_idx < tok_size && tokens[ret_idx].literal == ")"
            ) //   print(<string|expr> <, <string|expr> >*
        {
            auto args =printArgs(idx + 2, ret_idx, n_printArgs);
            if(args.empty() || ret_idx >= tok_size || tokens[ret_idx].literal != ")")
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_printArgs);
            ret_idx += 1;
            return llvm::make_unique<printStmtAST>(move(args));
        }
        else if (
            tokens[idx].type == T_Abort && idx + 1 < tok_size && tokens[idx + 1].literal == "(" 
            // && printArgs(idx + 2, ret_idx, n_printArgs) 
            // && ret_idx < tok_size && tokens[ret_idx].literal == ")"
            ) //   abort(<string|expr> <, <string|expr> >*
        {
            auto args =printArgs(idx + 2, ret_idx, n_printArgs);
            if(args.empty() || ret_idx >= tok_size || tokens[ret_idx].literal != ")")
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_printArgs);
            ret_idx += 1;
            return llvm::make_unique<abortStmtAST>(move(args));
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

                    // cout << tokens[ret_idx].literal <<endl;
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
            tokens[idx].type == T_For && idx + 1 < tok_size && tokens[idx + 1].literal == "(" 
            && Ident(idx + 2, ret_idx, n_ident, name) 
            && ret_idx < tok_size && tokens[ret_idx].literal == ":" 
            // && Expr(ret_idx + 1, ret_idx, n_expr) 
            // && ret_idx < tok_size && tokens[ret_idx].literal == ")" 
            // && Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok) //   for (ident : expr) stmtblock
        )
        {
            auto expr_n = Expr(ret_idx + 1, ret_idx, n_expr); 
            if(!expr_n || ret_idx >= tok_size || tokens[ret_idx].literal != ")" )
            {
                ret_idx = idx;
                return nullptr;
            }
            auto stmtblock_n = Stmtblock(ret_idx + 1, ret_idx, n_stmtblcok);
            if(!stmtblock_n)
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_ident);
            node->appendChild(n_expr);
            node->appendChild(n_stmtblcok);
            return llvm::make_unique<forarrStmtAST>(name, move(expr_n), move(stmtblock_n));
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
            Ident(idx, ret_idx, n_ident, name) 
            && ret_idx < tok_size && tokens[ret_idx].literal == "[" 
            ) //   ident\[expr\] := expr
        {
            auto arr_idx =  Expr(ret_idx + 1, ret_idx, n_expr);
            if(!arr_idx || ret_idx >= tok_size || tokens[ret_idx].literal != "]" 
            || ret_idx + 1 >= tok_size || tokens[ret_idx + 1].literal != ":" 
            || ret_idx + 2 >= tok_size || tokens[ret_idx + 2].literal != "=" )
            {
                ret_idx = idx;
                return nullptr;
            }
            auto expr_n =Expr(ret_idx + 3, ret_idx, n_expr_2);
            if(!expr_n)
            {
                ret_idx = idx;
                return nullptr;
            }
            node->appendChild(n_ident);
            node->appendChild(n_expr);
            node->appendChild(n_expr_2);
            return llvm::make_unique<ArrAssignmentStmtAST>(name, move(expr_n), move(arr_idx));
        }
        else if (auto expr_n = Expr(idx, ret_idx, n_expr)) //   expr
        {
            node->appendChild(n_expr);

            return llvm::make_unique<ExprStmtAST>(move(expr_n));
        }
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