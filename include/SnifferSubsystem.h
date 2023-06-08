//
// Library: Net Responsibility
// Package: Core
// Module:  SnifferSubsystem
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
// <SnifferSubsystem> is a Subsystem that takes care of setting up <Sniffer>


#ifndef SNIFFERSUBSYSTEM_H
#define SNIFFERSUBSYSTEM_H

#include "Options.h"
#include "Sniffer.h"
#include "MainApplication.h"

#include <fstream>

#if defined(POCO_OS_FAMILY_UNIX)
#include <signal.h>
#include "BootHistory.h"
#endif

#include "Poco/Util/Application.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/File.h"
#include "Poco/Logger.h"
#include "Poco/Process.h"
#include <iostream>

using Poco::Util::Application;
using Poco::Util::Subsystem;
using Poco::File;
using Poco::Logger;
using namespace std;

class SnifferSubsystem: public Poco::Util::Subsystem
	/// SnifferSubsystem is a Subsystem that takes care of setting up Sniffer.
{
	public:
		const char* name() const;
		void reinitialize(Application& app);

	protected:
		Logger *_logger;
		void initialize(Application& self);
		void uninitialize();
		bool isOnlyInstance();
			/// Determines if there is any other instance running, if so this one
			/// will not survive.
};

#endif // SNIFFERSUBSYSTEM_H
