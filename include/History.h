//
// Library: Net Responsibility
// Package: Core
// Module:  History
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
// <History> is able to store all visited URLs and useful additional info



#ifndef HISTORY_H
#define HISTORY_H

#include "Poco/Timestamp.h"
#include "Poco/Data/Binding.h"
#include "Poco/Data/Extraction.h"
#include "Poco/Data/Limit.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/Connector.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/DateTimeFormatter.h"

#include <iostream>
#include <vector>

using Poco::Timestamp;
using Poco::DateTimeFormatter;
using namespace Poco::Data;
using namespace std;

struct HistoryRow
	/// A struct that contains each row of the urls table. It is actually nothing
	/// more than the URL split up in hostname and path, as well as the time,
	/// divided in time, date and datetime.
{
	public:
		string hostname;
		string path;
		Timestamp date;
		Timestamp time;
		Timestamp dateTime;
};


class History
	/// The History class is especially important to understand if you're writing
	/// a report plugin. When running Database.getHistory(), you'll get the return
	/// as a History object. It is designed to be easy to iterate through and
	/// holds every URL with some additional info about it.
	///
	/// The following is an example of how the History object may be used to
	/// iterate through:
	///    History history = _db->getHistory();
	///    while (history.hasMore()) {
	///    	_body <<history.getDateTime("%d/%m - %H:%M:%S"))
	///    			<<" -  " <<history.getUrl() <<endl;
	///    	history.next();
	///    }
	///
	/// Pay extra attention to how the History is recieved from the database,
	/// used in a while loop, and the next object is accessed by the next()
	/// method.
	///
	/// You may also access information about any row in the History object
	/// if you have its index. You can get the index of the current URL with
	/// the method getIndex().

{
	public:
		History();
		virtual ~History();
		void setRows(vector<HistoryRow, allocator<HistoryRow> > &rows);
			/// This method is used to add all the HistoryRows to History.

		void addRow(HistoryRow);
			/// Use this method to only add a single HistoryRow.

		int getIndex() const;
			/// Returns the index of the current row. The index may be used to
			/// access a row at any time, without having to use next() and
			/// previous().

		string getHostname() const;
			/// Returns the current hostname as a string.

		string getHostname(int index) const;
			/// Returns the hostname of index as a string.

		string getPath() const;
			/// Returns the current path as a string. In the URL
			/// "www.netresponsibility.com/dev/docs", "/dev/docs" is the path.

		string getPath(int index) const;
			/// Same as getPath() but for the given index.

		string getUrl() const;
			/// Returns the current URL (both hostname and path) as a string.

		string getUrl(int index) const;
			/// Returns the URL of index as a string.

		string getDateTime(string fmt) const;
			/// Returns a formatted date and time as a string. fmt is the format
			/// you wish to return. See Poco::DateTimeFormatter::format() for
			/// more information on exactly how the format may be written.

		string getDateTime(string fmt, int index) const;

		Timestamp getDate() const;
			/// Get the current date as a Timestamp.

		Timestamp getDate(int index) const;
			/// Get the date of index as a Timestamp.

		Timestamp getTime() const;
			/// Get the current time as a Timestamp.

		Timestamp getTime(int index) const;
			/// Get the time of index as a Timestamp.

		Timestamp getDateTime() const;
			/// Get the current date and time as a Timestamp.

		Timestamp getDateTime(int index) const;
			/// Get the date and time of index as a Timestamp.

		bool hasMore() const;
			/// Returns true if there are any more rows to iterate through. Very
			/// useful to put inside a while condition e.g.

		void previous();
			/// Go to the previous row.

		void next();
			/// Go to the next row. Useful to put in the end of a while loop.
			// Also add the ++ and -- operators to History

	protected:
		vector<HistoryRow, allocator<HistoryRow> > _historyRows;
		int _index;
};


namespace Poco {
namespace Data {

template <>
class TypeHandler<class HistoryRow>
{
public:
	static size_t size()
	{
		return 4; // we handle four columns of the Table!
	}

	static void bind(size_t pos, const HistoryRow& obj, AbstractBinder* pBinder, AbstractBinder::Direction dir)
	{
		poco_assert_dbg (pBinder != 0);
		TypeHandler<string>::bind(pos++, obj.hostname, pBinder, dir);
		TypeHandler<string>::bind(pos++, obj.path, pBinder, dir);
		TypeHandler<Int64>::bind(pos++, obj.date.epochTime(), pBinder, dir);
		TypeHandler<Int64>::bind(pos++, obj.time.epochTime(), pBinder, dir);
	}

	static void prepare(size_t pos, const HistoryRow& obj,
			AbstractPreparator::Ptr pPrepare)
	{
		poco_assert_dbg (!pPrepare.isNull());

		TypeHandler<string>::prepare(pos++, obj.hostname, pPrepare);
		TypeHandler<string>::prepare(pos++, obj.path, pPrepare);
		TypeHandler<Int64>::prepare(pos++, obj.date.epochTime(), pPrepare);
		TypeHandler<Int64>::prepare(pos++, obj.time.epochTime(), pPrepare);
	}

	static void extract(size_t pos, HistoryRow& obj,
			const HistoryRow& defVal, AbstractExtractor* pExt)
		/// obj will contain the result, defVal contains values we should use when one column is NULL
	{
		poco_assert_dbg (pExt != 0);
		Int64 d, t, dt;
		TypeHandler<string>::extract(pos++, obj.hostname, defVal.hostname, pExt);
		TypeHandler<string>::extract(pos++, obj.path, defVal.path, pExt);
		TypeHandler<Int64>::extract(pos++, d, defVal.date.epochTime(), pExt);
		TypeHandler<Int64>::extract(pos++, t, defVal.time.epochTime(), pExt);
		dt = d + (t % 86400);
		obj.date = Timestamp::fromEpochTime(d);
		obj.time = Timestamp::fromEpochTime(t);
		obj.dateTime = Timestamp::fromEpochTime(dt);
	}
};

} } // namespace Poco::Data

#endif // HISTORY_H
