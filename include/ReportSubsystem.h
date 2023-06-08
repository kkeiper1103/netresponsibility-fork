//
// Library: Net Responsibility
// Package: Core
// Module:  ReportSubsystem
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
// <ReportSubsystem> is a Subsystem that takes care of sending reports



#ifndef REPORTSUBSYSTEM_H
#define REPORTSUBSYSTEM_H

#include "Options.h"
#include "Database.h"
#include "ReportBase.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/ClassLoader.h"
#include "Poco/Logger.h"
#include "Poco/Thread.h"
#include "Poco/File.h"
#include <iostream>
#include <fstream>

class MainApplication;

using Poco::Util::Application;
using Poco::Util::Subsystem;
using Poco::ClassLoader;
using Poco::Logger;
using Poco::File;
using namespace std;

enum ReportType
{
	REPORT_FALSE,
	REPORT_MANUAL,
	REPORT_SCHEDULED,
	REPORT_INSTANT,
	REPORT_TEST,
	REPORT_INSTALL,
	REPORT_UNINSTALL
};

class ReportSubsystem: public Poco::Util::Subsystem
	/// ReportSubsystem is a Poco::Util::Subsystem that determines if we should
	/// send a report, and makes sure it's done properly.
{
	public:
		const char* name() const;
			/// Returns the name of the class: "ReportSubsystem"

		void reinitialize(Application& app);

	protected:
		Logger *_logger;
		void initialize(Application& self);
			/// Run the Subsystem

		void uninitialize();
		void setScheduledReport(int& type);
			/// Check if it's time to send a scheduled report, and in that case
			/// set type to REPORT_SCHEDULED
};

#include "MainApplication.h"
#endif // REPORTSUBSYSTEM_H
