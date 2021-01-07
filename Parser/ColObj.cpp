#include "ColObj.h"
#include "../Record/Table.h"
#include "../Record/Database.h"
#include <exception>
using namespace std;

bool ColObj::isInTable(const Table& table) const {
    // 如果有 tbName 且 tbName 与当前的 table 不同，则不正确
    if(tbName.size() > 0) {
        if(strcmp(tbName.c_str(), table.tableName.c_str()) != 0) {
            return false;
        }
    }
    // 检查 colName 是否存在于 fieldList 中
    if(table.fieldList.GetColumnIndex(colName.c_str()) == -1) {
        return false;
    } else {
        // 存在则返回真
        return true;
    }
}

Field* ColObj::getFieldOrNull(Table& table) {
    // 如果有 tbName 且 tbName 与当前的 table 不同，则不正确
    if(tbName.size() > 0) {
        if(strcmp(tbName.c_str(), table.tableName.c_str()) != 0) {
            return nullptr;
        }
    }
    // 检查 colName 是否存在于 fieldList 中
    int columnIndex = table.fieldList.GetColumnIndex(colName.c_str());
    if(columnIndex == -1) {
        return nullptr;
    } else {
        // 存在则返回
        return &(table.fieldList.GetColumn(columnIndex));
    }
}

bool ColObj::isExistInDB(Table* table) const {
    if(tbName.size() == 0) {
        if(table != nullptr) {
            return this->isInTable(*table);
        } else {
            return false;
        }
    } else {
        try {
            table = Database::GetTable(tbName.c_str());
        }
        catch(const char * err) {
            return false;
        }
        catch(exception e){
            return false;
        }
        if(table != nullptr) {
            return this->isInTable(*table);
        } else {
            return false;
        }
    }
}

Field* ColObj::getFieldOrNullGlobal(Table* table) {
    // 如果缺省了表名，则必须默认利用当前的表
    if(tbName.size() == 0){
        if(table != nullptr) {
            return this->getFieldOrNull(*table);
        } else {
            return nullptr;
        }
    } else {
        try {
            table = Database::GetTable(tbName.c_str());
        }
        catch(const char * err) {
            return nullptr;
        }
        catch(exception e){
            return nullptr;
        }
        if(table != nullptr) {
            return this->getFieldOrNull(*table);
        } else {
            return nullptr;
        }
    }
}

bool ColObj::isInTbMap(const map<string, Table*> tbMap) {
    if(this->tbName.size() == 0) {
        // 在列表中搜索每一个 table
        Table* table = nullptr;
        for(map<string, Table*>::const_iterator it = tbMap.begin(); it != tbMap.end(); ++it) {
            if(it->second->fieldList.GetColumnIndex(this->colName.c_str()) != -1) {
                if(table != nullptr) {
                    // 如果此时已经找到了一个表，则说明这个键重名
                    // 抛出异常，此时无法继续工作
                    char buf[128];
                    snprintf(buf, 128, "Error: ambigious colName (%s)", this->colName.c_str());
                    throw string(buf);
                } else {
                    table = it->second;
                    this->tbName = it->first;
                }
            }
        }
        return true;
    } else {
        const map<string, Table*>::const_iterator it = tbMap.find(this->tbName);
        if(it == tbMap.end()) {
            // 如果表名不存在则返回 false
            return false;
        } else {
            return it->second->fieldList.GetColumnIndex(this->colName.c_str()) != -1;
        }
    }
}

Field* ColObj::getFieldFromList(FieldList& fieldList) {
    // 检查 colName 是否存在于 fieldList 中
    const int columnIndex = fieldList.GetColumnIndex(colName.c_str());
    if(columnIndex == -1) {
        return nullptr;
    } else {
        // 存在则返回
        return &(fieldList.GetColumn(columnIndex));
    }
}