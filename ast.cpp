#include "ast.h"
#include "llvm/ADT/APFloat.h"
#include <llvm/IR/Verifier.h>
#include <tuple>
#include <llvm/IR/TypeBuilder.h>


#include <iostream>
#include <scanner.h>

using namespace std;

extern LLVMContext TheContext;
extern IRBuilder<> Builder;
extern unique_ptr<Module> TheModule;
//extern map<string, AllocaInst *> NamedValues;


StructType * StructTy;

BasicBlock *ReturnBB;


AllocaInst *CreateEntryBlockAlloca(Function *TheFunction,
                                          const string &VarName , Type* type) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, 0,
                           VarName.c_str());
}

void declareRuntime()
{
    if (!(TheModule->getFunction("extern_input")))
    {
        FunctionType *funcType = FunctionType::get(Type::getInt64Ty(TheContext),false);
        Function::Create(funcType, Function::ExternalLinkage,"extern_input", TheModule.get());
    }
    if (!(TheModule->getFunction("extern_print_string")))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(Type::getVoidTy(TheContext),
                                                               llvm::Type::getInt8PtrTy(TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                "extern_print_string", TheModule.get());
    }

    if (!(TheModule->getFunction("extern_print_int")))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(Type::getVoidTy(TheContext),
                                                               Type::getInt64Ty(TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                            "extern_print_int", TheModule.get());
    }

    if (!(TheModule->getFunction("extern_pow")))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(Type::getInt64Ty(TheContext),
                                                               {Type::getInt64Ty(TheContext), 
                                                               Type::getInt64Ty(TheContext)},
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      "extern_pow", TheModule.get());
    }

    if (!(TheModule->getFunction("extern_abort")))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(Type::getVoidTy(TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      "extern_abort", TheModule.get());
    }

    if (!(TheModule->getFunction("memset")))
    {
        auto FuncTy1 = TypeBuilder<void(int8_t*, int8_t, int64_t, types::i<1>), false>::get(TheContext);
        auto FuncLLVMMemsetP0i8I64 = Function::Create(FuncTy1, GlobalValue::ExternalLinkage, 
        "llvm.memset.p0i8.i64", TheModule.get());
    }


}

/// LogError* - These are little helper functions for error handling.
Value *LogError(string Str)
{
  cout << Str;
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

  map<string, AllocaInst *> NamedValues;
  if (!TheFunction)
  {

    vector<Type*> ArgTypes;
    for (auto I : Args)
    {
      auto type = get<0>(I);
      if(type == "int")
          ArgTypes.push_back(Type::getInt64Ty(TheContext));
      else if (type == "array")
      {
        ArgTypes.push_back(PointerType::getUnqual(StructTy));
      }
    }
    // cout << "func arg complete" << endl;

    Type * retType;
    if (type == "int")
      retType = Type::getInt64Ty(TheContext);
    else if(type == "void")
      retType = Type::getVoidTy(TheContext);
    else if(type == "bool")
      retType = Type::getInt1Ty(TheContext);

    FunctionType *FT =
        FunctionType::get(retType, ArgTypes, false);

    Function *F =
        Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

    // cout << "func create complete" << endl;

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
    {
      Arg.setName(get<1>(Args[Idx++]));
    }

    TheFunction = F;
  }

  if (!TheFunction)
    return nullptr;

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
  ReturnBB = BasicBlock::Create(TheContext, "return");

  Builder.SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
  {
    // Create an alloca for this variable.
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName(), Arg.getType());

    // Store the initial value into the alloca.
    Builder.CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[Arg.getName()] = Alloca;
    
  }

  if(type != "void")
  {
    vector<string> tmp = {"retval"};
    llvm::make_unique<DeclStmtAST>(type,tmp)->codegen(NamedValues);
  }


  auto body_v = Body->codegen(NamedValues);
  if (!body_v)
    Builder.CreateBr(ReturnBB);

  TheFunction->getBasicBlockList().push_back(ReturnBB);
  Builder.SetInsertPoint(ReturnBB);

  if(type != "void")
  {
      auto retval = Builder.CreateLoad(NamedValues["retval"]);
      Builder.CreateRet(retval);
  }
  else
    Builder.CreateRet(nullptr);


  //cout << "ret val not null" << endl;
  // Finish off the function.
  
  // Validate the generated code, checking for consistency.
  verifyFunction(*TheFunction);
  // cout << getName() << endl;
  return TheFunction;

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;
}

