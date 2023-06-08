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
// <ReportSubsystem> is a Subsystem that takes care of sending reports



#include "ReportSubsystem.h"


const char* ReportSubsystem::name() const {
	return "ReportSubsystem";
}



void ReportSubsystem::reinitialize(Application& app) {
	uninitialize();
	initialize(app);
}



void ReportSubsystem::initialize(Application& app) {
	_logger = &Application::instance().logger();
	_logger->debug("Initializing subsystem: Report Subsystem");
	int type = app.config().getInt("report", REPORT_FALSE);
	setScheduledReport(type);
	if (type != REPORT_FALSE) {
		ClassLoader<ReportBase> loader;
		string lib = MainApplication::getOptions().getReportModule();
		try {
			loader.loadLibrary(lib);
			ReportBase* report = loader.create("Report");
			if (type == REPORT_TEST)  {
				_logger->notice("Sending test report");
				report->test();
			}
			else {
				_logger->notice("Generating report");
				if (type == REPORT_INSTALL)
					report->install();
				else if (type == REPORT_UNINSTALL)
					report->uninstall();
				else
					report->generate();

				_logger->notice("Sending report");
				int errorCode = report->send();
				while (errorCode == 1) {
					_logger->debug("Retrying to send report in 20 seconds");
					Poco::Thread::sleep(20000);
					errorCode = report->send();
				}
				if (errorCode == 0) {
					_logger->notice("Report finished");
					report->logFinish();
					app.config().setInt("report", REPORT_FALSE);
				}
				else {
					_logger->warning("Report could not be sent");
				}
			}

			delete report;

			loader.unloadLibrary(lib);
		}
		catch (Poco::Exception &exc) {
			_logger->warning(exc.displayText());
		}
	}
}



void ReportSubsystem::uninitialize() {
	//Stop current reporting, release memory or similar?
}



void ReportSubsystem::setScheduledReport(int &type) {
	int frequency = MainApplication::getOptions().getReportFrequency();
	if (type != REPORT_INSTALL && type != REPORT_UNINSTALL
			&& MainApplication::getDatabase().isReportTime(frequency))
	{
		type = REPORT_SCHEDULED;
	}
}
