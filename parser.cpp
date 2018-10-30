
#include <iostream>
#include <queue>
#include <tuple>
#include <iomanip>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>



#include "parser.h"
#include <fstream>

using namespace std;

LLVMContext TheContext;
IRBuilder<> Builder(TheContext);
unique_ptr<Module> TheModule;
map<string, AllocaInst *> NamedValues;


extern int yylineno;
extern char *yytext;
void yyrestart(FILE *fp);
int yylex();

// extern LLVMContext TheContext;
// extern IRBuilder<> Builder(TheContext);
// extern std::unique_ptr<Module> TheModule;
// extern std::map<std::string, Value *> NamedValues;

Parser::Parser(string path, int int_width) : root("Prog"), integer_width(int_width)
{
    tokens = scanner(path);
    //cout << "scanning complete " << endl;

    tok_size = tokens.size();
    // for (auto s : tokens)
    // {
    //     cout << s.literal << "\t" << s.lineno << endl;
    // }
}

unique_ptr<ProgramAST> Parser::parse()
{
    vector<token_t>::size_type ret_idx = 0;
    cout << "start parsing " << root.tok <<  endl;
    // if (auto prog = Prog(0, ret_idx, &root))
    // {
    //     cout << "parsing completed!" <<  endl;
    //     prog->codegen();
    //     return true;
    // }
    // else{
    //     return false;
    // }
    return Prog(0, ret_idx, &root);
}
void Parser::printTree()
{
    queue<tuple<AST_node, int>> s;
    s.push(make_tuple(root, 0));
    while (s.size() > 0)
    {
        cout << setiosflags(ios::left) << setw(15) << get<0>(s.front()).tok << "\t" << get<1>(s.front()) << endl;
        for (auto i : get<0>(s.front()).children)
        {
            s.push(make_tuple(*i, get<1>(s.front()) + 1));
        }
        s.pop();
    }
}

vector<token_t> Parser::scanner(string path)
{
    cout << "Parsing file: " << path << endl;

    FILE *fp = nullptr;
    fp = fopen(path.c_str(), "r");
    yyrestart(fp);

    vector<token_t> tokens;
    int tok = -1;

    while ((tok = yylex()))
    {
        token_t _tok = {yylineno = yylineno + 1, string(yytext), static_cast<token>(tok)};
        tokens.push_back(_tok);
    }
    token_t eof = {yylineno, "EOF", T_EOF};
    tokens.push_back(eof);
    fclose(fp);
    return tokens;
}

