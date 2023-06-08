//
// Library: Net Responsibility
// Package: Core
// Module:  MainApplication
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
// <MainApplication> is responsibility for making Net Responsibility a
// service/daemon and handle CLI arguments, logging etc.



#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#define STRINGIFY(path, file) #path #file
#define CONCAT(x, y) STRINGIFY(x, y)

#define DEFAULT_LOGFILE CONCAT(PKGDATADIR, /messages.log)

#include "Options.h"
#include "Database.h"
#include "Request.h"
#include "ConfigSubsystem.h"
#include "ReportSubsystem.h"
#include "SnifferSubsystem.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Platform.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/FileChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/AutoPtr.h"
#include "Poco/Thread.h"
#include "Poco/Process.h"
#include "Poco/PipeStream.h"
#include "Poco/StreamCopier.h"
#include <iostream>
#include <sstream>

#if defined(POCO_OS_FAMILY_UNIX)
#include "unistd.h"
#include "signal.h"
#include "Poco/SyslogChannel.h"
using Poco::SyslogChannel;
#endif

using Poco::Logger;
using Poco::FileChannel;
using Poco::ConsoleChannel;
using Poco::SplitterChannel;
using Poco::FormattingChannel;
using Poco::PatternFormatter;
using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Subsystem;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::DateTimeFormatter;
using Poco::AutoPtr;
using Poco::Process;
using Poco::ProcessHandle;

using namespace std;

class MainApplication: public ServerApplication
	/// MainApplication is the skeleton of Net Responsibility. This is where all
	/// Options are processed, the Database loaded, the Logger set up, the signals
	/// handled and the Subsystems called.
	///
	/// MainApplication is derived from Poco::Util::ServerApplication, which
	/// also takes care of making the instance a daemon or service, if requested.
{
	public:
		MainApplication();
			/// Default constructor. Sets some class variables and registers the
			/// Subsystems that are to be invoked.

		~MainApplication() override;

		static MainApplication &instance();
			/// A public static method to access this very instance from any class.

		static Options &getOptions();
			/// A public static method to access the Options object from any class.

		static Database &getDatabase();
			/// A public static method to access the Database from any class.

		static void terminateNicely(bool deletePidfile= false);
			/// Terminate Net Responsibility nicely. This unmasks all singals,
			/// deleted the pidfile if told to, and tells the instance to shut down.

	protected:
		void initialize(Application& self) override;
			/// Invokes a set of methods and constructs Options and Database.
			/// A short glimpse of the actual code will reveal pretty much what it
			/// does.

		void uninitialize() override;

		void setupLogger();
			/// Sets up the Logger. This is done differently on on different
			/// platforms. Unix platforms will talk to syslog by default. If the
			/// instance is not running as a daemon/service, all messages will be
			/// sent to stdout instead. If a specific logfile is specified, the
			/// messages will go there.

		void defineOptions(OptionSet& options) override;
			/// Defines all arguments that are accepted. These must be entered
			/// the Unix style: --argument=value or -avalue.

		void handleHelp(const string& name, const string& value);
			/// This method is called if the --help argument is specified. It will
			/// only print out all available arguments with displayHelp() and exit.

		void setOption(const string& name, const string& value);
			/// The default way that Poco handles configurations is not ideal for
			/// our purposes. It's way to flexible for accountability softwares.
			/// Therefor the options are processed in Options. However, some global
			/// values are specified in the default config system. This is because
			/// they're easliy reached from any class. setOption() is used to store
			/// these global values.

		void displayHelp();

		void checkForRoot();
			/// Checks if the user who started the program is having enough
			/// priviligues. In Linux you'll need to be running Net Responsibility
			/// as root.

		static void signalHandler(int sig = -1);
			/// The signal handler. Masks all possible signals, and catches them
			/// properly, by logging the shutdown in the Database.

		int main(const vector<string>& args) override;
			/// The main method invoked after the Subsystems are run. At this
			/// moment it is more or less redundent, since all the actual code
			/// is run inside the Subsystems.

	private:
		bool _helpRequested;
		static MainApplication *_instance;
		Options *_options;
		Database *_database;
};

#endif // MAINAPPLICATION_H
