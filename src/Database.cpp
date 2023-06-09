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



#include "Database.h"

using namespace Poco::Data::Keywords;

Database::Database() {}



Database::Database(Options &options)
{
	_logger = &Application::instance().logger();
	_logger->information("Connecting to database");
	_logger->debug("Database file: " + options.getDatabasefile());

	const int FINISHED = 20;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			SQLite::Connector::registerConnector();
			_session = new Session("SQLite", options.getDatabasefile());
			*_session <<"CREATE TABLE IF NOT EXISTS urls "
					<<"(hostname TEXT, path TEXT, date DATE, time TIME)", now;
			*_session <<"CREATE TABLE IF NOT EXISTS warnings "
					<<"(urlId INT, boldUrl TEXT, abbrUrl TEXT, strength INT, "
					<<"whitelist INT)", now;
			*_session <<"CREATE TABLE IF NOT EXISTS matches "
					<<"(urlId INT, keyword TEXT, category TEXT, strength INT)", now;
			*_session <<"CREATE TABLE IF NOT EXISTS reports "
					<<"(type INT, date DATE, time TIME, completed INT)", now;
			*_session <<"CREATE TABLE IF NOT EXISTS bypasses "
					<<"(type INT, date DATE, time TIME, details TEXT)", now;
			*_session <<"CREATE TABLE IF NOT EXISTS sessions "
					<<"(boot DATETIME, start DATETIME, stop DATETIME)", now;
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to load database");
				Thread::sleep(100);
			}
			else
				_logger->warning("Database locked, couldn't load database");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}

	setStatements();
	logInitBypasses(options.getInitBypasses());
	_bootHistory = new BootHistory();
	processPreviousSessions();
}



Database::~Database()
{
	SQLite::Connector::unregisterConnector();
}



void Database::setStatements() {
	_getLastRowId = new Statement(*_session);
	_logUrlStatement = new Statement(*_session);
	_logWarningStatement = new Statement(*_session);
	_logMatchStatement = new Statement(*_session);
	*_getLastRowId <<"SELECT last_insert_rowid()", into(_lastRowId);
	*_logUrlStatement <<"INSERT INTO urls VALUES "
			<<"(:hostname, :path, date(:date, 'unixepoch', 'localtime'), "
			<<"time(:time, 'unixepoch', 'localtime'))",
			use(_hostname), use(_path), use(_date), use(_time);
	*_logWarningStatement <<"INSERT INTO warnings VALUES "
			<<"(:urlId, :boldUrl, :abbrUrl, :strength, :whitelist)",
			use(_lastRowId), use(_blacklistMatch);
	*_logMatchStatement <<"INSERT INTO matches VALUES "
			<<"(:urlId, :keyword, :category, :strength)",
			use(_lastRowId), use(_keyword), use(_category), use(_strength);
}


void Database::logUrl(HTTPRequest& request)
{
	const int FINISHED = 10;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			_hostname = request.getHost();
			_path = request.getURI();
			_timestamp.update();
			_date = _time = _timestamp.epochTime();
			_logUrlStatement->execute();
			_getLastRowId->execute();
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log URL");
				Thread::sleep(i * 1000);
			}
			else
				_logger->warning("Database locked, couldn't log URL");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::logWarning(BlacklistMatch match)
{
	const int FINISHED = 10;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			_blacklistMatch = match;
			_logWarningStatement->execute();
			logMatch(match);
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log warning");
				Thread::sleep(i * 1000);
			}
			else
				_logger->warning("Database locked, couldn't log warning");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::logMatch(BlacklistMatch match) {
	const int FINISHED = 10;
	for (std::vector<BlacklistKeyword>::iterator it = match.keyword.begin();
			it != match.keyword.end(); it++)
	{
		for (int i = 0; i <= FINISHED; i++) {
			try {
				_keyword = it->asString;
				_category = it->category;
				_strength = it->strength;
				_logMatchStatement->execute();
				i = FINISHED;
			}
			catch (DBLockedException &e) {
				if (i < FINISHED) {
					_logger->debug("Database locked, will retry to log blacklist match");
					Thread::sleep(i * 1000);
				}
				else
					_logger->warning("Database locked, couldn't log blacklist match");
			}
			catch (Exception &e) {
				_logger->warning(e.displayText());
			}
		}
	}
}



