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



#include "MainApplication.h"



MainApplication* MainApplication::_instance = 0;



MainApplication::MainApplication()
{
	_instance = this;
	_helpRequested = false;
	setUnixOptions(true);

	addSubsystem(new ConfigSubsystem);
	addSubsystem(new ReportSubsystem);
	addSubsystem(new SnifferSubsystem);
}



MainApplication::~MainApplication() {
	_instance = 0;
}


MainApplication& MainApplication::instance() {
	return *_instance;
}



Options& MainApplication::getOptions() {
	return *_instance->_options;
}



Database& MainApplication::getDatabase() {
	return *_instance->_database;
}



void MainApplication::initialize(Application& self)
{
	setupLogger();
	checkForRoot();
	logger().notice("Starting Net Responsibility");
	_options = new Options();
	_database = new Database(*_options);
	signalHandler();
	ServerApplication::initialize(self);
}



void MainApplication::uninitialize()
{
	logger().notice("Shutting down Net Responsibility");
	delete _options;
	delete _database;
	ServerApplication::uninitialize();
}



void MainApplication::setupLogger() {
	Logger& logger = Logger::get("net-responsibility");
	bool debug = config().getBool("debug", false);
	string level,
		logfile = "",
		customLogfile = config().getString("logfile", "");
	AutoPtr<FormattingChannel> formattingChannel(new FormattingChannel);
	AutoPtr<PatternFormatter> patternFormatter(new PatternFormatter);
	if (customLogfile != "") {
		logfile = customLogfile;
	}
	else if (isInteractive()) {
		AutoPtr<ConsoleChannel> channel(new ConsoleChannel);
		formattingChannel->setChannel(channel);
		level = (debug ? "debug" : "information");
	}
#if defined(POCO_OS_FAMILY_UNIX)
	else if (config().getBool("application.runAsDaemon", false)) {
		AutoPtr<SyslogChannel> channel(new SyslogChannel);
		channel->setProperty("name", "net-responsibility");
		channel->setProperty("facility", "LOG_DAEMON");
		channel->setProperty("options", "LOG_PID");
		formattingChannel->setChannel(channel);
		level = "notice";
	}
#endif
	else
		logfile = DEFAULT_LOGFILE;

	if (logfile != "") {
		AutoPtr<FileChannel> channel(new FileChannel);
		channel->setProperty("path", logfile);
		channel->setProperty("times", "local");
		channel->setProperty("rotation", "200 K");
		channel->setProperty("purgeCount", "1");
		patternFormatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
		formattingChannel->setChannel(channel);
		formattingChannel->setFormatter(patternFormatter);
		level = (debug ? "debug" : "notice");
	}
	logger.setChannel(formattingChannel);
	logger.setLevel(level);
	Application::setLogger(logger);
}



void MainApplication::handleHelp(const string& name, const string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
	terminate();
}



void MainApplication::setOption(const string& name, const string& value)
{
	if (name == "report")
		config().setInt("report", REPORT_MANUAL);
	else if (name == "test-report")
		config().setInt("report", REPORT_TEST);
	else if (name == "config")
		config().setBool("config", true);
	else if (name == "debug")
		config().setBool("debug", true);
	else if (name == "no-sniffer")
		config().setBool("sniffer", false);
	else if (name == "install") {
		config().setBool("config", true);
		config().setInt("report", REPORT_INSTALL);
		config().setBool("sniffer", false);
	}
	else if (name == "uninstall") {
		config().setInt("report", REPORT_UNINSTALL);
		config().setBool("sniffer", false);
	}

}



void MainApplication::defineOptions(OptionSet& options)
{
	ServerApplication::defineOptions(options);
	config().setString("logfile", "");
	options.addOption(
			Option("install", "", "Send installation report")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));

	options.addOption(
			Option("uninstall", "", "Send uninstallation report")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));

	options.addOption(
			Option("debug", "d", "Log debug messages")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));

	options.addOption(
			Option("help", "h", "Display this help message")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::handleHelp)));

	options.addOption(
			Option("report", "r", "Manually send report")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));

	options.addOption(
			Option("test-report", "tr", "Send test report")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));

	options.addOption(
			Option("config", "c", "Configure Net Responsibility")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));

	options.addOption(
			Option("username", "u", "Configure with the specified username")
			.required(false)
			.repeatable(false)
			.argument("username")
			.binding("username"));

	options.addOption(
			Option("password", "p", "Configure with the specified password")
			.required(false)
			.repeatable(false)
			.argument("password")
			.binding("password"));

	options.addOption(
			Option("logfile", "l", "Log messages to the specified file")
			.required(false)
			.repeatable(false)
			.argument("file")
			.binding("logfile"));

	options.addOption(
			Option("no-sniffer", "ns", "Skip the sniffer on this instance")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<MainApplication>
			(this, &MainApplication::setOption)));
}



void MainApplication::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");

	//set header from txtfile instead?
	helpFormatter.setHeader("Net Responsibility is intended to be a good "
			"open source accountability software for all major platforms. "
			"It's purpose is to prevent people from certain surfing habits "
			"by holding them accountable to others. This is done by logging "
			"all internet traffic and generating clear reports that are "
			"emailed regularly to the accountability partner(s).");
	helpFormatter.format(cout);
}



void MainApplication::checkForRoot() {
#if defined(POCO_OS_FAMILY_UNIX)
	if (getuid() != 0) {
		logger().warning("ERROR: Net Responsibility must be run as root!");
		terminateNicely();
	}
#endif
}



void MainApplication::terminateNicely(bool deletePidfile) {
#if defined(POCO_OS_FAMILY_UNIX)
	struct sigaction dfl;
	dfl.sa_handler = SIG_DFL;
	sigaction(SIGINT, &dfl, 0);
#endif
	if (deletePidfile)
		File(_instance->_options->getPidfile()).remove();
	_instance->uninitialize();
	terminate();
}



void MainApplication::signalHandler(int sig) {
#if defined(POCO_OS_FAMILY_UNIX)
	switch(sig) {
		case -1:
			struct sigaction act;
			act.sa_handler = signalHandler;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;
			sigaction(SIGINT, &act, 0);	//Better way to do this?
			sigaction(SIGQUIT, &act, 0);
			sigaction(SIGKILL, &act, 0);
			sigaction(SIGTERM, &act, 0);
			sigaction(SIGHUP, &act, 0);
			sigaction(SIGSTOP, &act, 0);
			sigaction(SIGTSTP, &act, 0);
			sigaction(SIGCONT, &act, 0);
			sigaction(SIGUSR1, &act, 0);
			sigaction(SIGUSR2, &act, 0);
			break;
		case SIGINT:
		case SIGQUIT:
		case SIGTSTP:
		case SIGHUP:
		case SIGKILL:
		case SIGTERM:
			//Log Shutdown!
			_instance->_database->logSessionStop();
			terminateNicely(true);
			break;
		case SIGCONT:
			//Continued, means stopped
			break;
		case SIGUSR1:
			//Resetting Net Responsibility
			_instance->uninitialize();
			_instance->initialize(*_instance);
			break;
		case SIGUSR2:
			//Other action or just mask it
			break;
		default:
			//Caught signal
			break;
	}
#endif
}



int MainApplication::main(const vector<string>& args)
{
	if (!_helpRequested) {
		//waitForTerminationRequest();	//Redundant? Maybe needed for proxy etc.
	}
	return Application::EXIT_OK;
}
//POCO_SERVER_MAIN(MainApplication)
