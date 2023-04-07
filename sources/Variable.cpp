#include "Variable.hpp"
using std::string;
Variable::Variable(string key, string value){
    this->key = key;
    this->value =  value;
}

Variable::~Variable() {}

string Variable::getKey(){
    return key;
}

string Variable::getValue(){
    return value;
}