Value * StmtblockAST::codegen(map<string, AllocaInst *>& NamedValues) 
{
  // cout <<  "Stmtblock codegen" <<endl;
  llvm:Value * ret_val = nullptr;
  for (auto &stmt : Stmts)
  {
    if(dynamic_cast<ReturnStmtAST*>(stmt.get()) ||  dynamic_cast<abortStmtAST*>(stmt.get()))
    {
      ret_val = ConstantInt::get(TheContext, APInt(64,1));
    }

    stmt->codegen(NamedValues);
  }

  return ret_val;
}

Value * StmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  // cout <<  "StmtASTcodegen" <<endl;
  return nullptr;
}

Value * ReturnStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  // cout <<  "ReturnStmtAST code gen" <<endl;
  if(Expr)
  {
    auto RetVal = Expr->codegen(NamedValues);
    Builder.CreateStore(RetVal,NamedValues["retval"]);
    Builder.CreateBr(ReturnBB);
  }
  else
    Builder.CreateBr(ReturnBB);

  return nullptr;
}

Value * ExprStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  Expr->codegen(NamedValues);
  // cout <<  "ExprStmtAST codegen complete" <<endl;
  return nullptr;
}


void ProgramAST::codegen()
{
  declareRuntime();
  StructTy = StructType::create(TheContext, "ty_array");
  StructTy->setBody(ArrayRef<Type *>({Type::getInt64PtrTy(TheContext), Builder.getInt64Ty()}), false);
  // cout << typeid(StructTy).name() << endl;
  for (auto &func : Funcs)
  {
    func->codegen();
  }
} 

Value *NumberExprAST::codegen(map<string, AllocaInst *>& NamedValues) {

  return ConstantInt::get(TheContext, APInt(64,Val));
  // return ConstantFP::get(TheContext, APFloat(Val));
}


Value *BoolExprAST::codegen(map<string, AllocaInst *>& NamedValues) {
  return Builder.getInt1(Val);
}


Value *DeclStmtAST::codegen(map<string, AllocaInst *>& NamedValues) {
  for(auto ident : idents)
  {
    if (type == "int")
    {
      auto* A = Builder.CreateAlloca (Type::getInt64Ty(TheContext), nullptr, ident);
      auto* L = ConstantInt::get(Type::getInt64Ty(TheContext), 0);
      Builder.CreateStore (L, A, false);
        NamedValues[ident] = A;

    }
    else if(type == "bool")
    {
      auto* A = Builder.CreateAlloca(Type::getInt1Ty(TheContext), nullptr, ident);
      //auto* L = ConstantInt::get(Type::getInt1Ty(TheContext), 0);
      Builder.CreateStore(Builder.getInt1(false), A, false);
      NamedValues[ident] = A;

    }


  }

  return nullptr;
}


Value *VariableExprAST::codegen(map<string, AllocaInst *>& NamedValues) {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (!V)
    return LogErrorV("Unknown variable name");

  // Load the value.
  string type_str;
  llvm::raw_string_ostream rso(type_str);
  V->getType()->print(rso);
  // cout<<rso.str()<< endl;
  if (PointerType *pointerType = dyn_cast<PointerType>(V->getType()))
  {
    Type* elementType = pointerType->getElementType();
    //errs() << "name is " << Name << " The element type is: " << *elementType << "\n";

    if (elementType->isStructTy()) {
      //return Builder.CreateInBoundsGEP(V, {Builder.getInt64(0), Builder.getInt64(0)}, "arraydecay");
      return V;
    }
  }

  return Builder.CreateLoad(V, Name.c_str());
}

