//
// Library: Net Responsibility
// Package: Core
// Module:  Warnings
//
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


#ifndef WARNINGS_H
#define WARNINGS_H

#include "History.h"
#include "Blacklist.h"

class Warnings: public History
	/// Warnings is able to store all inappropriate URLs and useful additional info
	/// Iterate through it as with History.
{
	public:
		Warnings();
		virtual ~Warnings();
		void setRows(vector<HistoryRow, allocator<HistoryRow> > &rows,
				vector<BlacklistMatch, allocator<BlacklistMatch> > &matches);
			/// Set HistoryRows and BlacklistMatches

		void addRow(HistoryRow, BlacklistMatch);
			/// Add one HistoryRow and BlacklistMatch

		int getStrength() const;
			/// Returns the current strength.

		int getStrength(int index) const;
			/// Returns the strength of index

		int size() const;
			/// Returns the number of Warnings found

		string getBoldUrl() const;
			/// Returns the current bold url, with matches emphasized

		string getBoldUrl(int index) const;
			/// Returns the bold url of index, with matches emphasized

		string getAbbrUrl() const;
			/// Returns the current abbreviated url, with matches emphasized

		string getAbbrUrl(int index) const;
			/// Returns the abbreviated url of index, with matches emphasized

		vector<BlacklistKeyword> getKeywords() const;
			/// Returns all current BlacklistKeywords

		vector<BlacklistKeyword> getKeywords(int index) const;
			/// Returns all BlacklistKeywords of index

	protected:
		vector<BlacklistMatch, allocator<BlacklistMatch> > _blacklistMatches;
};


#endif // WARNINGS_H
