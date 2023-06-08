//
// Library: Net Responsibility
// Package: Core
// Module:  Bypasses
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
// <Bypasses> is able to store all useful information about attempts to bypass
// the software

#ifndef BYPASSES_H
#define BYPASSES_H

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

class Database;

enum BypassType
	/// What kind of bypass is it?
{
	BYPASS_UNKNOWN,
	BYPASS_SHUTDOWN,
	BYPASS_MISSING_FILE,
	BYPASS_MODIFIED_FILE
};

struct BypassRow
	/// A simple class to contain each Bypass.
{
	public:
		int type;
			/// Type of the bypass, given as BypassType.

		Timestamp date;
		Timestamp time;
		Timestamp dateTime;
		string details;
			/// Additional details about the bypass. Could be which file that was
			/// deleted, how Net Responsibility was shutdown etc.
};

class Bypasses
	/// Bypasses is able to store all useful information about attempts to bypass
	/// the software. All Bypasses will show up in the bypass section in the report.
	///
	/// Bypasses is iterated through in a similar manner as History.
{
	public:
		Bypasses();
		virtual ~Bypasses();
		void setRows(vector<BypassRow, allocator<BypassRow> > &rows);
		void addRow(BypassRow);
		void addRow(int type, string details = "");
			/// Add a row by giving its BypassType and details.

		int getIndex() const;
			/// Returns the index of the current BypassRow. This may be used to
			/// access this specific element later.

		int getType() const;
			/// Returns the type given as BypassType.

		int getType(int index) const;
		int size() const;
		string getDateTime(string fmt) const;
			/// Returns a DateTime string formatted as specified in fmt.

		string getDateTime(string fmt, int index) const;
		string getDetails() const;
		string getDetails(int index) const;
		string getTypeString() const;
			/// Returns a message that tries to explain what type of bypass it is.
			/// These messages are loaded from the txtfile.

		string getTypeString(int index) const;
		Timestamp getDate() const;
		Timestamp getDate(int index) const;
		Timestamp getTime() const;
		Timestamp getTime(int index) const;
		Timestamp getDateTime() const;
		Timestamp getDateTime(int index) const;
		vector<BypassRow> getRows() const;
			/// Get all _bypassRows inside a vector.

		bool hasMore() const;
			/// Returns true if Bypasses contains more elements to iterate through.

		void previous();
		void next();
		void clear();
			/// Clear out all _bypassRows.

	protected:
		vector<BypassRow, allocator<BypassRow> > _bypassRows;
			/// All BypassRows stored in a vector.

		int _index;
			/// The current index.
};



namespace Poco {
namespace Data {

template <>
class TypeHandler<class BypassRow>
{
public:
	static size_t size()
	{
		return 4; // we handle four columns of the Table!
	}

	static void bind(size_t pos, const BypassRow& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
	{
		poco_assert_dbg (!pBinder.isNull());

		TypeHandler<int>::bind(pos++, obj.type, pBinder, dir);
		TypeHandler<Poco::Int64>::bind(pos++, obj.date.epochTime(), pBinder, dir);
		TypeHandler<Poco::Int64>::bind(pos++, obj.time.epochTime(), pBinder, dir);
		TypeHandler<string>::bind(pos++, obj.details, pBinder, dir);
	}

	static void prepare(size_t pos, const BypassRow& obj,
			AbstractPreparator::Ptr pPrepare)
	{
		poco_assert_dbg (!pPrepare.isNull());

		TypeHandler<int>::prepare(pos++, obj.type, pPrepare);
		TypeHandler<Poco::Int64>::prepare(pos++, obj.date.epochTime(), pPrepare);
		TypeHandler<Poco::Int64>::prepare(pos++, obj.time.epochTime(), pPrepare);
		TypeHandler<string>::prepare(pos++, obj.details, pPrepare);
	}

	static void extract(size_t pos, BypassRow& obj,
			const BypassRow& defVal, AbstractExtractor* pExt)
		/// obj will contain the result, defVal contains values we should use when one column is NULL
	{
		poco_assert_dbg (pExt != 0);
		Int64 d, t, dt;
		TypeHandler<int>::extract(pos++, obj.type, defVal.type, pExt);
		TypeHandler<Int64>::extract(pos++, d, defVal.date.epochTime(), pExt);
		TypeHandler<Int64>::extract(pos++, t, defVal.time.epochTime(), pExt);
		TypeHandler<string>::extract(pos++, obj.details, defVal.details, pExt);
		dt = d + (t % 86400);
		obj.date = Timestamp::fromEpochTime(d);
		obj.time = Timestamp::fromEpochTime(t);
		obj.dateTime = Timestamp::fromEpochTime(dt);
	}
};

} } // namespace Poco::Data

#include "Options.h"
#include "MainApplication.h"
#endif // BYPASSES_H
