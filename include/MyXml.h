//
// Library: Net Responsibility
// Package: Core
// Module:  MyXml
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
// <MyXml> adds extra functionality to XMLConfiguration



#ifndef MYXML_H
#define MYXML_H

#include <iostream>
#include <vector>
#include <map>
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/Util/Application.h"
#include "Poco/RegularExpression.h"
#include "Poco/AutoPtr.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"

#include "Blacklist.h"

using Poco::AutoPtr;
using Poco::Util::XMLConfiguration;
using Poco::Util::Application;
using Poco::RegularExpression;
using Poco::Logger;
using namespace std;

class MyXml : public XMLConfiguration
	/// MyXml inherits XMLConfiguration and adds some extra functionality to it.
	/// It's most important task is to extract and compile the blacklists, as
	/// well as the txtfile.
{
	public:
		MyXml();

		MyXml(string path);
			/// Load the XML file found at path.

		vector<string> getStringVector(string key) const;
			/// Returns a string vector with the values found at "key".

		map<string, string> getStringMap() const;
			/// Returns a map<string, string> with all values found in the document.

		Blacklist getBlacklist();
			/// Extracts and compiles the keywords in the blacklist file.

		Extensions getExtensions();
			/// Extracts every Extension shipped with the blacklists.

	protected:
		Logger* _logger;
};

#endif // MYXML_H
