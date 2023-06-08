//
// Library: Net Responsibility
// Package: Core
// Module:  Sniffer
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
// <Sniffer> listens for HTTP requests, and makes sure they're getting logged
// Much of this code is inspired by examples provided by TCPDump and Winpcap



#ifndef SNIFFER_H
#define SNIFFER_H

#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
#define _CRT_SECURE_NO_WARNINGS
#endif


#include <iostream>
#include <fstream>
#include <vector>

#include "Poco/RegularExpression.h"
#include "Poco/SharedPtr.h"
#include "Poco/ThreadPool.h"
#include "Poco/Logger.h"
#include "Poco/LogStream.h"
#include "Poco/Net/HTTPRequest.h"

#include "Database.h"
#include "Options.h"
#include "Filter.h"
#include "Blacklist.h"
#include "SnifferThread.h"

#include <pcap.h>

#if defined(POCO_OS_FAMILY_WINDOWS)
#include <winsock2.h>
#elif defined (POCO_OS_FAMILY_UNIX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif //POCO_OS_FAMILY_

using Poco::RegularExpression;
using Poco::SharedPtr;
using Poco::ThreadPool;
using Poco::Logger;
using Poco::LogStream;
using Poco::Net::HTTPRequest;
using namespace std;

struct sniff_ethernet;
struct sniff_ip;
struct sniff_tcp;

class MainApplication;
class SnifferThread;

class Sniffer
	/// Sniffer sets up several SnifferThreads. One SnifferThread for each
	/// interface, or only one for the "any" interface if it's found.
	/// Much of this code is inspired by examples provided by TCPDump and Winpcap
{
	public:
		Sniffer();
			/// Default constructor

		void run();
			/// Run the sniffer

	private:
		Filter *_filter;
		Database *_db;
		LogStream *_logStream;
		static Sniffer* _instance;
		char _errbuf[PCAP_ERRBUF_SIZE];

		static Filter& getFilter();
		static LogStream& getLogStream();
		static void logUrl(HTTPRequest&);
		static void logWarning(BlacklistMatch);
		vector<string> getDevices();

		friend class SnifferThread;
};

#include "MainApplication.h"
#endif // SNIFFER_H
