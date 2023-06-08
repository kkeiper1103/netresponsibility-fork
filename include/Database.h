//
// Library: Net Responsibility
// Package: Core
// Module:  Database
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
// <Database> handles all connections from and to the database.

#ifndef DATABASE_H
#define DATABASE_H

#include "Poco/Data/Binding.h"
#include "Poco/Data/Extraction.h"
#include "Poco/Data/Limit.h"
#include "Poco/Data/SessionFactory.h"


#include "Poco/Data/Connector.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/SQLiteException.h"
#include "Poco/Exception.h"
#include "Poco/Timestamp.h"
#include "Poco/Tuple.h"
#include "Poco/Logger.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Thread.h"
#include "Poco/Process.h"

#include "Blacklist.h"
#include "Options.h"
#include "History.h"
#include "Warnings.h"
#include "Bypasses.h"
#include "BootHistory.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <set>

using Poco::Timestamp;
using Poco::Tuple;
using Poco::Exception;
using Poco::Data::SQLite::DBLockedException;
using Poco::Thread;
using Poco::Net::HTTPRequest;
using namespace Poco::Data;
using namespace std;

class Database
	/// Database handles all connections to the SQLite database. The Database
	/// object is instanciated in by MainApplication, and may be accessed by the
	/// static method &MainApplication::getDatabase().
	///
	/// The only class allowed to actually store any entries in the database is
	/// Sniffer. Other classes may only load data from it.
	///
	/// There is no method to write any custom SQLite query. Instead the queries
	/// are a little more limited. Most methods that returns data from the
	/// database also takes the arguments where and orderBy. With these you may
	/// customize the return values.
	///
	/// Queries that are done repeatedly are stored inside Statements. This will
	/// enchance their speed.
{
	public:
		Database();
		Database(Options& options);
		~Database();

		bool isReportTime(int frequency);
			/// Check if it's time to send a scheduled report, depending on
			/// 'frequency'.

		int getCount(string from) const;
			/// Return the number of rows found in the table 'from'.

		int getLastRowId() const;

		vector<string> getDistinctHostnames(string where
					= "hostname <> ''", string orderBy = "hostname ASC") const;
					/// Return all unique hostnames in a simple vector. By default
					/// it will not return hostnames that are empty
					/// entries (if found), and the hostnames will be ordered by name.

		Bypasses getBypasses(string where = "", string orderBy
					= "date, time ASC") const;
					/// Return all Bypasses found, in a Bypasses object.

		History getHistory(string where = "hostname <> ''", string orderBy
					= "hostname ASC") const;
					/// Return all History.

		Warnings getWarnings(string where = "", string orderBy
					= "u.hostname ASC", bool = false) const;
					/// Return all Warnings found.

		Warnings getWhitelist(string where = "", string orderBy = "u.hostname ASC") const;
					/// Return all matches that are whitelisted for some reason. This
					/// is wrapped up in a Warnings object, since it's actually
					/// the same thing, but the matches are also considered clean.

		void logBypass(int type, string details = "", int datetime = 0);
			/// Log attempts to bypass the software. You'll need to enter
			/// the type (BYPASS_TYPE), and optionally more verbose details.
			/// The int datetime is a unix timestamp of when the bypass happened.
			/// If 0 the current timestamp will be used.

		void logInitBypasses(Bypasses& bypasses);
			/// During startup the software makes several sanity checks, to make
			/// sure it hasn't been bypassed. Even attempts that fails will be
			/// reported. This may be discussed however, if they are redundant
			/// and confusing for the accountability partners?

		void logReportStart(int& id);
			/// Log that the reporting process have started. You'll need the id
			/// later to properly "close" the report.

		void logReportFinish(int id);
			/// Log that the report finished successfully. You'll need to enter the
			/// id given by logReportStart().

		void logSessionStart();
			/// Log that this current instance started. This info is used to process
			/// if Net Responsibility has been bypassed by shutting it down or
			/// omitting it at boot time. The boot time of this specific session is
			/// also stored.

		void logSessionStop();
			/// Log that the current instance stopped.

		void rotateLog(int reportId);
			/// This method rotated the database, to clean up everything that have
			/// been included. Only entries older than the time for the report with
			/// reportId will be deleted. The reportId was given by logReportStart().

		friend class Sniffer;

	protected:
		void setStatements();

		void logUrl(HTTPRequest& request);
			/// Log a URL that's visited. This may only be done by the Sniffer class.
			// This will later be replaced by HTTPHit.

		void logWarning(BlacklistMatch match);
			/// Log a Warning that is flagged by the Filter. It is connected to
			/// the last URL inserted.

		void logMatch(BlacklistMatch match);

		void logBypassShutdown(int datetime, int gap = 0);

		void getMatches(vector<int> urlId, vector<BlacklistMatch>& bm) const;

		void processPreviousSessions();
			/// Process the previous sessions, and log any attempts to bypass NR.

	private:
		Session *_session;
		Timestamp _timestamp;
		int _lastRowId;
		int _sessionRowId;
		int _strength;
		int _date;
		int _time;
		string _hostname;
		string _path;
		string _keyword;
		string _category;
		Logger *_logger;
		BootHistory *_bootHistory;
		BlacklistMatch _blacklistMatch;
		Statement *_getLastRowId;
		Statement *_logUrlStatement;
		Statement *_logWarningStatement;
		Statement *_logMatchStatement;
};

#endif // DATABASE_H
