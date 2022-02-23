//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/HelloWorld.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

PreservedAnalyses HelloWorldPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  // errs() << F.getName() << "\n";
  if (F.getName() == "main") {
    errs() << "Got Main\n";

    // check if main has over 10 instructions
    unsigned counter = 0;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (counter > 10) {
        return PreservedAnalyses::all();
      } else {
        counter++;
      }
    }

    // Loop over every instruction in main
    std::list<CallInst *> erase_inst;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (isa<CallInst>(*I)) {
        // Loop over every argument in a instruction
        CallInst *current_inst = cast<CallInst>(&*I);
        Function *f = cast<CallInst>(*I).getCalledFunction();

        // Check if f is a declaired function
        if (!f->isDeclaration()) {
          unsigned argcounter = 0;
          bool has_non_constants = false;

          // Check if all of arguments in this instruction is constansts
          // If not, break the loop
          for (Function::arg_iterator argI = f->arg_begin();
               argI != f->arg_end(); ++argI) {
            Value *value = cast<CallInst>(*I).getArgOperand(argcounter);
            // Check an argument is a constant or not
            if (!isa<Constant>(*value)) {
              has_non_constants = true;
              break;
            }
            argcounter++;
          }

          // If all the arguments are constants, we can inline this instruction
          if (has_non_constants == false) {
            // Push this instruction into the erase_inst list for erasing the
            // instruction in the future
            erase_inst.push_back(current_inst);
            unsigned second_argcounter = 0;
            // Loop through every argument in the instructions to create and
            // replace the formal argument
            for (Function::arg_iterator argI = f->arg_begin();
                 argI != f->arg_end(); ++argI) {
              Value *value =
                  cast<CallInst>(*I).getArgOperand(second_argcounter);
              // Creating a ConstantInt
              LLVMContext &context = value->getContext();
              ConstantInt *v = ConstantInt::get(
                  context, cast<ConstantInt>(*value).getValue());
              // Replace all uses of formal argument
              argI->replaceAllUsesWith(v);
              second_argcounter++;
            }

            // Clone and insert each instruction
          llvm:
            ValueToValueMapTy vmap;
            // Created a list of cloned instructions
            std::list<Instruction *> cloned_instructions;
            // Loop over every instructions in the called function
            for (inst_iterator calledI = inst_begin(f), calledE = inst_end(f);
                 calledI != calledE; ++calledI) {
              // Copy over the instructions
              const Value *inst = cast<Value>(&*calledI);
              auto *new_inst = calledI->clone();
              new_inst->insertBefore(current_inst);
              cloned_instructions.push_back(new_inst);
              vmap[inst] = new_inst;
              errs() << *new_inst << "\n";
            }

            // Remap Instructions
            for (auto *i : cloned_instructions) {
              RemapInstruction(
                  i, vmap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);
            }

            // For the last instruction, which is return, we determine whehter
            // return type is not void, replace all use of it.
            // Then we remove the return instruction that is cloned from the
            // called function
            Instruction *last_inst = cloned_instructions.back();
            Value *last_return_value =
                cast<ReturnInst>(last_inst)->getReturnValue();
            if (last_return_value != NULL) {
              // Return type is NOT void
              current_inst->replaceAllUsesWith(last_return_value);
            }
            // else, Return type is void, no need to replace
            // Delete the remove instruction
            last_inst->eraseFromParent();
          };
        }
      }
    }
    // Remove Instrucitons
    for (auto *i : erase_inst) {
      i->eraseFromParent();
    }
  }
  return PreservedAnalyses::all();
}