Value *BinaryExprAST::codegen(map<string, AllocaInst *>& NamedValues) {
  Value *L = LHS->codegen(NamedValues);
  Value *R = RHS->codegen(NamedValues);
  if (!L || !R)
    return nullptr;

  if (Op == "+")
    return Builder.CreateAdd(L, R, "addtmp");
  if (Op == "-")
    return Builder.CreateSub(L, R, "subtmp");
  if (Op == "*")
    return Builder.CreateMul(L, R, "multmp");
  // if (Op == "<")
  // {
  //   L = Builder.CreateFCmpULT(L, R, "cmptmp");
  //   // Convert bool 0/1 to double 0.0 or 1.0
  //   return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext),"booltmp");
  // }
  if (Op == "&")
    return Builder.CreateAnd(L, R);
  if (Op == "|")
    return Builder.CreateOr(L, R);
  if (Op == "!=")
    return Builder.CreateICmpNE(L, R);
  if (Op == "==")
    return Builder.CreateICmpEQ(L, R);
  if (Op == ">")
    return Builder.CreateICmpSGT(L, R);
  if (Op == ">=")
    return Builder.CreateICmpSGE(L, R);
  if (Op == "<")
    return Builder.CreateICmpSLT(L, R);
  if (Op == "<=")
    return Builder.CreateICmpSLE(L, R);
  if (Op == "/")
    return Builder.CreateSDiv(L, R);
  if (Op == "%")
    return Builder.CreateSRem(L, R);
  if (Op == "^")
    return Builder.CreateCall(TheModule->getFunction("extern_pow"), {L, R});

  return LogErrorV("invalid binary operator");
  
}

Value *UnaryExprAST::codegen(map<string, AllocaInst *>& NamedValues) {
  Value *exprv = expr->codegen(NamedValues);
  if (!exprv)
    return nullptr;

  if (Op == "-")
    return Builder.CreateNeg(exprv); 
  else if (Op == "!")
    return Builder.CreateNot(exprv); 

  return LogErrorV("invalid unary operator");
  
}


Value *ArrExprAST::codegen(map<string, AllocaInst *>& NamedValues) {

  auto arr_idx = index->codegen(NamedValues);

  Value* LoadPtrStruct1;

  if (PointerType *pointerType = dyn_cast<PointerType>(NamedValues[Name]->getType()))
  {
    Type* elementType = pointerType->getElementType();
    if (elementType -> isPointerTy()) {
        LoadPtrStruct1 = Builder.CreateLoad(NamedValues[Name]);
    }
    else
        LoadPtrStruct1 = NamedValues[Name];
  }

  auto GEPPtrInt32t = Builder.CreateInBoundsGEP(LoadPtrStruct1, {Builder.getInt32(0), Builder.getInt32(0)}, "a1");
  auto LoadPtrInt64t = Builder.CreateLoad(GEPPtrInt32t);
  auto GEPInt64t = Builder.CreateInBoundsGEP(LoadPtrInt64t,arr_idx, "arrayidx");
  return Builder.CreateLoad(GEPInt64t);

}

Value *IfStmtAST::codegen(map<string, AllocaInst *>& NamedValues) {
  Value *CondV = Cond->codegen(NamedValues);
  if (!CondV)
  {
    cout << "if cond shouldn't be null" <<endl;
    return nullptr;
  }
  //cout << "CondV gen" <<endl;
  Function *TheFunction = Builder.GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");
  Builder.CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  Builder.SetInsertPoint(ThenBB);

  auto new_namedvalues = NamedValues;
  Value *ThenV = Then->codegen(new_namedvalues);

  if(!ThenV)
  {
    Builder.CreateBr(MergeBB);
  }

  //cout << "ThenV gen" <<endl;

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);

  new_namedvalues = NamedValues;
  if (Else)
  {
      Value *ElseV = Else->codegen(new_namedvalues);
      if(!ElseV)
      {
        Builder.CreateBr(MergeBB);
      }
  }
  else
  {
    Builder.CreateBr(MergeBB);
  }

  //cout << "Else gen" <<endl;

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);

  return nullptr;
}

