%option noyywrap

%option prefix = "nw"

%x CHECK_IF_DEF
%x DEF_IGNORE_NOTFOUND
%x DEF_IGNORE_FOUND

%{
#include <bits/stdc++.h>

std::unordered_map<std::string, std::string> defTable;

std::string checkOtherDefs(std::string value, std::set<std::string> &visited);
%}

%%
<INITIAL>{
    \/\/[^\n]*\n  {
        return 1;
    }
    
    \/\*[^(\*\/)]*\*\/ {
        return 1;
    }

    #ifdef {
        BEGIN(CHECK_IF_DEF);
    }

    #elif   {
        BEGIN(DEF_IGNORE_FOUND);
    }

    #else   {
        BEGIN(DEF_IGNORE_FOUND);
    }

    #endif  {
        return 1;
    }
    
    #undef[ \t]+[a-zA-Z_][a-zA-Z_0-9]*[ \t]*    {
        std::string line(nwtext);
        int nameStart, nameEnd;
        int i = 6;
        while(line[i] == ' ' || line[i] == '\t'){
            i++;
        }
    
        nameStart = i;
        while(line[i] != ' ' && line[i] != '\t'){
            i++;
        }
    
        nameEnd = i;
    
        std::string name = line.substr(nameStart, nameEnd - nameStart);
    
        if(defTable.find(name) != defTable.end()){
            defTable.erase(name);
        }
        return 1;
    }
    
    #def[ \t]+[a-zA-Z_][a-zA-Z_0-9]*[ \t]*\n   {
        std::string line(nwtext);
        int nameStart, nameEnd;
        int i = 4;
        while(line[i] == ' ' || line[i] == '\t'){
            i++;
        }
    
        nameStart = i;
        while(line[i] != ' ' && line[i] != '\t' && line[i] != '\n'){
            i++;
        }
    
        nameEnd = i;
        defTable[line.substr(nameStart, nameEnd - nameStart)] = "1";
    
        return 1;
    }
    
    #def[ \t]+[a-zA-Z_][a-zA-Z_0-9]*[ \t]+([^\\\n]*\\[ \t]*\n)*[^\n]*\n {
        std::string line(nwtext);
        int nameStart, valueStart, nameEnd, valueEnd;
        int i = 4;
        while(line[i] == ' ' || line[i] == '\t'){
            i++;
        }
    
        nameStart = i;
        while(line[i] != ' ' && line[i] != '\t' && line[i] != '\n'){
            i++;
        }
    
        nameEnd = i;
    
        while(line[i] == ' ' || line[i] == '\t'){
            i++;
        }
    
        valueStart = i;
        valueEnd = line.size() - 1;
    
        std::string value = line.substr(valueStart, valueEnd - valueStart);
        std::size_t pos = value.find("\\");
        while((int)pos != -1){
            value.erase(pos, 1);
            pos = value.find("\\");
        }
    
        defTable[line.substr(nameStart, nameEnd - nameStart)] = value;
        return 1;
    }
    
    [A-Za-z_][A-Za-z_0-9]*  {
        std::string line(nwtext);
        if(defTable.find(line) != defTable.end()){
            return 2;
        }
        else{
            return 4;
        }
    }
    
    "+"       { return 4; }
    "-"       { return 4; }
    "*"       { return 4; }
    "/"       { return 4; }
    ";"       { return 4; }
    "("       { return 4; }
    ")"       { return 4; }
    "="       { return 4; }
    [0-9]+    { return 4; }
    [ \t\n]   { return 4;}
    .   {
        return 4;
    }
}

<CHECK_IF_DEF>{
    [A-Za-z_][A-Za-z_0-9]*  {
        std::string line(nwtext);
        if(defTable.find(line) != defTable.end()){
            BEGIN(INITIAL);
        }
        else{
            BEGIN(DEF_IGNORE_NOTFOUND);
        }
    }

    [ \t]   {
        return 1;
    }
}

<DEF_IGNORE_NOTFOUND>{
    "#elif" {
        BEGIN(CHECK_IF_DEF);
    }

    #else {
        BEGIN(INITIAL);
    }

    "#endif"    {
        BEGIN(INITIAL);
    }

    "\n"    {
        /* skip */
    }

    .   {
        /* skip */
    }
}

<DEF_IGNORE_FOUND>{
    "#endif"    {
        BEGIN(INITIAL);
    }
    "\n"    {
        /* skip */
    }
    .   {
        /* skip */
    }
}
%%

std::string returnString(int token, std::string lexeme){
    if(token == 1){
        return "";
    }
    else if(token == 2){
        std::string oldstr = "";
        std::set<std::string> visited;
        while(oldstr != lexeme){
            oldstr = lexeme;
            lexeme = checkOtherDefs(lexeme, visited);
        }

        return lexeme;
    }
    else{
        return lexeme;
    }
}

std::string checkOtherDefs(std::string value, std::set<std::string> &visited){
    int start = -1, end;
    for(end = 0; end < (int)value.size(); end++){
        if(!((value[end] >= 'A' && value[end] <= 'Z') || (value[end] >= 'a' && value[end] <= 'z') || (value[end] >= '0' && value[end] <= '9') || value[end] == '_')){
            std::string checkStr = value.substr(start + 1, end - start - 1);
            
            if(visited.find(checkStr) == visited.end()){
                visited.insert(checkStr);
            }
            else{
                std::cout << "[ERROR]: Define statements have cyclic dependencies" << std::endl;
                exit(0);
            }

            if(defTable.find(checkStr) != defTable.end()){
                value.replace(start + 1, end - start - 1, defTable[checkStr]);
            }

            start = end;
        }
    }

    std::string checkStr = value.substr(start + 1, end - start - 1);

    if(visited.find(checkStr) == visited.end()){
        visited.insert(checkStr);
    }
    else{
        std::cout << "[ERROR]: Define statements have cyclic dependencies" << std::endl;
        exit(0);
    }
    
    if(defTable.find(checkStr) != defTable.end()){
        value.replace(start + 1, end - start - 1, defTable[checkStr]);
    }

    return value;
}