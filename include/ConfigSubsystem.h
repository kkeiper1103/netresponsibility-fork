//
// Library: Net Responsibility
// Package: Core
// Module:  ConfigSubsystem
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
// <ConfigSubsystem> is a Subsystem that takes care of configuration



#ifndef CONFIGSUBSYSTEM_H
#define CONFIGSUBSYSTEM_H

#include "Options.h"
#include "Request.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/ClassLoader.h"
#include "Poco/Logger.h"
#include "Poco/Thread.h"
#include <iostream>

class MainApplication;

using Poco::Util::Application;
using Poco::Logger;
using namespace std;


class ConfigSubsystem: public Poco::Util::Subsystem
	/// ConfigSubsystem is a Subsystem that takes care of the configuration.
	/// It will only do anything if one of the arguments --config or --install
	/// is specified.
{
	public:
		const char* name() const;
		void reinitialize(Application &app);

	protected:
		Logger *_logger;
		void initialize(Application &self);
		void uninitialize();
		void getLogin(string &username, string &password);
			/// Get the username and password through std::cin. This is not
			/// neccessary if --username=X and --password=Y are given as arguments.
};

#include "MainApplication.h"
#endif // CONFIGSUBSYSTEM_H