Value *CallExprAST::codegen(map<string, AllocaInst *>& NamedValues) {
  // cout << "call " << Args.size() << " args" << endl;

  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  // cout << "CalleeF " << CalleeF->arg_size() << " args" << endl;

  if (!CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen(NamedValues));
    if (!ArgsV.back())
      return nullptr;
  }
  // cout << "call complete " << ArgsV.size()<< endl;
  return Builder.CreateCall(CalleeF, ArgsV);
}

Value *ForStmtAST::codegen(map<string, AllocaInst *>& NamedValues) {
  // Make the new basic block for the loop header, inserting after current
  // block.
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock *PreheaderBB = Builder.GetInsertBlock();


  BasicBlock *WhileCond =
      BasicBlock::Create(TheContext, "while.cond", TheFunction);
  BasicBlock *LoopBB =
      BasicBlock::Create(TheContext, "loop", TheFunction);
  BasicBlock *AfterBB =
    BasicBlock::Create(TheContext, "afterloop", TheFunction);

  
  Builder.CreateBr(WhileCond);
  Builder.SetInsertPoint(WhileCond);

  auto CondV = Cond->codegen(NamedValues);

    // Insert the conditional branch into the end of LoopEndBB.
  Builder.CreateCondBr(CondV, LoopBB, AfterBB);
    

  // Start insertion in LoopBB.
  Builder.SetInsertPoint(LoopBB);



  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  auto new_namedvalues = NamedValues;
  Body->codegen(new_namedvalues);

  // Convert condition to a bool by comparing non-equal to 0.0.
  // CondV = Builder.CreateICmpNE(
  //     CondV, ConstantInt::get(TheContext, APInt(64,0)), "loopcond");

      // Create the "after loop" block and insert it.
  BasicBlock *LoopEndBB = Builder.GetInsertBlock();

  // Insert an explicit fall through from the current block to the LoopBB.
  Builder.CreateBr(WhileCond);

  // Any new code will be inserted in AfterBB.
  Builder.SetInsertPoint(AfterBB);

  // for expr always returns 0.0.
  return nullptr;
}

Value * AssignmentStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  auto ExprV = Expr->codegen(NamedValues);
  
  Builder.CreateStore(ExprV, NamedValues[varname], false);
  // cout << "Assignment ir created \n";
  return nullptr;
}

Value * ArrAssignmentStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  // cout << "arrAssignment ir triggered \n";
  auto ExprV = Expr->codegen(NamedValues);
  auto arr_idx = arridx->codegen(NamedValues);

  if (PointerType *pointerType = dyn_cast<PointerType>(NamedValues[varname]->getType()))
  {
    Type* elementType = pointerType->getElementType();
    Value* LoadPtrStruct1;
    if (elementType -> isPointerTy()) {
        LoadPtrStruct1 = Builder.CreateLoad(NamedValues[varname]);
    }
    else
        LoadPtrStruct1 = NamedValues[varname];

    //errs() << *NamedValues[varname]->getType();
    auto GEPPtrInt32t = Builder.CreateInBoundsGEP(LoadPtrStruct1, {Builder.getInt32(0), Builder.getInt32(0)}, "a1");
    auto LoadPtrInt64t = Builder.CreateLoad(GEPPtrInt32t);
    auto GEPInt64t = Builder.CreateInBoundsGEP(LoadPtrInt64t,arr_idx, "arrayidx");
    Builder.CreateStore(ExprV, GEPInt64t);
    // cout << "arrAssignment ir created \n";
  }

  return nullptr;
}

