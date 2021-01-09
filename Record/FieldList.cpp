#include <cstring>
#include <iostream>
#include "FieldList.h"
#include "../Utils/Constraints.h"
using namespace std;

FieldList::FieldList(const FieldList& other)
	:fields(other.fields),pkConstraints(other.pkConstraints){

}

FieldList::~FieldList() {}

void FieldList::LoadFields(BufType b) {
	unsigned int size = b[0];
	b += 1;

	fields.resize(size);
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++) {
		it->Load(b);
		b += it->FieldSize() >> 2;
	}

	size = b[0];
	b += 1;

	pkConstraints.resize(size);
	for(vector<PrimaryKeyCstrnt>::iterator it = pkConstraints.begin(); it != pkConstraints.end(); it++) {
		it->LoadConstraint(b);
		b += it->GetConstraintSize() >> 2;
	}

	size = b[0];
	b += 1;

	pkConstraints.resize(size);
	for(vector<ForeignKeyCstrnt>::iterator it = fkConstraints.begin(); it != fkConstraints.end(); it++) {
		it->LoadConstraint(b);
		b += it->GetConstraintSize() >> 2;
	}
}
void FieldList::SaveFields(BufType b) const {
	b[0] = fields.size();
	b += 1;

	for(vector<Field>::const_iterator it = fields.begin(); it != fields.end(); it++) {
		it->Save(b);
		b += it->FieldSize() >> 2;
	}

	b[0] = pkConstraints.size();
	b += 1;

	for(vector<PrimaryKeyCstrnt>::const_iterator it = pkConstraints.begin(); it != pkConstraints.end(); it++) {
		it->SaveConstraint(b);
		b += it->GetConstraintSize() >> 2;
	}

	b[0] = fkConstraints.size();
	b += 1;

	for(vector<ForeignKeyCstrnt>::const_iterator it = fkConstraints.begin(); it != fkConstraints.end(); it++) {
		it->SaveConstraint(b);
		b += it->GetConstraintSize() >> 2;
	}
}
void FieldList::LoadDatas(unsigned char* b) {
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++) {
		it->LoadData(b);
		b += it->DataSize();
	}
}
void FieldList::SaveDatas(unsigned char* b) const{
	for(vector<Field>::const_iterator it = fields.begin(); it != fields.end(); it++) {
		it->SaveData(b);
		b += it->DataSize();
	}
}

