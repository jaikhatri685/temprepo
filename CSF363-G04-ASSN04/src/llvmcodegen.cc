#include "llvmcodegen.hh"
#include "ast.hh"
#include <iostream>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Function.h>
#include <vector>

#define MAIN_FUNC compiler->builder.GetInsertBlock()->getParent()

/*
The documentation for LLVM codegen, and how exactly this file works can be found
ins `docs/llvm.md`
*/

void LLVMCompiler::compile(Node *root)
{
    /* Adding reference to print_i in the runtime library */
    // void printi();
    FunctionType *printi_func_type = FunctionType::get(
        builder.getVoidTy(),
        {builder.getInt64Ty()},
        false);
    Function::Create(
        printi_func_type,
        GlobalValue::ExternalLinkage,
        "printi",
        &module);

    // FunctionType *prints_func_type = FunctionType::get(
    //     builder.getVoidTy(),
    //     {builder.getInt16Ty()},
    //     false
    // );
    // Function::Create(
    //     prints_func_type,
    //     GlobalValue::ExternalLinkage,
    //     "prints",
    //     &module
    // );

    // FunctionType *printl_func_type = FunctionType::get(
    //     builder.getVoidTy(),
    //     {builder.getInt64Ty()},
    //     false
    // );
    // Function::Create(
    //     printl_func_type,
    //     GlobalValue::ExternalLinkage,
    //     "printl",
    //     &module
    // );
    /* we can get this later
        module.getFunction("printi");
    */

    /* Main Function */
    // int main();
    // FunctionType *main_func_type = FunctionType::get(
    //     builder.getInt32Ty(), {}, false /* is vararg */
    // );
    // Function *main_func = Function::Create(
    //     main_func_type,
    //     GlobalValue::ExternalLinkage,
    //     "main",
    //     &module);

    // // create main function block
    // BasicBlock *main_func_entry_bb = BasicBlock::Create(
    //     *context,
    //     "entry",
    //     main_func);

    // // move the builder to the start of the main function block
    // builder.SetInsertPoint(main_func_entry_bb);

    root->llvm_codegen(this);
    Function *main_func = module.getFunction("main");
    if(!main_func){
        printf("[ERROR]: No main function found.\n");
        exit(-1);
    }

    // return 0;
    // builder.CreateRet(builder.getInt32(0));
}

void LLVMCompiler::dump()
{
    outs() << module;
}

void LLVMCompiler::write(std::string file_name)
{
    std::error_code EC;
    raw_fd_ostream fout(file_name, EC, sys::fs::OF_None);
    WriteBitcodeToFile(module, fout);
    fout.flush();
    fout.close();
}

//  ┌―――――――――――――――――――――┐  //
//  │ AST -> LLVM Codegen │  //
// └―――――――――――――――――――――┘   //

// codegen for statements
Value *NodeStmts::llvm_codegen(LLVMCompiler *compiler)
{   
    Value *last = nullptr;
    for (auto node : list)
    {
        last = node->llvm_codegen(compiler);
    }

    return last;
}

Value *NodeDebug::llvm_codegen(LLVMCompiler *compiler)
{
    Value *expr = expression->llvm_codegen(compiler);

    Function *print_func = compiler->module.getFunction("printi");
    Type *var_type = compiler->builder.getInt64Ty();

    Value *finalExpr = compiler->builder.CreateIntCast(expr, var_type, true);

    compiler->builder.CreateCall(print_func, {finalExpr});
    return compiler->builder.getInt64(0);
}

Value *NodeInt::llvm_codegen(LLVMCompiler *compiler)
{
    if (value >= -32768 && value <= 32767)
    {
        return compiler->builder.getInt16(value);
    }
    else if (value >= -2147483648 && value <= 2147483647)
    {
        return compiler->builder.getInt32(value);
    }
    else
    {
        return compiler->builder.getInt64(value);
    }
}

Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler)
{
    Value *left_expr = left->llvm_codegen(compiler);
    Value *right_expr = right->llvm_codegen(compiler);

    if (left_expr->getType() == compiler->builder.getInt32Ty() || right_expr->getType() == compiler->builder.getInt32Ty())
    {
        left_expr = compiler->builder.CreateIntCast(left_expr, compiler->builder.getInt32Ty(), true);
        right_expr = compiler->builder.CreateIntCast(right_expr, compiler->builder.getInt32Ty(), true);
    }

    if (left_expr->getType() == compiler->builder.getInt64Ty() || right_expr->getType() == compiler->builder.getInt64Ty())
    {
        left_expr = compiler->builder.CreateIntCast(left_expr, compiler->builder.getInt64Ty(), true);
        right_expr = compiler->builder.CreateIntCast(right_expr, compiler->builder.getInt64Ty(), true);
    }

    switch (op)
    {
    case PLUS:
        return compiler->builder.CreateAdd(left_expr, right_expr, "addtmp");
    case MINUS:
        return compiler->builder.CreateSub(left_expr, right_expr, "minustmp");
    case MULT:
        return compiler->builder.CreateMul(left_expr, right_expr, "multtmp");
    case DIV:
        return compiler->builder.CreateSDiv(left_expr, right_expr, "divtmp");
    }
}

