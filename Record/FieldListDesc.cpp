#include "FieldListDesc.h"

void FieldListDesc::addFieldDesc(const FieldDesc& fielddesc){
    if(fielddesc.type == FieldDesc::FieldType::PRIMARY) {
        if(this->columnList.size() > 0) {
            // 重复定义 primary key
            // 在解析的时候直接报错
            cerr << "Define Multiple Primary Keys" << endl;
            throw "Multiple Primary Keys";
        }
        else {
            // 否则添加主键的信息
            this->columnList = fielddesc.columnList;
        }
    }
    else if(fielddesc.type == FieldDesc::FieldType::FOREIGN) {
        if(this->ref_columnList.size() > 0) {

        }
        else {
            
        }
    }
}
