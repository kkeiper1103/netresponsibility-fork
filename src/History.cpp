// This file is part of Net Responsibility.
//
// Net Responsibility is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Net Responsibility is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Net Responsibility.  If not, see <http://www.gnu.org/licenses/>.
//
// <History> is able to store all visited URLs and useful additional info



#include "History.h"



History::History() {
	_index = 0;
}



History::~History() {
	//dtor
}



void History::setRows(vector<HistoryRow, allocator<HistoryRow> > &rows) {
	_historyRows = rows;
}



void History::addRow(HistoryRow row) {
	_historyRows.push_back(row);
}



int History::getIndex() const {
	return _index;
}



string History::getHostname() const {
	return getHostname(_index);
}



string History::getHostname(int index) const {
	return _historyRows[index].hostname;
}



string History::getPath() const {
	return getPath(_index);
}




string History::getPath(int index) const {
	return _historyRows[index].path;
}



string History::getUrl() const {
	return getUrl(_index);
}




string History::getUrl(int index) const {
	return _historyRows[index].hostname + _historyRows[index].path;
}



Timestamp History::getDate() const {
	return getDate(_index);
}




Timestamp History::getDate(int index) const {
	return _historyRows[index].date;
}



Timestamp History::getTime() const {
	return getTime(_index);
}



Timestamp History::getTime(int index) const {
	return _historyRows[index].time;
}



Timestamp History::getDateTime() const {
	return getDateTime(_index);
}



Timestamp History::getDateTime(int index) const {
	return _historyRows[index].dateTime;
}



string History::getDateTime(string fmt) const {
	return getDateTime(fmt, _index);
}



string History::getDateTime(string fmt, int index) const {
	return DateTimeFormatter::format(_historyRows[index].dateTime, fmt);
}



bool History::hasMore() const {
	return _index < (int)_historyRows.size();
}



void History::previous() {
	_index--;
}



void History::next() {
	_index++;
}