Value *NodeTerOp::llvm_codegen(LLVMCompiler *compiler)
{
    // Value * cond_expr = condition->llvm_codegen(compiler);
    // Value * left_expr = left->llvm_codegen(compiler);
    // Value * right_expr = right->llvm_codegen(compiler);

    // Value * boolean_eval = compiler->builder.CreateICmpEQ(cond_expr, compiler->builder.getInt32(0));
    // BasicBlock *LeftBB = BasicBlock::Create(left_expr->getContext(), "left");
    // BasicBlock *RightBB = BasicBlock::Create(right_expr->getContext(), "right");

    // Value * branchInstr = compiler->builder.CreateCondBr(boolean_eval, RightBB, LeftBB);
    // return branchInstr;

    return nullptr;
}

Value *NodeAssn::llvm_codegen(LLVMCompiler *compiler)
{   
    Value *expr = expression->llvm_codegen(compiler);
    Value *finalExpr = nullptr;

    IRBuilder<> temp_builder(
        &MAIN_FUNC->getEntryBlock(),
        MAIN_FUNC->getEntryBlock().begin());

    AllocaInst *alloc = nullptr;

    if (compiler->locals[compiler->scope].find(identifier) != compiler->locals[compiler->scope].end())
    {
        printf("[ERROR]: Using already defined variable\n");
        exit(-1);
    }

    if (var_type == "int")
    {
        // printf("Created int\n");
        alloc = temp_builder.CreateAlloca(compiler->builder.getInt32Ty(), 0, identifier);
        if (expr->getType() == compiler->builder.getInt32Ty() || expr->getType() == compiler->builder.getInt16Ty())
        {
            finalExpr = compiler->builder.CreateIntCast(expr, compiler->builder.getInt32Ty(), true);
        }
        else
        {
            printf("[ERROR]: Bigger sized type cannot be type casted into smaller type.\n");
            exit(-1);
        }
    }
    else if (var_type == "short")
    {
        // printf("Created short\n");
        alloc = temp_builder.CreateAlloca(compiler->builder.getInt16Ty(), 0, identifier);
        if (expr->getType() == compiler->builder.getInt16Ty())
        {
            finalExpr = expr;
        }
        else
        {
            printf("[ERROR]: Bigger sized type cannot be type casted into smaller type.\n");
            exit(-1);
        }
    }
    else if (var_type == "long")
    {
        printf("Created long\n");
        alloc = temp_builder.CreateAlloca(compiler->builder.getInt64Ty(), 0, identifier);
        finalExpr = compiler->builder.CreateIntCast(expr, compiler->builder.getInt64Ty(), true);
    }
    else
    {
        printf("[ERROR]: Invaild type!\n");
        exit(-1);
    }

    compiler->localTypes[compiler->scope][identifier] = var_type;
    compiler->locals[compiler->scope][identifier] = alloc;

    compiler->builder.CreateStore(finalExpr, alloc);
    return compiler->builder.getInt64(0);
}

