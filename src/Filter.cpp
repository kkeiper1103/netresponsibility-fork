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
// <Filter> determines whether the URLs are appropriate or not.

#include "Filter.h"


Filter::Filter() {
	setRegexps();
}



Filter::Filter(string blacklistFile) {
	setRegexps();
	loadBlacklist(blacklistFile);
}



Filter::Filter(Options *options, Database *db) {
	setRegexps();
	loadBlacklist(options, db);
}



void Filter::loadBlacklist(string path) {
	try {
		AutoPtr<MyXml> xmlBlacklist (new MyXml(path));
		_blacklist   = xmlBlacklist->getBlacklist();
		_extensions  = xmlBlacklist->getExtensions();
	}
	catch (Exception &err) {
		Application::instance().logger().information("Couldn't load blacklist");
		//download new
	}

}



void Filter::loadBlacklist(Options *options, Database *db) {
	for (int moreTries = 3; moreTries > 0; moreTries--) {
		try {
			AutoPtr<MyXml> xmlBlacklist (new MyXml(options->getBlacklistFile()));
			_blacklist   = xmlBlacklist->getBlacklist();
			_extensions  = xmlBlacklist->getExtensions();
			moreTries = 0;
		}
		catch (Poco::FileNotFoundException &err) {
			Application::instance().logger()
					.information("Couldn't load blacklist, downloading new: "
					+ err.displayText());
			if (moreTries == 3)
				db->logBypass(BYPASS_MISSING_FILE, "Blacklist");
			Request::downloadBlacklist(options);
		}
		catch (Poco::Exception &err) {
			Application::instance().logger()
					.information("Couldn't load blacklist, downloading new: "
					+ err.displayText());
			if (moreTries == 3)
				db->logBypass(BYPASS_MODIFIED_FILE, "Blacklist");
			Request::downloadBlacklist(options);
		}
	}
}



bool Filter::isMatch(HTTPRequest& request, BlacklistMatch& blacklistMatch) {
	return isUrlMatch(request, blacklistMatch)
			&& isTokenMatch(request, blacklistMatch);
}



bool Filter::isUrlMatch(HTTPRequest& request, BlacklistMatch& blacklistMatch) {
	blacklistMatch.keyword.clear();
	blacklistMatch.whitelist = false;
	string url = request.getHost() + request.getURI(),
		boldUrl = url;
	bool isSubMatch,
		isMatch = false;
	int strength = 0;
	RegularExpression::Match m;

	for (Blacklist::iterator c = _blacklist.begin(); c != _blacklist.end(); c++) {
		for (vector<BlacklistKeyword>::iterator k = c->keyword.begin();
				k != c->keyword.end(); k++)
		{
			isSubMatch = true;
			for (vector< SharedPtr<RegularExpression> >::iterator
					r = k->re.begin(); r != k->re.end(); r++)
			{
				if (!(**r).match(url, m)) {
					isSubMatch = false;
					break;
				}
			}
			if (isSubMatch) {
				for (vector< SharedPtr<RegularExpression> >::iterator
						r = k->re.begin(); r != k->re.end(); r++)
				{
					(**r).subst(boldUrl, "<b>$0</b>", RegularExpression::RE_GLOBAL);
				}
				blacklistMatch.keyword.push_back(*k);
				blacklistMatch.keyword.back().category = c->name;
				strength += k->strength;
				if (c->name == "Whitelist")
					blacklistMatch.whitelist = true;
				else
					isMatch = true;
			}
		}
	}
	blacklistMatch.boldUrl = (isMatch ? boldUrl : "");
	blacklistMatch.abbrUrl = (isMatch ? abbrUrl(boldUrl) : "");
	blacklistMatch.strength = (isMatch ? strength : 0);
	return isMatch;
}



bool Filter::isTokenMatch(HTTPRequest& request, BlacklistMatch& blacklistMatch) {
	bool isSubMatch,
		isMatch = false;
	RegularExpression::Match m, n;
	float strength = 0,
		strengthFactor = 0,
		wordFactor = 0.5;
	int tokenMatches = 0;
	unsigned int o = 0;
	string url = request.getHost() + request.getURI(),
		token,
		decodedUrl = "";
	try {
		Poco::URI::decode(url, decodedUrl);
	}
	catch (Poco::Exception &exc) {
		decodedUrl = url;
	}
	while (o < decodedUrl.length()) {
		_splitToken->match(decodedUrl, o, m);
		if (m.offset == string::npos)
			m.offset = decodedUrl.length();
		token = decodedUrl.substr(o, m.offset-o);
		if (token.length() > 2) {
			tokenMatches = 0;
			for (vector<BlacklistKeyword>::iterator
					k = blacklistMatch.keyword.begin();
					k != blacklistMatch.keyword.end(); k++)
			{
				isSubMatch = true;
				strengthFactor = 0;
				for (vector< SharedPtr<RegularExpression> >::iterator
						r = k->re.begin(); r != k->re.end(); r++)
				{
					if ((**r).match(token, n)) {
						while (n.offset != string::npos) {
							if (n.offset == 0
									|| _wordDelimiter->
									match(token.substr(n.offset-1, 1)))
								wordFactor += 0.5;
							if (n.offset + n.length == token.length()
									|| _wordDelimiter->match(
									token.substr(n.offset + n.length, 1)))
								wordFactor += 0.5;
							strengthFactor += (float)n.length/token.length()
									* wordFactor;
							wordFactor = 0.5;
							tokenMatches++;
							(**r).match(token, n.offset + n.length, n);
						}
					}
					else {
						isSubMatch = false;
						break;
					}
				}
				if (isSubMatch) {
					isMatch = true;
					strengthFactor += strengthFactor/tokenMatches;
					strength += strengthFactor * k->strength;
				}

			}
		}
		o = m.offset + m.length;
	}
	strength *= getExtensionFactor(url);

	if (isMatch)
		blacklistMatch.strength = (int)strength;
	return isMatch;
}



string Filter::abbrUrl(string boldUrl)
{
	RegularExpression a ("(^[^\\/]*\\/)([^<>]*)([^<>]{25}<b>)", 0, false),
		b ("(<\\/b>[^<>]{25})([^<>]*)([^<>]{25}<b>)", 0, false),
		c ("(<\\/b>[^<>]{25})([^<>]*$)", 0, false);
	a.subst(boldUrl, "$1 ... $3", RegularExpression::RE_GLOBAL);
	b.subst(boldUrl, "$1 ... $3", RegularExpression::RE_GLOBAL);
	c.subst(boldUrl, "$1 ...", RegularExpression::RE_GLOBAL);
	return boldUrl;
}



void Filter::setRegexps() {
	_splitToken =
			new RegularExpression("((\\?|\\&|;).*?\\=)|\\/|(\\%2F)|#", 0, true);
	_wordDelimiter = new RegularExpression("[\\s-_+\"']||\\.", 0, true);
	_splitExtension = new RegularExpression("/?(?:[^/?#]+/)+(?:[^?#]+\\.)"
			"([a-zA-Z0-9]{1,4})(?:$|\\?|#).*", 0, true);
}



float Filter::getExtensionFactor(string url) {
	if (_splitExtension->match(url)) {
		string ext = url;
		_splitExtension->subst(ext, "$1");
		for (Extensions::iterator it = _extensions.begin();
				it != _extensions.end(); it++)
		{
			if ((*it).re->match(ext))
				return (float)it->strength/100;
		}
	}
	return 1;
}
