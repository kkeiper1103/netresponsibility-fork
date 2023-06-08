//
// Library: Net Responsibility
// Package: Core
// Module:  Filter
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
// <Filter> determines whether the URLs are appropriate or not.

#ifndef FILTER_H
#define FILTER_H

#include "Blacklist.h"
#include "MyXml.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "Poco/RegularExpression.h"
#include "Poco/SharedPtr.h"
#include "Poco/Exception.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Util/Application.h"

using Poco::RegularExpression;
using Poco::SharedPtr;
using Poco::Net::HTTPRequest;
using Poco::Util::Application;
using namespace std;

class Options;
class Database;

class Filter
	/// This class will run all test to find out if the URLs are appropriate or
	/// not. It is done by initially loading the blacklists, and then testing
	/// each URL against them. When keywords are found within the given text,
	/// Filter thries to determine how strong the match is. This means every
	/// match will be given an individual strength to indicate how likely it is
	/// to be an inappropriate site. (The higher number, the more likely).
	///
	/// The URLs are filtered instantly, rather than at report time, as done in
	/// several previous versions.
{
	public:
		Filter();
		Filter(string blacklistFile);
			/// Load the Filter, given the path to the blacklist. This is
			/// especially useful when improvign the algorithms.

		Filter(Options* options, Database* db);
			/// Load the Filter, given both the options and database.

		bool isMatch(HTTPRequest& request, BlacklistMatch& blacklistMatch);
			/// This is the method used for running a complete scan on the URL.
			/// The URL is given as a HTTPRequest, and the result with a formatted
			/// URL, strength etc. is returned in blacklistMatch. The return value
			/// is true if the URL is considered suspicious, otherwise false.

		bool isUrlMatch(HTTPRequest& request, BlacklistMatch& blacklistMatch);

		bool isTokenMatch(HTTPRequest& request, BlacklistMatch& blacklistMatch);

		void loadBlacklist(string path);

		void loadBlacklist(Options* options, Database* db);

	private:
		Blacklist _blacklist;
		Extensions _extensions;
		SharedPtr<RegularExpression> _splitToken;
		SharedPtr<RegularExpression> _splitExtension;
		SharedPtr<RegularExpression> _wordDelimiter;

		string abbrUrl(string boldUrl);
		void setRegexps();
		float getExtensionFactor(string url);

};

#include "Database.h"
#include "Request.h"
#include "Options.h"
#endif // FILTER_H