void FieldList::AddField(const Field& field) {
	if(FieldCount() >= MAX_COL_NUM){
		printf("MAX_COL_NUM = %d, FieldCount = %d", MAX_COL_NUM, FieldCount());
		throw "Too many fields!";
	}
	if(GetColumnIndex(field.columnName) >= 0) {
		cerr << field.columnName << endl;
		throw "Field already exists!";
	}
	fields.push_back(field);
}
void FieldList::AddFieldDesc(const FieldDesc& fieldDesc) {
	switch(fieldDesc.type) {
		case FieldDesc::FieldType::UNDEFINED:
			throw "Error in FieldList::AddFieldDescVec: undefined FieldType";
		case FieldDesc::FieldType::PRIMARY: {
			cerr <<  "Warning: try to add primary key by AddFieldDesc. Ignored." << endl;
			return;
		}
		case FieldDesc::FieldType::FOREIGN: {
			cerr <<  "Warning: try to add foreign key by AddFieldDesc. Ignored." << endl;
			return;
		}
		default:
			this->fields.push_back(fieldDesc.field);
			break;
	}
}
void FieldList::AddFieldDescVec(string tbName, const vector<FieldDesc>& vec) {
	for(vector<FieldDesc>::const_iterator it = vec.begin(); it != vec.end(); it++)
		switch(it->type) {
			case FieldDesc::FieldType::UNDEFINED:
				throw "Error in FieldList::AddFieldDescVec: undefined FieldType";
				break;
			case FieldDesc::FieldType::PRIMARY: {
				if(pkConstraints.size() > 0)
					// 在声明的时候只能定义一个主键，所以此时有语法错误
					throw "Error: Multiple Primary keys defined";
				// 否则记录下来所有的主键，最后进行检验
				pkConstraints.push_back(PrimaryKeyCstrnt("_pk_0", it->columnList));
				break;
			}
			case FieldDesc::FieldType::FOREIGN: {
				fkConstraints.push_back(ForeignKeyCstrnt("_fk_0", it->tbName, it->columnList, it->ref_columnList));
				break;
			}
			default:
				this->fields.push_back(it->field);
				break;
		}
	for(vector<PrimaryKeyCstrnt>::iterator it = pkConstraints.begin(); it != pkConstraints.end(); it++) {
		for(int i = 0; i < (signed)it->colNames.size(); i++) {
			string colName = it->colNames[i];
			int colIndex = GetColumnIndex(colName);
			if(colIndex == -1)
				throw "Error: Column does not exist";
			Field& field = GetColumn(colIndex);
			if((field.constraints & Field::PRIMARY_KEY) || (field.constraints & Field::FOREIGN_KEY))
				throw "Error: Multiple constraint is not supported!";
			field.constraints |= Field::PRIMARY_KEY;
		}
	}
	for(vector<ForeignKeyCstrnt>::iterator it = fkConstraints.begin(); it != fkConstraints.end(); it++) {
		for(int i = 0; i < (signed)it->colNames.size(); i++) {
			string colName = it->colNames[i];
			int colIndex = GetColumnIndex(colName);
			if(colIndex == -1)
				throw "Error: Column does not exist";
			Field& field = GetColumn(colIndex);
			if((field.constraints & Field::PRIMARY_KEY) || (field.constraints & Field::FOREIGN_KEY))
				throw "Error: Multiple constraint is not supported!";
			field.constraints |= Field::FOREIGN_KEY;
		}
	}
}

void FieldList::PrintFields() {
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		cout << " | " << it->columnName;
	cout << " | " << endl;
}
void FieldList::PrintDatas(unsigned int bitMap) {
	for(int index = 0; index < (signed)fields.size(); index++) {
		if((bitMap & (1u << index)) == 0)
			cout << " | NULL";
		else
			cout << " | " << fields[index].data;
	}
	cout << " | " << endl;
}

