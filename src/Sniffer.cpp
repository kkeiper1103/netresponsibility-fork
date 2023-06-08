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


#include "Sniffer.h"



Sniffer* Sniffer::_instance = 0;



Sniffer::Sniffer() {
	_instance = this;
	_logStream = new LogStream(Application::instance().logger());
	_logStream->warning();
	_db = &MainApplication::getDatabase();
	_filter = new Filter(&MainApplication::getOptions(), _db);
}



void Sniffer::run() {
	vector<string> devs = getDevices();
	vector<SnifferThread*> threads;
	for (vector<string>::iterator it = devs.begin(); it != devs.end(); it++) {
		threads.push_back(new SnifferThread());
		if (threads.back()->openDevice(*it) == -1)
			threads.pop_back();
		else
			Poco::ThreadPool::defaultPool().start(*(threads.back()));
	}
	Poco::ThreadPool::defaultPool().joinAll();
}



Filter& Sniffer::getFilter() {
	return *_instance->_filter;
}



LogStream& Sniffer::getLogStream() {
	return *_instance->_logStream;
}



void Sniffer::logUrl(HTTPRequest& request) {
	_instance->_db->logUrl(request);
}



void Sniffer::logWarning(BlacklistMatch match) {
	_instance->_db->logWarning(match);
}



vector<string> Sniffer::getDevices() {
	vector<string> devices;
	pcap_if_t *alldevs,
		*d;

	if(pcap_findalldevs(&alldevs, _errbuf) == -1) {
		*_logStream <<"Error in pcap_findalldevs_ex: " <<_errbuf <<endl;
		throw;
	}

	for(d = alldevs; d; d = d->next) {
		string dev(d->name);
		if (dev == "any") {
			devices.clear();
			devices.push_back(dev);
			break;
		}
		else
			devices.push_back(dev);
	}

	if (devices.size() == 0) {
		*_logStream <<"No interfaces found! "
				<<"Make sure LibPcap/WinPcap is installed." <<endl;
		throw;
	}

	pcap_freealldevs(alldevs);

	return devices;
}
