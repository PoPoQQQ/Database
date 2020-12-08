#include "Global.h"

Global::Global(){}

Global::~Global(){
    delete this->instance;
}

Global::Global(const Global&) {

}

Global& Global::operator=(const Global&) {return *this;}

Global* Global::instance = new Global();
Global* Global::getInstance() {
    return instance;
}
