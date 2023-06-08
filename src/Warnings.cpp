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
// <Warnings> is able to store all inappropriate URLs and useful additional info



#include "Warnings.h"

Warnings::Warnings() {
	_index = 0;
}

Warnings::~Warnings()
{
	//dtor
}



void Warnings::setRows(vector<HistoryRow, allocator<HistoryRow> > &rows,
		vector<BlacklistMatch, allocator<BlacklistMatch> > &matches)
{
	_historyRows = rows;
	_blacklistMatches = matches;
}



void Warnings::addRow(HistoryRow row, BlacklistMatch match) {
	_historyRows.push_back(row);
	_blacklistMatches.push_back(match);
}



int Warnings::getStrength() const {
	return getStrength(_index);
}



int Warnings::getStrength(int index) const {
	return _blacklistMatches[index].strength;
}



int Warnings::size() const {
	return _blacklistMatches.size();
}



string Warnings::getBoldUrl() const {
	return getBoldUrl(_index);
}



string Warnings::getBoldUrl(int index) const {
	return _blacklistMatches[index].boldUrl;
}



string Warnings::getAbbrUrl() const {
	return getAbbrUrl(_index);
}



string Warnings::getAbbrUrl(int index) const {
	return _blacklistMatches[index].abbrUrl;
}



vector<BlacklistKeyword> Warnings::getKeywords() const {
	return getKeywords(_index);
}



vector<BlacklistKeyword> Warnings::getKeywords(int index) const {
	return _blacklistMatches[index].keyword;
}
