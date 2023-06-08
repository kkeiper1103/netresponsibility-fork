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
// <Request> is in charge of interaction with the server



#include "Request.h"



void Request::addMac(Options *options, string password) {
	Application::instance().logger().information("Downloading configuration file");
	if (password == "")
		send(options, "/request/add_mac.php", options->getConfigfile());
	else
		send(options, "/request/add_mac.php", options->getConfigfile(),
				"online_password=" + password);
}



void Request::downloadConfig(Options *options, string password) {
	Application::instance().logger().information("Downloading configuration file");
	if (password == "")
		send(options, "/request/config.php", options->getConfigfile());
	else
		send(options, "/request/config.php", options->getConfigfile(),
				"online_password=" + password);
}



void Request::downloadBlacklist(Options *options) {
	Application::instance().logger().information("Downloading blacklists");
	send(options, "/request/blacklist.php", options->getBlacklistFile());
}



string Request::sendImprovementData(Options *options, string impData) {
	impData = "warnings=" + impData;
	return send(options, "/request/add_improve_data.php", "", impData);
}



bool Request::modifiedFilesUpdate(Options *options) {
	try {
		bool isModified = false;
		string downloaded = send(options, "/request/downloaded.php");
		stringstream ss(downloaded);
		AutoPtr<XMLConfiguration> xml(new XMLConfiguration(ss));
		if (downloaded != "") {
			//Check configfile
			int downloadedInt;
			downloadedInt = xml->getInt("config", 0);
			Timestamp ft = File(options->getConfigfile()).getLastModified();
			if (ft.epochTime() > (downloadedInt + 15)) {
				downloadConfig(options);
				options->getInitBypasses().addRow(BYPASS_MODIFIED_FILE, "Config file");
				isModified = true;
			}

			//Check blacklist
			downloadedInt = xml->getInt("blacklist", 0);
			Timestamp ft2 = File(options->getBlacklistFile()).getLastModified();
			if (ft2.epochTime() > (downloadedInt + 15)) {
				downloadBlacklist(options);
				options->getInitBypasses().addRow(BYPASS_MODIFIED_FILE, "Blacklist");
				isModified = true;
			}
		}
		return isModified;
	}
	catch (Exception& exc) {
		Application::instance().logger().debug(exc.displayText());
		return false;
	}
}



string Request::send(Options *options, string uriPath,
		string filePath, string morePostVars)
{
	while (true) {
		try {
			string uriString = "http://" + options->getServer() + uriPath;
			URI uri(uriString);
			string path(uri.getPathAndQuery());
			if (path.empty())
				path = "/";

			HTTPClientSession session(uri.getHost(), uri.getPort());
			HTTPRequest req(HTTPRequest::HTTP_POST, path);
			req.setContentType("application/x-www-form-urlencoded");
			string reqBody = "";
			reqBody += "online_user=" + options->getUsername()
					+ "&mac=" + options->getMacAddress()
					+ "&version=" + options->getVersion();
			if (morePostVars != "")
				reqBody += "&" + morePostVars;
			req.setContentLength(reqBody.length());
			session.sendRequest(req) <<reqBody;
			HTTPResponse res;
			istream& rs = session.receiveResponse(res);

			if (res.getStatus() == 200) {
				string s((istreambuf_iterator<char>(rs)), istreambuf_iterator<char>());
				if (s.substr(0, 5) == "ERROR")
					Application::instance().logger().warning(s);
				else if (filePath == "") {
					return s;
				}
				else {
					ofstream saveFile(filePath.c_str(), ios::out);
					saveFile << s;
				}
			}
			return "";
		}
		catch (Exception& exc) {
			Application::instance().logger().debug(exc.displayText());
			Poco::Thread::sleep(20000);
		}
	}
}
