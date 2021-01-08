//! 一个 Field 名可以重复的 FieldList
//! 用于打印 select 的结果
#ifndef __SELECT_FIELDLIST_H__
#define __SELECT_FIELDLIST_H__
#include "FieldList.h"
class ColObj;
class SelectFieldList: public FieldList {
public:
    /**
     * 在不考虑重复的 field name 的情况下添加 field
     * 只用于处理 select 中不同表相同列名的情况
     * */
    void AddSelectField(const ColObj& colObj, const Field& field);
    vector<ColObj> cols; // 用于储存与 fields 对应的 col 对象
    bool isUnique = true; // 一个变量来表示当前 SelectFieldList 中的 col 对象是否有重复的

    void PrintFields() { FieldList::PrintFields(); }
	void DescFields() const { FieldList::DescFields(); }
	void PrintDatas(unsigned int bitMap) { FieldList::PrintDatas(bitMap); }
    int GetColumnIndex(const ColObj&);
	Field& GetColumn(int index) {return FieldList::GetColumn(index); }
// 隐藏部分函数，它们可能根本就不能正常工作
private:
    void LoadFields(BufType b) { FieldList::LoadFields(b); }
	void SaveFields(BufType b) const { FieldList::SaveFields(b); }
	void LoadDatas(unsigned char* b) { FieldList::LoadDatas(b); }
	// 将 FieldList 中储存的所有数据保存到 buffer 中（按 Byte 进行储存）
	void SaveDatas(unsigned char* b) const { FieldList::SaveDatas(b); }

	void AddField(const Field& field) { FieldList::AddField(field); }
	void AddFieldDescVec(const char* tbName, const vector<FieldDesc>& field_desc_vec) { FieldList::AddFieldDescVec(tbName, field_desc_vec); }

	int FieldCount() const { return FieldList::FieldCount(); }
	int RoundedDataSize() const {return FieldList::RoundedDataSize(); }
	const Field& GetColumn(int index) const { return FieldList::GetColumn(index); }
	int GetColumnIndex(const char* columnName) const { return FieldList::GetColumnIndex(columnName); }
};
#endif // __SELECT_FIELDLIST_H__