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
// <MyXml> adds extra functionality to XMLConfiguration



#include "MyXml.h"



MyXml::MyXml() : XMLConfiguration() {
	_logger = &Application::instance().logger();
}



MyXml::MyXml(string path) : XMLConfiguration(path) {
	_logger = &Application::instance().logger();
}



vector<string> MyXml::getStringVector(string key) const {
	vector<string> vec,
		subkeys;
	this->keys(key, subkeys);
	for (vector<string>::iterator it = subkeys.begin();
			it != subkeys.end(); ++it)
	{
		vec.push_back(this->getString(key + '.' + *it));
	}
	return vec;
}



map<string, string> MyXml::getStringMap() const {
	map<string, string> m;
	vector<string> subkeys;
	this->keys(subkeys);
	for (vector<string>::iterator it = subkeys.begin();
			it != subkeys.end(); ++it)
	{
		m[*it] = this->getString(*it);
	}
	return m;
}



Blacklist MyXml::getBlacklist() {
	string line;
	Blacklist blacklist;
	BlacklistCategory tempCategory;
	BlacklistKeyword tempKeyword;
	vector< SharedPtr<RegularExpression> > tempRe;
	vector<string> categories, keywords;

	int o,
		options = RegularExpression::RE_CASELESS;
	const int DEFAULT_STRENGTH = 100;
	RegularExpression::Match m;
	RegularExpression whitespace ("[^\\s]+", 0, true);

	this->keys(categories);

	for (vector<string>::iterator c = categories.begin();
			c != categories.end(); c++)
	{
		if (this->hasProperty(*c + "[@name]")) {
			tempCategory.name = this->getString(*c + "[@name]");
			blacklist.push_back(tempCategory);
			this->keys(*c, keywords);

			for (vector<string>::iterator k = keywords.begin();
					k != keywords.end(); k++)
			{
				tempRe.clear();
				try {
					line = this->getString(*c + '.' + *k);
					if (line.find(" ") != string::npos) {
						o = 0;
						while (whitespace.match(line, o, m)) {
							tempRe.push_back(new RegularExpression
									(line.substr(m.offset, m.length), options, true));
							o = m.offset + m.length;
						}
					}
					else
						tempRe.push_back(new RegularExpression(line, options, true));
				}
				catch (Poco::Exception &err) {
					_logger->information(err.displayText() + (string)": " + line);
					continue;
				}
				tempKeyword.re = tempRe;
				tempKeyword.asString = line;

				tempKeyword.strength = (this->hasProperty(*c + '.' + *k + "[@s]")
						? this->getInt(*c + '.' + *k + "[@s]") : DEFAULT_STRENGTH);
				blacklist.back().keyword.push_back(tempKeyword);
			}
		}
	}
	return blacklist;
}



Extensions MyXml::getExtensions() {
	Extensions extensions;
	Extension tempExt;
	vector<string> keywords;
	string line;
	int options = RegularExpression::RE_CASELESS;
	const int DEFAULT_STRENGTH = 100;

	this->keys("extensions", keywords);

	for (vector<string>::iterator k = keywords.begin();
			k != keywords.end(); k++)
	{
		line = this->getString("extensions." + *k);
		try {
			tempExt.re = new RegularExpression(line, options, true);
		}
		catch (Poco::Exception &err) {
			_logger->information(err.displayText() + (string)": " + line);
			continue;
		}
		tempExt.group = this->getString("extensions." + *k + "[@group]");
		tempExt.strength = (this->hasProperty("extensions." + *k + "[@s]")
				? this->getInt("extensions." + *k + "[@s]") : DEFAULT_STRENGTH);
		extensions.push_back(tempExt);
	}
	return extensions;
}
