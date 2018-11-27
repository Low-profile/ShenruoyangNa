#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Pass.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"


#include <string>
#include <deque>
#include <unordered_map>
#include <iomanip>

using namespace llvm;

const int bit_width = 32;

namespace {
  struct MyCRPass : public FunctionPass {
    private:
      std::unordered_map<Value *, ConstantRange> Results;
      std::deque<Value *> Worklist;
    public:
      static char ID;
      MyCRPass() : FunctionPass(ID) {}

      ConstantRange ProcessValue(Value *V) {
        // V->print(outs());
        // outs() << "\t";
        if (Results.find(V) != Results.end()) {
          //outs() << "found\n";
          return Results.at(V);
        }

        if (auto I = dyn_cast<Instruction>(V)) {
          return ProcessInstruction(I);
        } else if (auto C = dyn_cast<ConstantInt>(V)) {
          return ConstantRange(C->getValue());
        }

        return ConstantRange(bit_width);
      }

      ConstantRange ProcessInstruction (Instruction *I) {
        ConstantRange result = ConstantRange(bit_width);
        if (I->isBinaryOp()) {
          auto Op1 = I->getOperand(0);
          auto Op2 = I->getOperand(1);

          // Op1->print(outs());
          // outs() << "\t";
          // Op2->print(outs());
          // outs() << "\n";
          auto R1 = ProcessValue(Op1);
          auto R2 = ProcessValue(Op2);
          result = R1.binaryOp(Instruction::BinaryOps(I->getOpcode()),R2);

        }

        return result;
      }
      bool runOnFunction(Function &F) override {

        //init
        for (auto &&BB : F) {
          for (auto &&I : BB) {
            Results.insert(std::make_pair(&I,ProcessInstruction(&I)));

            //Results[&I] = ProcessInstruction(&I);
            Worklist.push_back(&I);
          }
        }
        //run
        while (!Worklist.empty()) {
          
          auto I = Worklist.front();
          Worklist.pop_front();

          auto OldResult = Results.at(I);
          auto NewResult = ProcessValue(I);
          if (OldResult != NewResult) {
            Results.at(I) = NewResult;
            // get dependency of instruction
            //outs() << "get dependencies";
            for(auto U : I->users()){
              //U->print(outs());
              if (auto UserV = dyn_cast<Value>(U)){
                Worklist.push_back(UserV);
              }
            }
          }
        }

        outs() << F.getName() <<"\n";

        for (auto p : Results) {
          p.first->print(outs());
          outs() << "\t:\t";
          p.second.print(outs());
          outs() << "\n";
        }
        Results.clear();
        return false;
      }
  }; // end of struct Hello
}  // end of anonymous namespace

char MyCRPass::ID = 0;
static RegisterPass<MyCRPass> X("MyCRPass", "Hello World Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);