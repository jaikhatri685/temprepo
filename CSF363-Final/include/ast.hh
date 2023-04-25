#ifndef AST_HH
#define AST_HH

#include <llvm/IR/Value.h>
#include <string>
#include <vector>

struct LLVMCompiler;

/**
Base node class. Defined as `abstract`.
*/
struct Node
{
    enum NodeType
    {
        BIN_OP,
        INT_LIT,
        STMTS,
        ASSN,
        DBG,
        IDENT,
        REASSN,
        TER_OP
    } type;

    virtual std::string to_string() = 0;
    virtual llvm::Value *llvm_codegen(LLVMCompiler *compiler) = 0;
};

/**
    Node for list of statements
*/
struct NodeStmts : public Node
{
    std::vector<Node *> list;

    NodeStmts();
    void push_back(Node *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for binary operations
*/
struct NodeBinOp : public Node
{
    enum Op
    {
        PLUS,
        MINUS,
        MULT,
        DIV
    } op;

    Node *left, *right;

    NodeBinOp(Op op, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};
/*
    Node for ternary operations
*/

struct NodeTerOp : public Node
{
    Node *condition, *left, *right;

    NodeTerOp(Node *conditionptr, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for integer literals
*/
struct NodeInt : public Node
{
    int value;

    NodeInt(int val);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for variable assignments
*/
struct NodeAssn : public Node
{
    std::string identifier;
    std::string var_type;
    Node *expression;

    NodeAssn(std::string id, Node *expr, std::string _var_type);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
 * Node for reassignments
 */

struct NodeReAssn : public Node
{
    std::string identifier;
    Node *expression;

    NodeReAssn(std::string id, Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};
/**
    Node for `dbg` statements
*/
struct NodeDebug : public Node
{
    Node *expression;

    NodeDebug(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for idnetifiers
*/
struct NodeIdent : public Node
{
    std::string identifier;

    NodeIdent(std::string ident);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for if statements
*/
struct NodeIf : public Node
{
    Node *condition, *ifBlock, *elseBlock;

    NodeIf(Node *condition, Node *ifBlock, Node *elseBlock);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for call inputs
*/

struct NodeCallIn : public Node
{
    std::vector<Node *> expressions;
    
    NodeCallIn();
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler * compiler);
};

/**
    Node for call inputs
*/

struct NodeCall : public Node
{
    std::vector<Node *> expressions;
    std::string funName;
    
    NodeCall(std::vector<Node *> _expressions, std::string _funName);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler * compiler);
};

/**
    Node for function inputs
*/

struct NodeFuncIn : public Node
{
    std::vector<std::pair<std::string, std::string>> inputs;
    
    NodeFuncIn();
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler * compiler);
};

/**
    Node for function statements
*/
struct NodeFunction : public Node
{
    Node *codeBlock;
    std::vector<std::pair<std::string, std::string>> inputs;
    std::string retType;
    std::string funName;

    NodeFunction(Node *_codeBlock, std::vector<std::pair<std::string, std::string>> _inputs, std::string _retType, std::string _funName);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for return statements
*/
struct NodeRet : public Node
{
    Node *expression;

    NodeRet(Node * _expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

#endif