void Database::logBypassShutdown(int datetime, int gap) {
	stringstream msg;
	// Put the following message in txt.xml instead
	if (gap == 0) {
		msg <<"Manually it seems";
	}
	else {
		msg <<"It was down for ";
		if (gap > 3600) {
			msg <<gap / 3600 <<" hour(s) and ";
			gap = gap % 3600;
		}
		msg <<gap / 60 <<" minute(s).";
	}

	logBypass(BYPASS_SHUTDOWN, msg.str(), datetime);
	_logger->debug("Bypass: " + msg.str());
}



void Database::logBypass(int type, string details, int datetime) {
	const int FINISHED = 10;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			if (datetime == 0) {
				*_session <<"INSERT INTO bypasses VALUES "
						<<"(:type, date('now', 'localtime')"
						<<", time('now', 'localtime'), :details)",
						use(type), use(details), now;
			}
			else {
				*_session <<"INSERT INTO bypasses VALUES "
						<<"(:type, date(:datetime, 'unixepoch', 'localtime')"
						<<", time(:datetime, 'unixepoch', 'localtime'), :details)",
						use(type), use(datetime), use(details), now;

			}

			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log bypass");
				Thread::sleep(i * 1000);
			}
			else
				_logger->warning("Database locked, couldn't log bypass");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::logInitBypasses(Bypasses &bypasses) {
	const std::vector<BypassRow> rows = bypasses.getRows();
    int type = Poco::Util::Application::instance().config().getInt("bypass", 0);

	if (rows.size() > 0) {
		const int FINISHED = 10;
		for (int i = 0; i <= FINISHED; i++) {
			try {
				*_session <<"INSERT INTO bypasses VALUES (:type, "
						<<"date(:date, 'unixepoch', 'localtime'), "
						<<"time(:time, 'unixepoch', 'localtime'), "
						<<":details)", use(type), now;
				i = FINISHED;
			}
			catch (DBLockedException &e) {
				if (i < FINISHED) {
					_logger->debug("Database locked, will retry to log init bypasses");
					Thread::sleep(i * 1000);
				}
				else
					_logger->warning("Database locked, couldn't log init bypasses");
			}
			catch (Exception &e) {
				_logger->warning(e.displayText());
			}
		}
	}
	bypasses.clear();
}