Value *NodeReAssn::llvm_codegen(LLVMCompiler *compiler)
{
    int numScope = compiler->scope;
    while (numScope >= 0)
    {
        if (compiler->locals[numScope].find(identifier) != compiler->locals[numScope].end())
        {
            break;
        }
        numScope--;
    }

    if (numScope == -1)
    {   
        if(compiler->namedValues.find(identifier) != compiler->namedValues.end()){
            AllocaInst *alloc = compiler->builder.CreateAlloca(compiler->namedValues[identifier]->getType(), 0, identifier);
            compiler->locals[compiler->scope][identifier] = alloc;
            compiler->locals[0][identifier] = alloc;

            compiler->localTypes[compiler->scope][identifier] = "int";
            compiler->localTypes[0][identifier] = "int";
            if(compiler->namedValues[identifier]->getType() == compiler->builder.getInt16Ty()){
                compiler->localTypes[compiler->scope][identifier] = "short";
                compiler->localTypes[0][identifier] = "short";
            }
            if(compiler->namedValues[identifier]->getType() == compiler->builder.getInt64Ty()){
                compiler->localTypes[compiler->scope][identifier] = "long";
                compiler->localTypes[0][identifier] = "long";
            }
        }
        else{
            printf("[ERROR]: Variable not defined\n");
            exit(-1);
        }
    }
    else
    {
        compiler->locals[compiler->scope][identifier] = compiler->locals[numScope][identifier];
        compiler->localTypes[compiler->scope][identifier] = compiler->localTypes[numScope][identifier];
    }

    Value *expr = expression->llvm_codegen(compiler);
    if (compiler->localTypes[compiler->scope][identifier] == "short" && compiler->builder.getInt16Ty() != expr->getType())
    {
        printf("[ERROR]: Bigger sized type cannot be type casted into smaller type.\n");
        exit(-1);
    }

    if (compiler->localTypes[compiler->scope][identifier] == "int" && compiler->builder.getInt64Ty() == expr->getType())
    {
        printf("[ERROR]: Bigger sized type cannot be type casted into smaller type.\n");
        exit(-1);
    }

    if (compiler->localTypes[compiler->scope][identifier] == "int")
    {
        expr = compiler->builder.CreateIntCast(expr, compiler->builder.getInt32Ty(), true);
    }
    else if (compiler->localTypes[compiler->scope][identifier] == "short")
    {
        expr = compiler->builder.CreateIntCast(expr, compiler->builder.getInt16Ty(), true);
    }
    else if (compiler->localTypes[compiler->scope][identifier] == "long")
    {
        expr = compiler->builder.CreateIntCast(expr, compiler->builder.getInt64Ty(), true);
    }
    else
    {
        expr = nullptr;
    }

    compiler->builder.CreateStore(expr, compiler->locals[compiler->scope][identifier]);

    return compiler->builder.getInt64(0);
}

Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler)
{
    int numScope = compiler->scope;
    while (numScope >= 0)
    {
        if (compiler->locals[numScope].find(identifier) != compiler->locals[numScope].end())
        {
            break;
        }
        numScope--;
    }

    if (numScope == -1)
    {   
        if(compiler->namedValues.find(identifier) != compiler->namedValues.end()){
            return compiler->namedValues[identifier];
        }
        else{
            printf("[ERROR]: Variable not defined\n");
            exit(-1);
        }
    }
    else
    {
        compiler->locals[compiler->scope][identifier] = compiler->locals[numScope][identifier];
        compiler->localTypes[compiler->scope][identifier] = compiler->localTypes[numScope][identifier];
    }

    if (compiler->locals[compiler->scope].find(identifier) == compiler->locals[compiler->scope].end())
    {
        std::cout << "[ERROR]: Using undeclared variable " << identifier << std::endl;
        exit(-1);
    }

    AllocaInst *alloc = compiler->locals[compiler->scope][identifier];

    // if your LLVM_MAJOR_VERSION >= 14
    if (compiler->localTypes[compiler->scope][identifier] == "int")
    {
        // printf("Created int identifier\n");
        // fflush(stdout);
        return compiler->builder.CreateLoad(compiler->builder.getInt32Ty(), alloc, identifier);
    }
    else if (compiler->localTypes[compiler->scope][identifier] == "short")
    {
        // printf("Created short identifier\n");
        // fflush(stdout);
        return compiler->builder.CreateLoad(compiler->builder.getInt16Ty(), alloc, identifier);
    }
    else
    {
        // printf("Created long identifier\n");
        // fflush(stdout);
        return compiler->builder.CreateLoad(compiler->builder.getInt64Ty(), alloc, identifier);
    }
}