bool Parser::Newlines(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    auto n_newlines = new AST_node("newline");

    if (idx < tok_size)
    {
        if (tokens[idx].type == T_NewLine)
        {
            if (idx + 1 < tok_size && Newlines(idx + 1, ret_idx, n_newlines))
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

unique_ptr<ProgramAST> Parser::Prog(vector<token_t>::size_type idx,vector<token_t>::size_type &ret_idx, AST_node *node)
{
    auto n_newlines = new AST_node("n_newlines");
    auto n_prog = new AST_node("n_prog");
    vector<unique_ptr<FunctionAST>> Funcs;
    auto tmp_idx = idx;
    if (idx < tok_size)
    {
        int count = 0;
        while(true)
        {
            auto n_func = new AST_node("func");
            if(auto func = Func(idx, ret_idx, n_func))
            {
                if(Newlines(ret_idx, ret_idx, n_newlines))
                {
                    count ++;
                    node->appendChild(n_func);
                    Funcs.push_back(move(func));
                    idx = ret_idx;
                    continue;
                }
                else
                    return nullptr;

            }
            else
            {
                if(count > 0 && tokens[idx].type == T_EOF)
                {
                    return llvm::make_unique<ProgramAST>(move(Funcs),integer_width);
                }
                else
                    return nullptr;
            }
        }
    }
    else
    {
        ret_idx = idx;
        return nullptr;
    }
}


unique_ptr<FunctionAST> Parser::Func(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    auto n_stmtblock = new AST_node("stmtblock");
    auto n_args = new AST_node("args");

    string type;
    string name;
    vector<tuple<string,string>> args;

    if (Type(idx, ret_idx, node, type)
    && Ident(ret_idx, ret_idx, node, name) 
    && tokens[ret_idx].literal == "(" 
    && Args(ret_idx + 1, ret_idx, n_args, args)
    && tokens[ret_idx].literal == ")" 
    )
    {
        if( auto stmtblock_n = Stmtblock(ret_idx + 1, ret_idx, n_stmtblock))
        {
            node->appendChild(n_args);
            node->appendChild(n_stmtblock);
            return llvm::make_unique<FunctionAST>(type,name,args, move(stmtblock_n),integer_width);
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

bool Parser::Type(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node, string& node_n)
{
    if (idx < tok_size &&
        tokens[idx].type == T_Int ||
        tokens[idx].type == T_Void || tokens[idx].type == T_Bool || tokens[idx].type == T_Array)
    {
        node->appendChild(new AST_node(tokens[idx].literal));
        node_n = tokens[idx].literal;
        ret_idx = idx + 1;
        return true;
    }
    else
    {
        ret_idx = idx;
        return false;
    }
}

bool Parser::Ident(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node, string& node_n)
{
    if (idx < tok_size)
    {
        if (tokens[idx].type == T_Identifier)
        {
            node_n = tokens[idx].literal;
            node->appendChild(new AST_node(tokens[idx].literal));
            ret_idx = idx + 1;
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



bool Parser::Args(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node, vector<tuple<string,string>>& node_n)
{
    auto tmp_idx = idx;
    if (idx < tok_size)
    {
        while(true)
        {
            string type,name;
            if (!Type(idx, ret_idx, node, type) || !Ident(ret_idx, ret_idx, node, name))
            {
                ret_idx = tmp_idx;
                return true;
            }
            else if(ret_idx < tok_size && tokens[ret_idx].literal == ",")
            {
                node_n.push_back(make_tuple(type,name));
                idx = ret_idx + 1;
                continue;
            }
            node_n.push_back(make_tuple(type,name));
            return true;
        }
    }
    else
    {
        ret_idx = idx;
        return false;
    }
}

unique_ptr<StmtblockAST> Parser::Stmtblock(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node)
{
    auto n_newlines = new AST_node("newlines");
    if (idx < tok_size)
    {
        if (tokens[idx].literal == "{" && idx + 1 < tok_size && Newlines(idx + 1, ret_idx, n_newlines) 
        )
        {
            auto stmts_n = Stmts(ret_idx, ret_idx, node);
            if(tokens[ret_idx].literal == "}")
            {
                ret_idx = ret_idx + 1;
                return llvm::make_unique<StmtblockAST>(move(stmts_n));
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







bool Parser::Binop(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node,string& name)
{
    if (idx < tok_size && tokens[idx].type == T_Binop 
    || tokens[idx].type == T_Eq 
    || tokens[idx].type == T_Le 
    || tokens[idx].type == T_Ne 
    || tokens[idx].type == T_Ge)
    {
        node->appendChild(new AST_node(tokens[idx].literal));
        name =tokens[idx].literal;
        ret_idx = idx + 1;
        return true;
    }
    else
        return false;
}

bool Parser::Unaryop(vector<token_t>::size_type idx, vector<token_t>::size_type &ret_idx, AST_node *node,string& name)
{
    if (idx < tok_size && tokens[idx].literal == "!" || tokens[idx].literal == "-")
    {
        node->appendChild(new AST_node(tokens[idx].literal));
        name =tokens[idx].literal;
        ret_idx = idx + 1;
        return true;
    }
    else
        return false;
}


//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        cerr << "Usage: parse <filename>" << endl;
        return 1;
    }

    TheModule = llvm::make_unique<Module>("my cool jit", TheContext);
    
    string path;
    int width = 64;
    if (argc == 2)
    {
        path = string(argv[1]);
    }

    if (argc == 3)
    {
        // -width=64
        string arg1 = string(argv[1]);
        width = stoi(arg1.substr(7));
        path = string(argv[2]);

    }
    Parser p(path,width);
    if (auto prog = p.parse())
    {
        prog->codegen();
        cout << path << "\t" << width << endl;
        cout << "success" << endl;
        //p.printTree();

        raw_ostream& outHandle = outs();
        std::string ir_file = "test.ll";
        if (!ir_file.empty()) {
            std::error_code errc;
            llvm::raw_fd_ostream outfile(ir_file, errc, llvm::sys::fs::OpenFlags::F_None);
            TheModule->print(outfile, nullptr);
            outfile.close();

            system("clang -O0 runtime.c test.ll -o a.out");

        }
        return 0;
    }
    else
    {
        cout << "failed to parse" << endl;
        return 1;
    }
    //return true;
}
