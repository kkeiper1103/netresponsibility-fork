//
// Library: Net Responsibility
// Package: Core
// Module:  ReportBase
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
// <ReportBase> sets up basic functionality for making/sending reports. Must be
// inherited by <Report>



#ifndef REPORTBASE_H
#define REPORTBASE_H

#include "Options.h"
#include "Database.h"
#include "Bypasses.h"
#include "History.h"
#include "Warnings.h"
#include "Request.h"

#include <sstream>
#include <iostream>
#include <vector>

#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/MailMessage.h"
#include "Poco/Net/MailRecipient.h"
#include "Poco/Net/SMTPClientSession.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/Application.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/URI.h"
#include "Poco/Path.h"


using Poco::Timestamp;
using Poco::DateTimeFormatter;
using Poco::Net::MailMessage;
using Poco::Net::MailRecipient;
using Poco::Net::SMTPClientSession;
using Poco::Net::StringPartSource;
using Poco::Net::FilePartSource;
using Poco::Net::NoAddressFoundException;
using Poco::Util::Application;
using Poco::Exception;
using Poco::Logger;
using Poco::Path;

class MainApplication;

class ReportBase
	/// ReportBase provides basic functionality for making and sending reports,
	/// but must be inherited by Report.
{
	public:
		ReportBase();
			/// Default constructor. Assigns some default values and logs that the
			/// creating of a report started.

		ReportBase(const ReportBase&);
			/// Copy constructor

		virtual ~ReportBase();

		string getBody() const;
			/// Returns the content of the reports body

		string getSubject() const;
			/// Returns the email's subject

		string getContentType() const;
			/// Returns what content-type the email consist of. By default it is
			/// "text/plain".

		void logFinish();
			/// Logs that the report was successfully created and sent

		void sendImprovementData();
			/// Sends improvement data to the servers. This is optional for every
			/// user.

		void install();
			/// Call this function if it is an install report

		void uninstall();
			/// Call this function if it is an uninstall report

		virtual int send(bool receiveCopy = false);
			/// Send the report. Set receiveCopy to true if you wish to send a
			/// report to the user as well as the Accountability Partners.

		virtual void sendCout();
			/// Write the report to stdout instead of mailing it. Useful for
			/// debugging but really nothing else.

		virtual void sendToFile(char* filename);
			/// Write the report to filename instead of mailing it. Useful for
			/// debugging but really nothing else.

		virtual void test();
			/// Call this function if it is an test report

		virtual void generate() = 0;
			/// Override this method in Report to generate nicely formatted reports

		virtual string name() const = 0;
			/// Override this method in Report. Should actually only return the
			/// class name, which is "Report".

	protected:
		Options *_options;
		Database *_db;
		Logger *_logger;
		stringstream _body;
			/// The body of the report. Use it to build your Report.

		vector<Path> _attachments;
			/// Attachments to the report. Each Path will be included.

		string _contentType;
			/// The content-type of the report. You may change it to "text/html" or
			/// whatever suits your purpose.

		string _subject;
			/// The subject to be shown in the subject line.

		int _reportId;

		void replaceVar(string &subject, string var, string replacement);
			/// Replace {var} with replacement in subject
};

#include "MainApplication.h"
#endif // REPORTBASE_H