void Database::logReportStart(int &id) {
	int type = Poco::Util::Application::instance().config().getInt("report", 0);
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"INSERT INTO reports VALUES "
					<<"(:type, date('now', 'localtime'), time('now', 'localtime'), 0)",
					use(type), now;
			id = getLastRowId();
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log report start");
				Thread::sleep(200);
			}
			else {
				_logger->warning("Database locked, couldn't log report start");
				e.rethrow();
			}
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::logReportFinish(int id) {
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"UPDATE reports SET completed=1 WHERE rowId=:id",
							use(id), now;
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log report finish");
				Thread::sleep(200);
			}
			else {
				_logger->warning("Database locked, couldn't log report finish");
				e.rethrow();
			}
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::logSessionStart() {
	int boot = _bootHistory->getBootTime();
	 // If the boot time couldn't be found in BootHistory, skip logging at all
	 // Later on we might decide to report it, but for now we don't
	if (boot == 0) {
		_sessionRowId = -1;
		return;
	}
	_timestamp.update();
	int start = _timestamp.epochTime(),
		prevRowid,
		prevBoot,
		prevStop;
	const int FINISHED = 30,
		ALLOWED_GAP = 60;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			// First try to load existing entries with the current bootTime.
			// If stop is also found, check the length of the gap,
			// otherwise report that the previous session was killed.
			*_session <<"SELECT rowid, strftime('%s', boot, 'utc'), "
					<<"strftime('%s', stop, 'utc') FROM sessions "
					<<"WHERE strftime('%s', boot, 'utc') = '" <<boot <<"' "
					<<"ORDER BY boot DESC",
					into(prevRowid), into(prevBoot), into(prevStop, -1), limit(1), now;

			// Check if a session is found
			if (prevBoot == boot) {
				// Check if previous instance was killed
				int gap = start - prevStop;
				if (prevStop == -1) {
					logBypassShutdown(start);
				}
				// Otherwise if the gap between instances was a little too long
				else if (gap > ALLOWED_GAP) {
					logBypassShutdown(prevStop, gap);
				}
				*_session <<"UPDATE sessions SET stop = null "
						<<"WHERE rowid = :prevRowid", use(prevRowid), now;
				_sessionRowId = prevRowid;
			}
			else {
				// Insert a new entry into sessions
				*_session <<"INSERT INTO sessions (boot, start) VALUES "
						<<"(datetime(:boot, 'unixepoch', 'localtime'), "
						<<"datetime(:start, 'unixepoch', 'localtime'))",
						use(boot), use(start), now;
							// Do we really need to log the pid and stop?

				_getLastRowId->execute();
				_sessionRowId = _lastRowId;
			}

			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log sniffer start");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't log sniffer start");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::logSessionStop() {
	if (_sessionRowId == -1)
		return;
	_timestamp.update();
	int stop = _timestamp.epochTime();
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"UPDATE sessions SET stop="
					<<"datetime(:stop, 'unixepoch', 'localtime') WHERE rowid = "
					<<_sessionRowId, use(stop), now;

			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to log sniffer stop");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't log sniffer stop");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::processPreviousSessions() {
	const int FINISHED = 20;
	const int ALLOWED_GAP = 180;	// Allowed gap right after boot and right before halt
	for (int i = 0; i <= FINISHED; i++) {
		try {
			std::vector< Tuple<int, int, int, int> > sess;	// rowid, boot, start, stop
			set<int> logged;
			*_session <<"SELECT rowid, strftime('%s', boot, 'utc'), "
					<<"strftime('%s', start, 'utc'), "
					<<"strftime('%s', stop, 'utc') FROM sessions ",
					into(sess), now;
			for (std::vector< Tuple<int, int, int, int> >::const_iterator it = sess.begin();
					it != sess.end(); it++)
			{
				int boot  = it->get<1>();
				logged.insert(boot);
				int halt  = _bootHistory->getHaltTime(boot);
				if (halt > 0) {
					int rowid = it->get<0>();
					int start = it->get<2>();
					int stop  = it->get<3>();

					// Check if the start of Net Responsibility was delayed
					int gap = start - boot;
					if (gap > ALLOWED_GAP)
						logBypassShutdown(boot, gap);

					// Check if NR stopped too early
					gap = halt - stop;
					if (stop <= 0)	// Is this a proper way to find out if stop is null?
						logBypassShutdown(stop);
					else if (gap > ALLOWED_GAP)
						logBypassShutdown(stop, gap);


					*_session <<"DELETE FROM sessions WHERE rowid = :rowid",
							use(rowid), now;
				}
				else if (boot != _bootHistory->getBootTime()) {
					// The computer halted improperly, no need to report that.
					int rowid = it->get<0>();
					*_session <<"DELETE FROM sessions WHERE rowid = :rowid",
							use(rowid), now;
				}
			}

			// Determine if NR have been omitted in earlier sessions
			map<int, int> omitted = _bootHistory->getOmittedSessions(logged);
			map<int, int>::iterator it;

			for (it = omitted.begin(); it != omitted.end(); it++) {
				int gap = it->second - it->first;
				if (gap > ALLOWED_GAP)
					logBypassShutdown(it->first, gap);
			}

			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to process previous sessions");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't process previous sessions");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



void Database::rotateLog(int reportId) {
	int datetime;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT strftime('%s', date) + (strftime('%s', time) % 86400) "
					<<"FROM reports WHERE rowid=:id", use(reportId), into(datetime), now;
			string condition = " WHERE strftime('%s', date) +"
					" (strftime('%s', time) % 86400) < :datetime";
			*_session <<"DELETE FROM matches WHERE urlId IN (SELECT rowid FROM urls"
					<<condition <<")", use(datetime), now;
			*_session <<"DELETE FROM warnings WHERE urlId IN (SELECT rowid FROM urls"
					<<condition <<")", use(datetime), now;
			*_session <<"DELETE FROM urls" <<condition, use(datetime), now;
			*_session <<"DELETE FROM bypasses" <<condition, use(datetime), now;

			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to rotate the log");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't rotate the log");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
}



bool Database::isReportTime(int frequency) {
	if (frequency <= 0)
		return false;

	int count = -1;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT COUNT() FROM reports "
					<<"WHERE date > date('now', '-" <<frequency <<" days', 'localtime')"
					<<" AND completed = 1", into(count), now;
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to determine report time");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't determine report time");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
	return (count == 0);
}