int FieldList::FieldCount() const {
	return fields.size();
}
int FieldList::RoundedDataSize() const {
	int dataSize = 0;
	for(vector<Field>::const_iterator it = fields.begin(); it != fields.end(); it++)
		dataSize += it->DataSize();
	return ((dataSize | 3) ^ 3) + ((dataSize & 3) ? 4 : 0);
}
Field& FieldList::GetColumn(int index) {
	if(index < 0 || index >= FieldCount())
		throw "Invalid index!";
	return fields[index];
}
const Field& FieldList::GetColumn(int index) const {
	if(index < 0 || index >= FieldCount())
		throw "Invalid index!";
	return fields[index];
}
int FieldList::GetColumnIndex(string columnName) const {
	for(int i = 0; i < (signed)fields.size(); i++)
		if(string(fields[i].columnName) == columnName)
			return i;
	return -1;
}
void FieldList::DescFields() const {
	const FieldList& fieldList = *this;
	// get max length
	int max_field_length = 5;
	int max_type_length = 4;
	int max_null_length = 4;
	int max_key_length = 3;
	int max_default_length = 7;
	// max field length
	for(vector<Field>::const_iterator it = fieldList.fields.begin(); it != fieldList.fields.end(); it++) {
		max_field_length = max(max_field_length, (signed)it->columnName.length());
		char buf[256];
		switch(it->data.dataType & 0xff) {
			case Data::INT:
				snprintf(buf, 256, "INT(%d)", it->data.dataType >> 8);
				max_type_length = max(max_type_length, (int) strlen(buf));
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%d", it->data.intData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::VARCHAR:
				snprintf(buf, 256, "VARCHAR(%d)", it->data.dataSize);
				max_type_length = max(max_type_length, (int) strlen(buf));
				if(it->constraints & Field::DEFAULT) {
					max_default_length = max(max_default_length, min((int) strlen(it->data.stringData), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::DATE:
				// data == type : 4 == 4
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%d", it->data.intData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::FLOAT:
				max_type_length = max(max_type_length, 5);
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%f", it->data.floatData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			default:
				cerr << it->data.dataType << endl;
				throw "Error: error type in DescTable";
		}
	}

	// print header
	cout << "+";
	for(int i = 0; i < max_field_length + 2; ++i)
		cout << "-";
	cout << "+";
	for(int i = 0;i < max_type_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_null_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_key_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_default_length + 2; ++i) {
		cout << "-";
	}
	cout << "+" << endl;
	
	cout << "| Field";
	for(int i = 0; i < max_field_length + 2 - 6; ++i)
		cout << " ";
	cout << "| Type";
	for(int i = 0; i < max_type_length + 2 - 5; ++i)
		cout << " ";
	cout << "| Null ";

	cout << "| Key ";

	cout << "| Default";
	for(int i = 0;i < max_default_length + 2 - 8; ++i) {
		cout << " ";
	}
	cout << "|" << endl;

	cout << "+";
	for(int i = 0; i < max_field_length + 2; ++i)
		cout << "-";
	cout << "+";
	for(int i = 0;i < max_type_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_null_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_key_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_default_length + 2; ++i) {
		cout << "-";
	}
	cout << "+" << endl;

	for(vector<Field>::const_iterator it = fieldList.fields.begin(); it != fieldList.fields.end(); it++) {
		cout << "| " << it->columnName;
		for(int i = 0; i < max_field_length + 2 - (signed)it->columnName.length() - 1; i++)
			cout << " ";
		
		cout << "| ";
		char buf[256];
		switch(it->data.dataType & 0xff) {
			case Data::INT:
				snprintf(buf, 256, "INT(%d)", it->data.dataType >> 8);
				cout << buf;
				for(int i = 0; i < max_type_length + 2 - (int) strlen(buf) - 1; i++)
					cout << " ";
				break;
			case Data::VARCHAR:
				snprintf(buf, 256, "VARCHAR(%d)", it->data.dataSize);
				cout << buf;
				for(int i = 0; i < max_type_length + 2 - (int) strlen(buf) - 1; i++)
					cout << " ";
				break;
			case Data::DATE:
				cout << "DATE";
				for(int i = 0; i < max_type_length + 2 - 4 - 1; i++)
					cout << " ";
				break;
			case Data::FLOAT:
				cout << "FLOAT";
				for(int i = 0; i < max_type_length + 2 - 5 - 1; i++)
					cout << " ";
				break;
			default:
				cout << "TYPE";
				for(int i = 0; i < max_type_length + 2 - 4 - 1; i++)
					cout << " ";
				break;
		}

		cout << "| ";	
		if(it->constraints & Field::NOT_NULL) {
			cout << "NO   ";
		} else {
			cout << "YES  ";
		}

		cout << "| ";
		if(it->constraints & Field::PRIMARY_KEY) {
			cout << "PRI ";
		} else if (it->constraints & Field::FOREIGN_KEY) {
			cout << "MUL ";
		} else {
			cout << "    ";
		}

		cout << "| ";
		if(it->constraints & Field::DEFAULT) {
			switch(it->data.dataType){
				default:
					cout << "DEFAULT ";
					break;
			}
		} else {
			cout << "NULL";
			for(int i = 0;i < max_default_length + 2 - 4 - 1; ++i) {
				cout << " ";
			}
		}
		cout << "|" << endl;
	}
	// print end line
	cout << "+";
	for(int i = 0; i < max_field_length + 2; ++i)
		cout << "-";
	cout << "+";
	for(int i = 0;i < max_type_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_null_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_key_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_default_length + 2; ++i) {
		cout << "-";
	}
	cout << "+" << endl;
}