Value * ArrDeclStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  // cout << "decl arra\n";
  for (auto i = 0; i< idents.size(); i++)
  {
    //auto arrayPtr = new llvm::AllocaInst(Type::getInt64Ty(TheContext), "", block);
    auto exprv = arr_size[i]->codegen(NamedValues);

    //cout << "exprv code gen" <<endl;
    if(auto expr_int = dyn_cast<ConstantInt>(exprv))
    {
      auto size = expr_int->getZExtValue();
      auto arrayType = ArrayType::get(Type::getInt64Ty(TheContext), size);
      auto arrayPtr = Builder.CreateAlloca(arrayType,0,"");
      auto BitcastPtrChar = Builder.CreateBitCast(arrayPtr, Type::getInt8PtrTy(TheContext));
      Builder.CreateCall(TheModule->getFunction("llvm.memset.p0i8.i64"), {BitcastPtrChar, Builder.getInt8(0), Builder.getInt64(8 * size), Builder.getInt1(false)});
      auto AllocStruct1 = Builder.CreateAlloca(StructTy, nullptr, "arr");
      auto GEPPtrInt64t = Builder.CreateInBoundsGEP(AllocStruct1, {Builder.getInt64(0), Builder.getInt32(0)}, "a1");
      auto GEPInt64t = Builder.CreateInBoundsGEP(arrayPtr, {Builder.getInt64(0), Builder.getInt64(0)}, "arraydecay");
      Builder.CreateStore(GEPInt64t, GEPPtrInt64t);
      auto GEPInt64t2 = Builder.CreateInBoundsGEP(AllocStruct1, {Builder.getInt64(0), Builder.getInt32(1)}, "b2");
      Builder.CreateStore(Builder.getInt64(size), GEPInt64t2);
      NamedValues[idents[i]] = AllocStruct1;
    }
    else
    {
      auto arrayPtr = Builder.CreateAlloca(Type::getInt64Ty(TheContext),exprv,"");
      auto BitcastPtrChar = Builder.CreateBitCast(arrayPtr, Type::getInt8PtrTy(TheContext));
      auto memset_size = Builder.CreateMul(exprv, Builder.getInt64(8), "");
      Builder.CreateCall(TheModule->getFunction("llvm.memset.p0i8.i64"), {BitcastPtrChar, Builder.getInt8(0), memset_size, Builder.getInt1(false)});
      auto AllocStruct1 = Builder.CreateAlloca(StructTy, nullptr, "arr");
      auto GEPPtrInt64t = Builder.CreateInBoundsGEP(AllocStruct1, {Builder.getInt64(0), Builder.getInt32(0)}, "a1");
      auto GEPInt64t = Builder.CreateInBoundsGEP(arrayPtr, {Builder.getInt64(0)}, "arraydecay");
      Builder.CreateStore(GEPInt64t, GEPPtrInt64t);
      auto GEPInt64t2 = Builder.CreateInBoundsGEP(AllocStruct1, {Builder.getInt64(0), Builder.getInt32(1)}, "b2");
      Builder.CreateStore(exprv, GEPInt64t2);
      NamedValues[idents[i]] = AllocStruct1;
    }


  }

  return nullptr;
}

Value * inputExprAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  return Builder.CreateCall(TheModule->getFunction("extern_input"));
}

Value * printStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  //cout << "print called" << endl;
  for (auto& arg : args) 
  {
    if(arg->expr !=  nullptr)
    {
      //cout << "expr called" << endl;
      auto value = arg->expr->codegen(NamedValues);
      //cout << "arg generated" << endl;
      Builder.CreateCall(TheModule->getFunction("extern_print_int"), {value});
    }
    else
    {
      // if (arg->str != "\"\"")
      // {
        auto tmpstr = Builder.CreateGlobalStringPtr(arg->str);
        Builder.CreateCall(TheModule->getFunction("extern_print_string"), {tmpstr});
        // cout << arg->str << "\nstring called" << endl;

      // }
    } 
  }
  return nullptr;
}

Value * abortStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  Builder.CreateCall(TheModule->getFunction("extern_abort"));
  Builder.CreateUnreachable();
}

