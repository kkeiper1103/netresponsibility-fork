#include "SnifferSubsystem.h"
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
// <SnifferSubsystem> is a Subsystem that takes care of setting up <Sniffer>


const char* SnifferSubsystem::name() const {
	return "SnifferSubsystem";
}



void SnifferSubsystem::reinitialize(Application& app) {
	uninitialize();
	initialize(app);
}



void SnifferSubsystem::initialize(Application& app) {
	_logger = &app.logger();
	if (app.config().getBool("sniffer", true) && isOnlyInstance()) {
		_logger->information("Starting sniffer");

		Sniffer sniffer;
		sniffer.run();
	}

}



void SnifferSubsystem::uninitialize() {
	//Stop current sniffing, release memory or similar?
}



bool SnifferSubsystem::isOnlyInstance() {
	string pidfile = MainApplication::getOptions().getPidfile();
	int currentPid = Poco::Process::id(),
		oldPid = currentPid;
	if (File(pidfile).exists()) {
		ifstream f(pidfile.c_str(), ios::in);
		f >> oldPid;
		f.close();
	}
#if defined(POCO_OS_FAMILY_UNIX)
	if (oldPid != currentPid && kill(oldPid, 0) == 0) {
		_logger->warning("Other instance found");
		return false;
	}
	MainApplication::getDatabase().logSessionStart();
#endif
	ofstream f(pidfile.c_str(), ios::out);
	f << currentPid;
	f.close();
	return true;

}