int Database::getCount(string from) const {
	int count = 0;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT COUNT() FROM " <<from, into(count), now;
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to get count");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't get count");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
	return count;
}




int Database::getLastRowId() const {
	int rowid;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT last_insert_rowid()", into(rowid), now;
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to get lastrowid");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't get lastrowid");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}

	return rowid;
}



std::vector<string> Database::getDistinctHostnames(string where, string orderBy) const {
	std::vector<string> hostnames;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT DISTINCT hostname FROM urls WHERE " <<where
					<<" ORDER BY " <<orderBy, into(hostnames), now;
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to get hostnames");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't get hostnames");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
	return hostnames;
}



Bypasses Database::getBypasses(string where, string orderBy) const {
	if (where != "")
		where = " WHERE " + where;
	std::vector<BypassRow> rows;
	Bypasses bypasses;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT type, strftime('%s', date),"
					<<" strftime('%s', time), details FROM bypasses"
					<<where <<" ORDER BY " <<orderBy, into(rows), now;
			bypasses.setRows(rows);
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to get bypasses");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't get bypasses");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
	return bypasses;
}



History Database::getHistory(string where, string orderBy) const {
	std::vector<HistoryRow> rows;
	History history;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT hostname, path, "
					<<"strftime('%s', date), strftime('%s', time) FROM urls WHERE "
					<<where <<" ORDER BY " <<orderBy, into(rows), now;
			history.setRows(rows);
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to get history");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't get history");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
	return history;
}



Warnings Database::getWarnings(string where, string orderBy, bool whitelist) const {
	if (where != "")
		where += " AND ";
	if (whitelist)
		where += "w.whitelist = 1";
	else
		where += "w.whitelist = 0";
	std::vector<HistoryRow> historyRows;
	std::vector<BlacklistMatch> blacklistMatches;
	Warnings warnings;
	std::vector<int> urlId;
	const int FINISHED = 30;
	for (int i = 0; i <= FINISHED; i++) {
		try {
			*_session <<"SELECT w.urlId, u.hostname, u.path, "
					<<"strftime('%s', u.date), strftime('%s', u.time), "
					<<"w.boldUrl, w.abbrUrl, w.strength, w.whitelist "
					<<"FROM warnings AS w JOIN urls AS u ON w.urlId = u.rowid WHERE "
					<<where <<" ORDER BY " <<orderBy,
					into(urlId), into(historyRows), into(blacklistMatches), now;
			getMatches(urlId, blacklistMatches);
			warnings.setRows(historyRows, blacklistMatches);
			i = FINISHED;
		}
		catch (DBLockedException &e) {
			if (i < FINISHED) {
				_logger->debug("Database locked, will retry to get warnings");
				Thread::sleep(200);
			}
			else
				_logger->warning("Database locked, couldn't get warnings");
		}
		catch (Exception &e) {
			_logger->warning(e.displayText());
		}
	}
	try {
	}
	catch (Exception &err) {
		_logger->warning(err.displayText());
	}
	return warnings;
}



Warnings Database::getWhitelist(string where, string orderBy) const {
	return getWarnings(where, orderBy, true);
}



void Database::getMatches(std::vector<int> urlId, std::vector<BlacklistMatch> &bm) const {
	std::vector<BlacklistKeyword> keywords;
	for (int i = 0; i < urlId.size(); i++) {
		const int FINISHED = 30;
		for (int j = 0; j <= FINISHED; j++) {
			try {
				keywords.clear();
				*_session <<"SELECT keyword, category, strength "
						<<"FROM matches WHERE urlId = :urlId",
						into(keywords), use(urlId[i]), now;
				bm[i].keyword = keywords;
				j = FINISHED;
			}
			catch (DBLockedException &e) {
				if (j < FINISHED) {
					_logger->debug("Database locked, will retry to get matches");
					Thread::sleep(200);
				}
				else
					_logger->warning("Database locked, couldn't get matches");
			}
			catch (Exception &e) {
				_logger->warning(e.displayText());
			}
		}
	}
}