Value * forarrStmtAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  vector<string> idxdeclidents = {"idx"};
  llvm::make_unique<DeclStmtAST>("int", idxdeclidents)->codegen(NamedValues);

  auto arr_ident = array->codegen(NamedValues);
  Value* LoadPtrStruct1;
  if (PointerType *pointerType = dyn_cast<PointerType>(arr_ident->getType()))
  {
    Type* elementType = pointerType->getElementType();
    if (elementType -> isPointerTy()) {
        LoadPtrStruct1 = Builder.CreateLoad(arr_ident);
    }
    else
        LoadPtrStruct1 = arr_ident;

  }
  auto GEPPtrInt32t = Builder.CreateInBoundsGEP(LoadPtrStruct1, {Builder.getInt32(0), Builder.getInt32(1)}, "size");
  auto arr_size = Builder.CreateLoad(GEPPtrInt32t);

  
  auto array_gep = Builder.CreateInBoundsGEP(LoadPtrStruct1, {Builder.getInt32(0), Builder.getInt32(0)}, "arrgep");
  auto arr_ptr = Builder.CreateLoad(array_gep);


  
  auto cond_var = llvm::make_unique<VariableExprAST>(NamedValues["idx"]->getName().str());
  auto idx = cond_var->codegen(NamedValues);




  // Make the new basic block for the loop header, inserting after current
  // block.
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock *PreheaderBB = Builder.GetInsertBlock();


  //COND BB
  auto forcondBB = BasicBlock::Create(TheContext, "for.cond", TheFunction);
  Builder.CreateBr(forcondBB);
  Builder.SetInsertPoint(forcondBB);

  PHINode *cond_phi = Builder.CreatePHI(Type::getInt64Ty(TheContext),
                                        2, "");
  cond_phi->addIncoming(idx, PreheaderBB);
  auto CondV = Builder.CreateICmpSLT(cond_phi, arr_size);

  //LoopBB

  BasicBlock *LoopBB =
  BasicBlock::Create(TheContext, "loop", TheFunction);

  BasicBlock *AfterBB =
  BasicBlock::Create(TheContext, "afterloop", TheFunction);

  // Insert an explicit fall through from the current block to the LoopBB.
  //Builder.CreateBr(LoopBB);
  Builder.CreateCondBr(CondV, LoopBB, AfterBB);


  // Start insertion in LoopBB.
  Builder.SetInsertPoint(LoopBB);

  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  auto val_gep = Builder.CreateInBoundsGEP(arr_ptr, {cond_phi}, "valgep");
  auto it = Builder.CreateLoad(val_gep);

  auto* A = Builder.CreateAlloca(Type::getInt64Ty(TheContext), nullptr, iterator);
  Builder.CreateStore (it, A, false);
  NamedValues[iterator] = A;

  auto new_namedvalues = NamedValues;
  Body->codegen(new_namedvalues);

  auto NextVar = Builder.CreateAdd(cond_phi, ConstantInt::get(TheContext, APInt(64,1)), "inc");


  // cout << "assgin end\n";

  // Convert condition to a bool by comparing non-equal to 0.0.
  // CondV = Builder.CreateICmpNE(
  //     CondV, ConstantInt::get(TheContext, APInt(64,0)), "loopcond");

      // Create the "after loop" block and insert it.
  BasicBlock *LoopEndBB = Builder.GetInsertBlock();


  // Insert the conditional branch into the end of LoopEndBB.
  Builder.CreateBr(forcondBB);

  // Any new code will be inserted in AfterBB.
  Builder.SetInsertPoint(AfterBB);


  // Add a new entry to the PHI node for the backedge.
  cond_phi->addIncoming(NextVar, LoopBB);

  return nullptr;
}

Value * SizeofExprAST::codegen(map<string, AllocaInst *>& NamedValues)
{
  if (PointerType *pointerType = dyn_cast<PointerType>(NamedValues[ident]->getType()))
  {
    Type* elementType = pointerType->getElementType();
    Value* LoadPtrStruct1;
    if (elementType -> isPointerTy()) {
        LoadPtrStruct1 = Builder.CreateLoad(NamedValues[ident]);
    }
    else
        LoadPtrStruct1 = NamedValues[ident];

    auto GEPPtrInt32t = Builder.CreateInBoundsGEP(LoadPtrStruct1, {Builder.getInt32(0), Builder.getInt32(1)}, "arrsize");
    return Builder.CreateLoad(GEPPtrInt32t);
  }
  return nullptr;
}


