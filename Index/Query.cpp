#include "Query.h"
using namespace std;
Query::Query() {}
Query::Query(const vector<string>& colNames, const vector<Data>& keyTypes): 
	colNames(colNames), invalidQuery(false), coverRate(0.f) {
	for(vector<Data>::const_iterator it = keyTypes.begin(); it != keyTypes.end(); it++) {
		pair<Data, Data> interval(*it, *it);
		interval.first.SetNegInf();
		interval.second.SetPosInf();
		intervals.push_back(interval);
	}
}
void Query::GetIntervals(WhereCondition& whereCondition) {
	if(whereCondition.type == WhereCondition::CondType::COMBINDED) {
		GetIntervals(*whereCondition.condition1);
		GetIntervals(*whereCondition.condition2);
		return;
	}
	if(whereCondition.type == WhereCondition::CondType::UNDEFINED)
		throw "Where condition error!";
	if(whereCondition.type == WhereCondition::CondType::IS_NOT_NULL)
		return;

	string colName = whereCondition.col.colName;
	int index;
	for(index = 0; index < (signed)colNames.size(); index++)
		if(colNames[index] == colName)
			break;
	if(index == (signed)colNames.size())
		return;
	if(whereCondition.type == WhereCondition::CondType::IS_NULL) {
		Data nullData = intervals[index].first;
		nullData.SetNull();
		intervals[index].first = max(intervals[index].first, nullData);
		intervals[index].second = min(intervals[index].second, nullData);
		return;
	}
	if(whereCondition.type != WhereCondition::CondType::EXPR)
		throw "Invalid where condition!";
	if(whereCondition.expr.isCol)
		return;
	Data data = whereCondition.expr.value;
	if((data.dataType & 0xff) == Data::VARCHAR)
		data = HashData(data);
	switch(whereCondition.op) {
		case OpEnum::EQUAL:
			intervals[index].first = max(intervals[index].first, data);
			intervals[index].second = min(intervals[index].second, data);
			break;
		case OpEnum::NOTEQUAL:
			break;
		case OpEnum::LESS:
		case OpEnum::LEQUAL:
			intervals[index].second = min(intervals[index].second, data);
			break;
		case OpEnum::GREATER:
		case OpEnum::GEQUAL:
			intervals[index].first = max(intervals[index].first, data);
			break;
		case OpEnum::NONE:
			throw "Error: WhereCondition has NONE op in GetIntervals";
		default:
			throw "Error: undefined op type in GetIntervals";
	}
}
void Query::SetUpQuery(WhereCondition& whereCondition) {
	GetIntervals(whereCondition);
	for(vector<pair<Data, Data>>::iterator it = intervals.begin(); it != intervals.end(); it++)
		if(it->second < it->first) {
			invalidQuery = true;
			return;
		}
	for(vector<pair<Data, Data>>::iterator it = intervals.begin(); it != intervals.end(); it++) {
		if(it->first == it->second)
			coverRate += 1.0f;
		else {
			Data negInf = it->first;
			Data posInf = it->second;
			negInf.SetNegInf();
			posInf.SetPosInf();
			if(!(it->first == negInf) || !(it->second == posInf))
				coverRate += 0.5f;
			break;
		}
	}
}
vector<Data> Query::LowerBound() {
	vector<Data> ret;
	for(vector<pair<Data, Data>>::iterator it = intervals.begin(); it != intervals.end(); it++)
		ret.push_back(it->first);
	return ret;
}
vector<Data> Query::UpperBound() {
	vector<Data> ret;
	for(vector<pair<Data, Data>>::iterator it = intervals.begin(); it != intervals.end(); it++)
		ret.push_back(it->second);
	return ret;
}
