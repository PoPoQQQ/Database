#include "ForeignKeyCstrnt.h"
#include "Database.h"

ForeignKeyCstrnt::ForeignKeyCstrnt(const char* name, FieldConstraint::ConstraintType type)
    :FieldConstraint(name, type){

}

ForeignKeyCstrnt::ForeignKeyCstrnt(const ForeignKeyCstrnt& other) {
    this->colList = other.colList;
    strcpy(this->tbName, other.tbName);
    this->ref_colList = other.ref_colList;
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

int ForeignKeyCstrnt::getFieldIndex(const vector<Field>& fields, const char* colName) const {
    for(int i = 0; i < (signed)fields.size(); i++)
		if(strcmp(fields[i].columnName, colName) == 0)
			return i;
	return -1;
}

void ForeignKeyCstrnt::apply(FieldList& fieldList) {
    // 1. 检查 tbName 对应的表是否存在
	// 检查外键所关联的表是否存在
    map<string, Table*>::iterator ref_entry = (Database::currentDatabase)->tables.find(string(this->tbName));
    // 不存在则报错退出
	if(ref_entry == (Database::currentDatabase)->tables.end()){
		char buf[256];
		snprintf(buf, sizeof(buf), "Error: Can't load referenced table `%s`", this->tbName);
		throw string(buf);
	}
    
    // 检查两者长度是否相同（只能绑定对应长度相同的联合键）
    //* 已经在解析阶段进行过判断

    // 检查绑定的键是否都存在
    for(vector<string>::const_iterator it = colList.begin(); it != colList.end(); ++it) {
        if(fieldList.GetColumnIndex(it->c_str()) == -1) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Error: key `%s` doesn't exist in current Table", it->c_str());
            throw string(buf);
        }
    }
    // 检查外键是否都存在
    Table* const ref_table = ref_entry->second;
    FieldList& ref_fieldList = ref_table->fieldList;
    for(vector<string>::const_iterator it = ref_colList.begin(); it != ref_colList.end(); ++it) {
        if(ref_fieldList.GetColumnIndex(it->c_str()) == -1) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Error: key `%s` doesn't exist in reference Table", it->c_str());
            throw string(buf);
        }
    }
    // 检查外键是否具有 index
    // TODO: 未来可能会修改检查 index 的方式
    if(ref_colList.size() == 1){
        // 如果长度为一则只需要查看该键是否具有 index (也就是是否具有主键或是外键)
        const Field& ref_field = ref_fieldList.GetColumn(ref_fieldList.GetColumnIndex(ref_colList[0].c_str()));
        // 判断其是否具有索引
        if((ref_field.constraints & Field::PRIMARY_KEY) || (ref_field.constraints & Field::FOREIGN_KEY)) {
            // 如果具有索引，则将当前的键标记外键
            Field& bind_field = fieldList.GetColumn(fieldList.GetColumnIndex(this->colList[0].c_str()));
            bind_field.SetForeignKey(tbName, ref_colList[0].c_str(), this->name);
        } else {
            char buf[256];
            snprintf(buf, sizeof(buf), "Error: Foreign key `%s` doesn't have index", ref_field.columnName);
            throw string(buf);
        }
    } else if(ref_colList.size() > 1) {
        // 如果超过一，则说明是联合外键，必须关联联合主键
        const vector<PrimaryKeyCstrnt>& ref_pkConstraints = ref_table->fieldList.pkConstraints;
        // 如果没有主键则不可能绑定成功
        if(ref_pkConstraints.size() == 0) {
            throw "Error: ForeignKey reference doesn't have combined primary key.";
        }
        // 判断联合外键对应的联合主键是否正确
        if(this->ref_colList.size() != ref_pkConstraints[0].pkList.size()) {
            throw "Error: ForeignKey reference error length";
        } else {
            // TODO: 逐个进行名称匹配
            cout << "TODO: Add multikey reference" << endl;
        }
    } else {
        throw "Error: ForeignKey reference length is 0";
    }
}