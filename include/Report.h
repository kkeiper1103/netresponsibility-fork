//
// Library: Net Responsibility
// Package: Report
// Module:  Report
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
// <Report> generates the reports, and may override <ReportBase>'s send method.



#ifndef REPORT_H
#define REPORT_H

#include "ReportBase.h"

#include <map>

#ifdef _WIN32
  #ifdef BUILD_DLL
    #define ADDAPI __declspec(dllexport)
  #else
    #define ADDAPI __declspec(dllimport)
  #endif
#else
  #define ADDAPI
#endif

#include "Poco/RegularExpression.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/ClassLibrary.h"
#include "Poco/Zip/Compress.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/NumberFormatter.h"

using Poco::RegularExpression;
using Poco::DirectoryIterator;
using Poco::Zip::Compress;
using Poco::Path;
using Poco::File;

class Report: public ReportBase
	/// This class is generating the reports. It may also override ReportBase's
	/// send method to send the reports another way than usual. Report is written
	/// as a plugin, so one may extend Net Responsibility with different types
	/// of reports. So far this is the only existing plugin.
{
	public:
		Report(): ReportBase() {}
			/// The default constructor, only uses ReportBase's regular constructor.

		string name() const;
			/// Returns the name of the class

		void generate();
			/// Generates the report. This method invokes a number of other methods

	protected:
		/*Inherited:
		stringstream _body;
		string _subject;
		string _contentType;
		vector<Path> _attachments;
		Options *_options;
		Database *_db;
		Logger *_logger;

		*/

	private:
		stringstream _attached;
		void makeBypassesSection();
		void makeWarningsSection(string&);
		void makeWhitelistSection();
		void makeHistorySection();
		void addTemplate(string suspicious = "");
		void saveAttachedReport();
		string jsContent(string);
		string makeColoredStrength(int);
		string makeTableBranch(string, string, string anchorName = "");
		string makeJavascriptBranch(string, string);
};


POCO_BEGIN_MANIFEST(ReportBase)
POCO_EXPORT_CLASS(Report)
POCO_END_MANIFEST

#endif // REPORT_H
