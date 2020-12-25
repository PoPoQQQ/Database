//! 描述了一种字段的约束条件
//! 是所有约束条件的基类
#ifndef __FIELD_CONSTRAINTS_H__
#define __FIELD_CONSTRAINTS_H__
#include "../Utils/Constraints.h"
#include <string>
using namespace std;
class FieldConstraint {
public:
    enum ConstraintType {
        NONE = 0,
        NOT_NULL = 1,
		DEFAULT = 2,
		PRIMARY_KEY = 4,
		FOREIGN_KEY = 8
    };
    int type;
    char name[MAX_IDENTIFIER_LEN + 1]; // 约束名，一般出现在 ADD CONSTRAINT <name> 语句处
    
    // 从页中读取信息恢复约束数据
    // 在没有重写前默认只进行基础信息的读取
    BufType LoadConstraint(BufType b);
    // 将自身数据保存在页中
    // 在没有重写前默认只进行基础信息的保存
    BufType SaveConstraint(BufType b) const;
    string toString() const;

    FieldConstraint(const char* name="", ConstraintType type=NONE);
    FieldConstraint(const FieldConstraint&);
protected:
    // 储存约束名和类型的信息
    //* @return 储存信息后的指针位置
    BufType SaveBasic(BufType b) const;
    // 从指定位置按照格式读取数据构造
    //* @return 读取所有信息后的指针位置
    BufType LoadBasic(BufType b);
};
#endif //__FIELD_CONSTRAINTS_H__ 
