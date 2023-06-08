#include "Bypasses.h"

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
// <Bypasses> is able to store all useful information about attempts to bypass
// the software


Bypasses::Bypasses() {
	_index = 0;
}



Bypasses::~Bypasses() {
	//dtor
}



void Bypasses::setRows(vector<BypassRow, allocator<BypassRow> > &rows) {
	_bypassRows = rows;
}



void Bypasses::addRow(BypassRow row) {
	_bypassRows.push_back(row);
}



void Bypasses::addRow(int type, string details) {
	BypassRow row;
	row.type = type;
	row.details = details;
	row.date.update();
	row.time.update();
	_bypassRows.push_back(row);
}



int Bypasses::getIndex() const {
	return _index;
}



int Bypasses::getType() const {
	return getType(_index);
}



int Bypasses::getType(int index) const {
	return _bypassRows[index].type;
}



int Bypasses::size() const {
	return _bypassRows.size();
}



string Bypasses::getDateTime(string fmt) const {
	return getDateTime(fmt, _index);
}



string Bypasses::getDateTime(string fmt, int index) const {
	return DateTimeFormatter::format(_bypassRows[index].dateTime, fmt);
}



string Bypasses::getDetails() const {
	return getDetails(_index);
}




string Bypasses::getDetails(int index) const {
	return _bypassRows[index].details;
}



string Bypasses::getTypeString() const {
	return getTypeString(_index);
}



string Bypasses::getTypeString(int index) const {
	Options *options = &MainApplication::getOptions();
	switch(_bypassRows[index].type) {
		case BYPASS_SHUTDOWN:
			return options->getTxt("bypassShutdown");
		case BYPASS_MISSING_FILE:
			return options->getTxt("bypassMissingFile");
		case BYPASS_MODIFIED_FILE:
			return options->getTxt("bypassModifiedFile");
		default:
			return options->getTxt("bypassUnknown");
	}
}



Timestamp Bypasses::getDate() const {
	return getDate(_index);
}




Timestamp Bypasses::getDate(int index) const {
	return _bypassRows[index].date;
}



Timestamp Bypasses::getTime() const {
	return getTime(_index);
}



Timestamp Bypasses::getTime(int index) const {
	return _bypassRows[index].time;
}



Timestamp Bypasses::getDateTime() const {
	return getDateTime(_index);
}



Timestamp Bypasses::getDateTime(int index) const {
	return _bypassRows[index].dateTime;
}



vector<BypassRow> Bypasses::getRows() const {
	return _bypassRows;
}



bool Bypasses::hasMore() const {
	return _index < (int)_bypassRows.size();
}



void Bypasses::previous() {
	_index--;
}



void Bypasses::next() {
	_index++;
}



void Bypasses::clear() {
	_bypassRows.clear();
}
