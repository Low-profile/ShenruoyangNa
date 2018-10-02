#include "ast.h"
#include "llvm/ADT/APFloat.h"
#include <llvm/IR/Verifier.h>
#include <tuple>

#include <iostream>


using namespace std;

extern LLVMContext TheContext;
extern IRBuilder<> Builder;
extern std::unique_ptr<Module> TheModule;
extern std::map<std::string, Value *> NamedValues;

/// LogError* - These are little helper functions for error handling.
Value *LogError(std::string Str)
{
  std::cout << Str;
  return nullptr;
}

Value *LogErrorV(const char *Str)
{
  LogError(Str);
  return nullptr;
}

Function *FunctionAST::codegen()
{
  // First, check for an existing function from a previous 'extern' declaration.
  Function *TheFunction = TheModule->getFunction(getName());

  if (!TheFunction)
  {
    //std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(TheContext));

    std::vector<Type*> ArgTypes;
    for (auto I : Args)
    {
      auto type = std::get<0>(I);
      if(type == "int")
          ArgTypes.push_back(Type::getInt64Ty(TheContext));
    }

    FunctionType *FT =
        FunctionType::get(Type::getInt64Ty(TheContext), ArgTypes, false);

    Function *F =
        Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
    {
      Arg.setName(std::get<1>(Args[Idx++]));
    }

    TheFunction = F;
  }

  if (!TheFunction)
    return nullptr;

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[Arg.getName()] = &Arg;

  if (Value *RetVal = Body->codegen())
  {
    cout << "ret val not null" << endl;
    // Finish off the function.
    Builder.CreateRet(RetVal);
  }

  // Validate the generated code, checking for consistency.
  verifyFunction(*TheFunction);
  cout << getName() << endl;
  return TheFunction;

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;
}

Value * StmtblockAST::codegen() 
{
  // cout <<  "Stmtblock codegen" <<endl;
  llvm:Value * ret_val = nullptr;
  for (auto &stmt : Stmts)
  {
    if(auto tmp = stmt->codegen())
      ret_val = tmp;
  }
  if (ret_val)
  {
    return ret_val;
  }
  else 
    return nullptr;
}


Value * StmtAST::codegen()
{
  cout <<  "StmtASTcodegen" <<endl;
  return nullptr;
}

Value * ReturnStmtAST::codegen()
{
  cout <<  "ReturnStmtAST code gen" <<endl;
  if(Expr)
    return Expr->codegen();
  else
    return nullptr;
}

Value * ExprStmtAST::codegen()
{
  // cout <<  "StmtASTcodegen" <<endl;
  return Expr->codegen();
}


void ProgramAST::codegen()
{
  for (auto &func : Funcs)
  {
    func->codegen();
  }
} 

Value *NumberExprAST::codegen() {

  return ConstantInt::get(TheContext, APInt(64,Val));
  // return ConstantFP::get(TheContext, APFloat(Val));
}


Value *BoolExprAST::codegen() {
  cout << Val <<  "boolgen" <<endl;
  return ConstantFP::get(TheContext, APFloat(Val));
}

Value *IdentExprAST::codegen() {
  return nullptr;
}

Value *DeclStmtAST::codegen() {
  auto* A = Builder.CreateAlloca (Type::getInt32Ty(TheContext), nullptr, "a");
  auto* L = ConstantInt::get(Type::getInt32Ty(TheContext), 0);
  Builder.CreateStore (L, A, false);

  //cout << dyn_cast<ConstantInt>(ExprV)->getValue().toString(10,true) << endl;
  NamedValues[A->getName()] = A;
  return nullptr;
}

Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (!V)
    LogErrorV("Unknown variable name");
  return V;
}

Value *BinaryExprAST::codegen() {
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return Builder.CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder.CreateFSub(L, R, "subtmp");
  case '*':
    return Builder.CreateFMul(L, R, "multmp");
  case '<':
    L = Builder.CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext),
                                "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}


Value *IfStmtAST::codegen() {
  Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = Builder.CreateICmpNE(
      CondV, ConstantInt::get(TheContext, APInt(64, 0)), "ifcond");

  Function *TheFunction = Builder.GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

  Builder.CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  Builder.SetInsertPoint(ThenBB);

  Value *ThenV = Then->codegen();
  if (!ThenV)
    return nullptr;
  cout << "ThenV not null" <<endl;

  Builder.CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder.GetInsertBlock();

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);

  Value *ElseV = Else->codegen();
  if (!ElseV)
    return nullptr;
  cout << "ElseV not null" <<endl;

  Builder.CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = Builder.GetInsertBlock();

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  PHINode *PN = Builder.CreatePHI(Type::getInt64Ty(TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}

Value *CallExprAST::codegen() {
  cout << "call expr" <<endl;
  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Value *ForStmtAST::codegen() {
  // Make the new basic block for the loop header, inserting after current
  // block.
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock *PreheaderBB = Builder.GetInsertBlock();
  BasicBlock *LoopBB =
      BasicBlock::Create(TheContext, "loop", TheFunction);

  // Insert an explicit fall through from the current block to the LoopBB.
  Builder.CreateBr(LoopBB);

  // Start insertion in LoopBB.
  Builder.SetInsertPoint(LoopBB);

  // Start the PHI node with an entry for Start.
  PHINode *Variable = Builder.CreatePHI(Type::getInt64Ty(TheContext),
                                        2, "");
  
  auto CondV = Cond->codegen();
  Variable->addIncoming(CondV, PreheaderBB);

  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  Body->codegen();

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = Builder.CreateICmpNE(
      CondV, ConstantInt::get(TheContext, APInt(64,0)), "loopcond");

      // Create the "after loop" block and insert it.
  BasicBlock *LoopEndBB = Builder.GetInsertBlock();
  BasicBlock *AfterBB =
      BasicBlock::Create(TheContext, "afterloop", TheFunction);

  // Insert the conditional branch into the end of LoopEndBB.
  Builder.CreateCondBr(CondV, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  Builder.SetInsertPoint(AfterBB);

  // for expr always returns 0.0.
  return nullptr;
}

Value * AssignmentStmtAST::codegen()
{
  auto ExprV = Expr->codegen();
  cout << dyn_cast<ConstantInt>(ExprV)->getValue().toString(10,true) << endl;
  Builder.CreateStore(ExprV, NamedValues[varname], false);

  return nullptr;
}