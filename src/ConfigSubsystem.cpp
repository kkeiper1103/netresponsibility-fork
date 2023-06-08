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



#include "ConfigSubsystem.h"


const char* ConfigSubsystem::name() const {
	return "ConfigSubsystem";
}



void ConfigSubsystem::reinitialize(Application& app) {
	uninitialize();
	initialize(app);
}



void ConfigSubsystem::initialize(Application& app) {
	if (app.config().getBool("config", false)) {
		string username = app.config().getString("username", ""),
			password = app.config().getString("password", "");
		Options* options = &MainApplication::getOptions();
		if ((username == "" || password == "")
				&& MainApplication::instance().isInteractive())
		{
			getLogin(username, password);
		}
		else if (username == "")
			username = options->getUsername();
		options->setUsername(username);
		Request::addMac(options, password);
		options->loadConfigfile();
		app.config().setBool("config", false);
	}
}



void ConfigSubsystem::uninitialize() {

}



void ConfigSubsystem::getLogin(string &username, string &password) {
	cout <<endl <<"Configure:"
			<<endl <<"Username: ";
	cin >> username;
	cout <<"Password: ";
	while (password == "")
		cin >> password;
	return;
}
