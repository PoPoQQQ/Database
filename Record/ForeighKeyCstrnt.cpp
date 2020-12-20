#include "ForeignKeyCstrnt.h"
#include "Database.h"

ForeignKeyCstrnt::ForeignKeyCstrnt(const char* name, FieldConstraint::ConstraintType type)
    :FieldConstraint(name, type){

}

ForeignKeyCstrnt::ForeignKeyCstrnt(const ForeignKeyCstrnt& other) {
    this->colList = other.colList;
    strcpy(this->tbName, other.tbName);
    this->ref_colList = other.colList;
}

string ForeignKeyCstrnt::toString() const {
    char buf[256];
    snprintf(buf, 256, "ForeignKeyCstrnt {name: %s}", this->name);
    return string(buf);
}

BufType ForeignKeyCstrnt::LoadConstraint(BufType b) {
    return LoadBasic(b);
}

BufType ForeignKeyCstrnt::SaveConstraint(BufType b) const {
    return SaveBasic(b);
}

void ForeignKeyCstrnt::apply(const Table& table) {
    // 检查外键所关联的表是否存在
    map<string, Table*>::iterator ref_entry = (Database::currentDatabase)->tables.find(string(this->tbName));
    // 不存在则报错退出
    if(ref_entry == (Database::currentDatabase)->tables.end()){
        char buf[100];
        snprintf(buf, 100, "Error: Referenced table doesn't exist: %s", this->tbName);
        throw buf;
    }
    // 检查外键的长度，如果为 1 则可以直接绑定
    if(ref_colList.size() == 1){

    } else if(ref_colList.size() > 1) {
        // 如果超过一，则说明是联合外键，必须关联联合主键
        const Table* ref_table = ref_entry->second;
        const PrimaryKeyCstrnt& ref_pkConstraint = ref_table->fieldList.pkConstraint;
        // 判断联合外键对应的联合主键是否正确
        if(this->ref_colList.size() != ref_pkConstraint.pkList.size()) {
            throw "Error: ForeignKey reference error length";
        } else {
            // 逐个进行名称匹配
        }
    } else {
        throw "Error: ForeignKey reference length is 0";
    }
}