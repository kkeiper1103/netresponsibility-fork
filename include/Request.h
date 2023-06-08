//
// Library: Net Responsibility
// Package: Core
// Module:  Request
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
// <Request> is in charge of interaction with the server



#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <fstream>
#include <sstream>

#include "Options.h"
#include "Blacklist.h"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/Logger.h"
#include "Poco/StreamCopier.h"
#include "Poco/Timestamp.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Thread.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Util::Application;
using Poco::Util::XMLConfiguration;
using Poco::Logger;
using Poco::StreamCopier;
using Poco::Timestamp;
using Poco::Path;
using Poco::File;
using Poco::URI;
using Poco::Exception;

using namespace std;

class Options; //Forward declaration

class Request
	/// Request is in charge of interaction with the server. It will always send
	/// the username, MAC address and version. You may also post more variables.
{
	public:
		Request() {}
		virtual ~Request() {}
		static void addMac(Options* options, string password = "");
			/// Configure the server to remember the MAC address of this computer.
			/// It will then be used for authentification.

		static void downloadConfig(Options* options, string password = "");
			/// Download a new config file.

		static void downloadBlacklist(Options* options);
			/// Download a new blacklist

		static string sendImprovementData(Options* options, string impData);
			/// Send the improvement data to the server. It's given in impData.

		static bool modifiedFilesUpdate(Options* options);
			/// Update the config file and blacklist if they're modified.

	private:
		static string send(Options *options, string uriPath,
					string filePath = "", string morePostVars = "");
};

#endif // REQUEST_H
