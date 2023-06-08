//
// Library: Net Responsibility
// Package: Core
// Module:  Options
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
// <Options> stores the configurations and other useful variables



#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>

#include "MyXml.h"
#include "Request.h"

#include "Poco/Exception.h"
#include "Poco/Environment.h"
#include "Poco/Util/Application.h"
#include "Poco/Logger.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif

#define STRINGIFY(path, file) #path #file
#define CONCAT(x, y) STRINGIFY(x, y)

#ifndef DATABASEDIR
#define DATABASEDIR /var/log
#endif

#ifndef PKGDATADIR
#define PKGDATADIR /opt/netresponsibility
#endif

#ifndef PKGLIBDIR
#define PKGLIBDIR /opt/netresponsibility
#endif

#ifndef PIDDIR
#define PIDDIR /var/run
#endif

#define DATABASEFILE CONCAT(DATABASEDIR, /net-responsibility.db)
#define PIDFILE CONCAT(PIDDIR, /net-responsibility.pid)
#define CONFIGFILE CONCAT(PKGDATADIR, /config.xml)
#define BLACKLISTFILE CONCAT(PKGDATADIR, /blacklist.xml)
#define TXTFILE CONCAT(PKGDATADIR, /txt.xml)
#define REPORT_DIR CONCAT(PKGDATADIR, /reports/)
#define REPORT_MODULE CONCAT(PKGLIBDIR, /report.so)	//Make cross-platform
#define SERVER "www.netresponsibility.com"

#ifndef VERSION
#define VERSION "0"
#endif //VERSION

using Poco::Logger;
using Poco::Util::Application;
using namespace std;

class MainApplication;
class Bypasses;

class Options
	/// Poco::Util::ServerApplication is already handling configurations and
	/// arguments, but it's way too flexible for us. This is where the Options
	/// class comes in. It processes the config and txt files, and stores all
	/// usable values in class members.
{
	public:
		Options();

		Options(int argc, char* argv[]);
			/// Deprecated, use the default contructor Options() instead.

		virtual ~Options();

		bool doSaveHistory() const;

		bool doSendImprovementData() const;
			/// Returns true if we're supposed to send the improvement data to
			/// the server.

		bool doCompressAttachedReport() const;
			/// Returns true if the report should be zipped.

		bool isReportPart(string part) const;
			/// Returns true if the given part should be included in the regular report.

		bool isAttachedReportPart(string part) const;
			/// Returns true if the given part should be included in the attached report.

		string getName() const;
			/// Returns the name of the user. This name will be used as sender of
			/// the reports.

		string getUsername() const;
			/// Returns the username, used to communicate with the server.

		string getEmailFrom() const;
			/// Returns the user's email address. This will be specified as reply-to
			/// in the report header, so that accountability partners will reply
			/// directly to the user instead of report@netresponsibility.com.

		string getConfigfile() const;
			/// The path to the local config file.

		string getServer() const;
			/// The server, usually www.netresponsibility.com

		string getMacAddress() const;
			/// Returns the computer's MAC Address. This is used when communicating
			/// with the server.

		string getVersion() const;
			/// The current version of Net Responsibility.

		string getDatabasefile() const;
			/// The path to the database.

		string getPidfile() const;
			/// The path to the pidfile.

		string getBlacklistFile() const;
			/// The path to the local blacklist file.

		string getTxt(string) const;
			/// The path to the local txt file.

		string getReportModule() const;
			/// The path to the report module.

		vector<string> getEmailTo() const;
			/// Returns all emails to send reports to.

		vector<string> getReportParts() const;
			/// Returns the report parts.

		vector<string> getAttachedReportParts() const;
			/// Returns the attached report parts.

		int getReportFrequency() const;
			/// Returns the number of days between scheduled reports.

		int getReportStrengthThreshold() const;
			/// Returns the report strength threshold.

		Bypasses &getInitBypasses() const;

		void setUsername(string);

		void loadConfigfile();
			/// Load all options from the local configfile, or download a new one
			/// if it's corrupt.

	private:
		string _databasefile;
		string _pidfile;
		string _configfile;
		string _txtfile;
		string _username;
		string _name;
		string _emailFrom;
		string _blacklistFile;
		string _version;
		string _server;
		string _reportModule;
		bool _saveHistory;
		bool _sendImprovementData;
		bool _compressAttachedReport;
		vector<string> _reportParts;
		vector<string> _attachedReportParts;
		vector<string> _emailTo;
		map<string, string> _txt;
		int _reportFrequency;
		int _reportStrengthThreshold;
		Logger *_logger;
		Bypasses *_initBypasses;

		void loadDefaultValues();
		void loadTxt();
};

#include "Bypasses.h"
#endif // OPTIONS_H