Value *NodeIf::llvm_codegen(LLVMCompiler *compiler)
{
    Value *condV = condition->llvm_codegen(compiler);
    if (!condV)
        return nullptr;

    // Creating scope
    compiler->locals.push_back(std::unordered_map<std::string, AllocaInst *>());
    compiler->localTypes.push_back(std::unordered_map<std::string, std::string>());

    compiler->scope++;

    condV = compiler->builder.CreateIntCast(condV, compiler->builder.getInt64Ty(), true);
    condV = compiler->builder.CreateICmpEQ(condV, compiler->builder.getInt64(0));

    Function *TheFunction = compiler->builder.GetInsertBlock()->getParent();

    BasicBlock *ThenBB = BasicBlock::Create(*compiler->context, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(*compiler->context, "else");
    BasicBlock *MergeBB = BasicBlock::Create(*compiler->context, "ifcont");

    compiler->builder.CreateCondBr(condV, ElseBB, ThenBB);

    compiler->builder.SetInsertPoint(ThenBB);

    Value *thenV = ifBlock->llvm_codegen(compiler);
    if (!thenV)
        return nullptr;

    compiler->builder.CreateBr(MergeBB);
    ThenBB = compiler->builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(ElseBB);
    compiler->builder.SetInsertPoint(ElseBB);

    // Destroying scope
    compiler->scope--;
    compiler->locals.pop_back();
    compiler->localTypes.pop_back();

    // Creating scope
    compiler->locals.push_back(std::unordered_map<std::string, AllocaInst *>());
    compiler->localTypes.push_back(std::unordered_map<std::string, std::string>());

    compiler->scope++;

    Value *elseV = elseBlock->llvm_codegen(compiler);
    if (!elseV)
        return nullptr;

    compiler->builder.CreateBr(MergeBB);
    ElseBB = compiler->builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(MergeBB);
    compiler->builder.SetInsertPoint(MergeBB);
    PHINode *PN = compiler->builder.CreatePHI(compiler->builder.getInt64Ty(), 2, "iftmp");

    thenV = compiler->builder.CreateIntCast(thenV, compiler->builder.getInt64Ty(), true);
    elseV = compiler->builder.CreateIntCast(elseV, compiler->builder.getInt64Ty(), true);

    PN->addIncoming(thenV, ThenBB);
    PN->addIncoming(elseV, ElseBB);

    // Destroying Scope
    compiler->scope--;
    compiler->locals.pop_back();
    compiler->localTypes.pop_back();

    return PN;
}

Value * NodeCallIn::llvm_codegen(LLVMCompiler * compiler){
    return nullptr;
}

Value * NodeCall::llvm_codegen(LLVMCompiler * compiler){
    Function * CalleeF = compiler->module.getFunction(funName);

    if (!CalleeF){
        printf("[ERROR]: Unknown function referenced\n");
        exit(-1);
    }

    if (CalleeF->arg_size() != expressions.size()){
        printf("[ERROR]: Incorrect # arguments passed\n");
        exit(-1);
    }

    std::vector<Value *> ArgsV;
    for (unsigned i = 0; i < expressions.size(); ++i) {
        Value * expr = expressions[i]->llvm_codegen(compiler);

        ArgsV.push_back(expr);

        if (!ArgsV.back())
            return nullptr;
    }

    int idx = 0;
    for(auto &Arg : CalleeF->args()){
        ArgsV[idx] = compiler->builder.CreateIntCast(ArgsV[idx], Arg.getType(), true);
        idx++;
    }

    return compiler->builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Value * NodeFuncIn::llvm_codegen(LLVMCompiler * compiler){
    return nullptr;
}

Value * NodeFunction::llvm_codegen(LLVMCompiler *compiler){
    std::vector<Type *> params;
    for(auto in : inputs){
        if(in.second == "int"){
            params.push_back(compiler->builder.getInt32Ty());
        }
        else if(in.second == "short"){
            params.push_back(compiler->builder.getInt16Ty());
        }
        else{
            params.push_back(compiler->builder.getInt64Ty());
        }
    }

    Type * returnType;
    if(retType == "int"){
        returnType = compiler->builder.getInt32Ty();
    }
    else if(retType == "short"){
        returnType = (compiler->builder.getInt16Ty());
    }
    else{
        returnType = compiler->builder.getInt64Ty();
    }

    FunctionType *FT = FunctionType::get(returnType, params, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, funName, &compiler->module);

    int idx = 0;
    for(auto &Arg : F->args()){
        Arg.setName(inputs[idx++].first);
    }

    Function * TheFunction = F;

    if (!TheFunction)
        return nullptr;

    if (!TheFunction->empty()){
        printf("[ERROR]: Function cannot be redefined.");
        exit(-1);
    }
    
    BasicBlock *BB = BasicBlock::Create(*(compiler->context), "entry", TheFunction);
    compiler->builder.SetInsertPoint(BB);

    compiler->locals.clear();
    compiler->localTypes.clear();
    compiler->locals.push_back(std::unordered_map<std::string, AllocaInst *>());
    compiler->localTypes.push_back(std::unordered_map<std::string, std::string>());

    compiler->namedValues.clear();
    for (auto &Arg : TheFunction->args()){
        compiler->namedValues[std::string(Arg.getName())] = (Value *)&Arg;
    }

    Value * retVal = codeBlock->llvm_codegen(compiler);
    retVal = compiler->builder.CreateIntCast(retVal, returnType, true);
    compiler->builder.CreateRet(retVal);

    verifyFunction(*TheFunction);

    return TheFunction;
}

Value * NodeRet::llvm_codegen(LLVMCompiler * compiler){
    Value * expr = expression->llvm_codegen(compiler);
    expr = compiler->builder.CreateIntCast(expr, compiler->builder.getInt64Ty(), true);
    return expr;
}

#undef MAIN_FUNC