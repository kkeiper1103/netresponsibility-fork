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
// <Options> stores the configurations and other useful variables



#include "Options.h"



Options::Options()
{
	_initBypasses = new Bypasses();
	_logger = &Application::instance().logger();
	_logger->information("Loading options");
	loadDefaultValues();
	_logger->debug("Config file: " + _configfile);
	if (!Application::instance().config().getBool("config", false))
		loadConfigfile();
	_logger->debug("Txt file: " + _txtfile);
	loadTxt();
}


Options::Options(int argc, char* argv[])
{
	loadDefaultValues();
	loadConfigfile();
	loadTxt();
}



Options::~Options()
{
	//dtor
}



bool Options::doSaveHistory() const {
	return _saveHistory;
}



bool Options::doSendImprovementData() const {
	return _sendImprovementData;
}



bool Options::doCompressAttachedReport() const {
	return _compressAttachedReport;
}



string Options::getName() const {
	return _name;
}



string Options::getUsername() const {
	return _username;
}



string Options::getEmailFrom() const {
	return _emailFrom;
}



string Options::getConfigfile() const {
	return _configfile;
}



string Options::getServer() const {
	return _server;
}



string Options::getMacAddress() const {
	// Compute the MAC address every time, since it will be 00:00:00:00:00:00
	// if no internet connection is found. Then we have to reload it when the
	// internet connection is established.
	return Poco::Environment::nodeId();
}



string Options::getVersion() const {
	return _version;
}



string Options::getDatabasefile() const {
	return _databasefile;
}



string Options::getPidfile() const {
	return _pidfile;
}



string Options::getBlacklistFile() const {
	return _blacklistFile;
}



string Options::getTxt(string key) const {
	if (_txt.find(key) != _txt.end())
		return _txt.find(key)->second;
	else
		return "";
}



string Options::getReportModule() const {
	return _reportModule;
}



vector<string> Options::getEmailTo() const {
	return _emailTo;
}



vector<string> Options::getReportParts() const {
	return _reportParts;
}



vector<string> Options::getAttachedReportParts() const {
	return _attachedReportParts;
}



int Options::getReportFrequency() const {
	return _reportFrequency;
}



int Options::getReportStrengthThreshold() const {
	return _reportStrengthThreshold;
}



Bypasses &Options::getInitBypasses() const {
	return *_initBypasses;
}



void Options::setUsername(string username) {
	_username = username;
}



void Options::loadDefaultValues() {
	_configfile    = CONFIGFILE;
	_databasefile  = DATABASEFILE;
	_pidfile       = PIDFILE;
	_blacklistFile = BLACKLISTFILE;
	_server        = SERVER;
	_txtfile       = TXTFILE;
	_reportModule  = REPORT_MODULE;
	_version       = VERSION;
	_saveHistory   = true;
	_username      = "";
	_logger->debug("Version " + _version);
}



void Options::loadConfigfile() {
	for (int moreTries = 3; moreTries > 0; moreTries--) {
		try {
			AutoPtr<MyXml> xmlConfig (new MyXml(_configfile));
			_username   = xmlConfig->getString("username");
			_emailFrom  = xmlConfig->getString("emailFrom");
			_name       = xmlConfig->getString("name");

			_sendImprovementData    = xmlConfig->getBool("improveData", false);
			_compressAttachedReport	= xmlConfig->getBool("compressAttachedReport", false);

			_emailTo             = xmlConfig->getStringVector("emailTo");
			_reportParts         = xmlConfig->getStringVector("reportParts");
			_attachedReportParts = xmlConfig->getStringVector("attachedReportParts");

			_reportFrequency = xmlConfig->getInt("reportFrequency", 7);
			_reportStrengthThreshold = xmlConfig->getInt("reportStrengthThreshold", 0);

			_saveHistory = isAttachedReportPart("history_hostnames")
					|| isAttachedReportPart("history_paths");

			if (!Request::modifiedFilesUpdate(this))
				moreTries = 0;
		}
		catch (Poco::FileNotFoundException &err) {
			Application::instance().config().setBool("config", true);
			_initBypasses->addRow(BYPASS_MISSING_FILE, "Config file");
			moreTries = 0;
		}
		catch (Poco::Exception &err) {
			_logger->information("Corrupt configfile, trying to download new: "
					+ err.displayText());
			if (moreTries == 3)
				_initBypasses->addRow(BYPASS_MODIFIED_FILE, "Config file");
			else if (moreTries == 1)
				Application::instance().config().setBool("config", true);
			Request::downloadConfig(this);
		}
	}
}



void Options::loadTxt() {
	try {
		AutoPtr<MyXml> xmlTxt (new MyXml(_txtfile));
		_txt = xmlTxt->getStringMap();
	}
	catch (Poco::Exception &err) {
		_logger->warning("Corrupt txtfile");
		_initBypasses->addRow(BYPASS_MODIFIED_FILE, "Txt file");
	}
}



bool Options::isReportPart(string part) const {
	vector<string>::const_iterator it;
	it = find (_reportParts.begin(), _reportParts.end(), part);
	return it != _reportParts.end();
}



bool Options::isAttachedReportPart(string part) const {
	vector<string>::const_iterator it;
	it = find (_attachedReportParts.begin(), _attachedReportParts.end(), part);
	return it != _attachedReportParts.end();
}
