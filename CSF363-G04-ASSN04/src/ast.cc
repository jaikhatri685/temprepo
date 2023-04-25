#include "ast.hh"

#include <string>
#include <vector>

NodeBinOp::NodeBinOp(NodeBinOp::Op ope, Node *leftptr, Node *rightptr) {
    type = BIN_OP;
    op = ope;
    left = leftptr;
    right = rightptr;
}

std::string NodeBinOp::to_string() {
    std::string out = "(";
    switch(op) {
        case PLUS: out += '+'; break;
        case MINUS: out += '-'; break;
        case MULT: out += '*'; break;
        case DIV: out += '/'; break;
    }

    out += ' ' + left->to_string() + ' ' + right->to_string() + ')';

    return out;
}

NodeTerOp::NodeTerOp(Node * conditionptr, Node * leftptr, Node * rightptr){
    type = TER_OP;
    condition = conditionptr;
    left = leftptr;
    right = rightptr;
}

std::string NodeTerOp::to_string(){
    return "(?: " + condition->to_string() + " " + left->to_string() + " " + right->to_string() + ")";
}

NodeInt::NodeInt(int val) {
    type = INT_LIT;
    value = val;
}

std::string NodeInt::to_string() {
    return std::to_string(value);
}

NodeStmts::NodeStmts() {
    type = STMTS;
    list = std::vector<Node*>();
}

void NodeStmts::push_back(Node *node) {
    list.push_back(node);
}

std::string NodeStmts::to_string() {
    std::string out = " ";
    for(auto i : list) {
        out += i->to_string() + " ";
    }

    out += ' ';

    return out;
}

NodeAssn::NodeAssn(std::string id, Node *expr, std::string _var_type) {
    type = ASSN;
    var_type = _var_type;
    identifier = id;
    expression = expr;
}

std::string NodeAssn::to_string() {
    return "(let " + identifier + " " + expression->to_string() + ")";
}

NodeReAssn::NodeReAssn(std::string id, Node *expr) {
    type = REASSN;
    identifier = id;
    expression = expr;
}

std::string NodeReAssn::to_string() {
    return "(assign " + identifier + " " + expression->to_string() + ")";
}

NodeDebug::NodeDebug(Node *expr) {
    type = DBG;
    expression = expr;
}

std::string NodeDebug::to_string() {
    return "(dbg " + expression->to_string() + ")";
}

NodeIdent::NodeIdent(std::string ident) {
    identifier = ident;
}
std::string NodeIdent::to_string() {
    return identifier;
}

NodeIf::NodeIf(Node * _condition, Node * _ifBlock, Node * _elseBlock){
    condition = _condition;
    ifBlock = _ifBlock;
    elseBlock = _elseBlock;
}

std::string NodeIf::to_string(){
    return "(if-else " + condition->to_string() + " " + ifBlock->to_string() + " " + elseBlock->to_string() + ")";
}

NodeCallIn::NodeCallIn(){
    expressions = std::vector<Node *>();
}

std::string NodeCallIn::to_string(){
    return "";
}

NodeCall::NodeCall(std::vector<Node *> _expressions, std::string _funName){
    funName = _funName;
    expressions = _expressions;
}

std::string NodeCall::to_string(){
    std::string out = "( " + funName + "(";

    for(auto node : expressions){
        out += node->to_string() + ", ";
    }
    out += "))";

    return out;
}

NodeFuncIn::NodeFuncIn(){
    inputs = std::vector<std::pair<std::string, std::string>>();
}

std::string NodeFuncIn::to_string(){
    return "";
}

NodeFunction::NodeFunction(Node * _codeBlock, std::vector<std::pair<std::string, std::string>> _inputs, std::string _retType, std::string _funName){
    codeBlock = _codeBlock;
    inputs = _inputs;
    retType = _retType;
    funName = _funName;
}

std::string NodeFunction::to_string(){
    std::string out = "(fun " + funName + " (";
    for(auto in : inputs){
        out += in.first + " : " + in.second + ", ";
    }
    out += ") : " + retType + " ";
    out += codeBlock->to_string() + ")";

    return out;
}

NodeRet::NodeRet(Node * _expr){
    expression = _expr;
}

std::string NodeRet::to_string(){
    return "ret " + expression->to_string();
}