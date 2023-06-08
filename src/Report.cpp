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
// <Report> generates the reports, and may override <ReportBase>'s send method.



#include "Report.h"



string Report::name() const {
	return "Report";
}



void Report::generate() {
	_contentType = "text/html";
	string suspicious;
	makeBypassesSection();
	makeWarningsSection(suspicious);
	makeWhitelistSection();
	makeHistorySection();
	addTemplate(suspicious);
	saveAttachedReport();
}



void Report::makeBypassesSection() {
	Bypasses bypasses = _db->getBypasses();
	try {
		if (bypasses.size() > 0) {
			string content = "",
				attContent = "",
				details;
			while (bypasses.hasMore()) {
				details = bypasses.getDetails();
				content += "<i>" + bypasses.getDateTime(
						_options->getTxt("dateTimeFormat")) + "</i> - "
						+ bypasses.getTypeString()
						+ (details != "" ? " (" + details + ")" : "");
				attContent += "['" + jsContent(bypasses.getTypeString())
						+ (details != "" ? " (" + jsContent(details) + ")" : "")
						+ "', [''],, '" + jsContent(bypasses.getDateTime(
								_options->getTxt("dateTimeFormat"))) + "']";
				bypasses.next();
				if (bypasses.hasMore()) {
					content += "<br>";
					attContent += ",\n";
				}

			}
			if (_options->isReportPart("bypasses"))
				_body <<makeTableBranch(
						_options->getTxt("reportBypassesTitle"), content);
			if (_options->isAttachedReportPart("bypasses"))
				_attached <<makeJavascriptBranch(
						jsContent(_options->getTxt("reportBypassesTitle")),
								attContent);
		}
	}
	catch (Exception &err) {
		_logger->warning(err.displayText());
	}
}



void Report::makeWarningsSection(string &suspicious) {
	RegularExpression getAnchorName("[^a-zA-Z]", 0, true);
	string keywordsContent = "",
		attKeywordsContent = "",
		key,
		anchorName;
	stringstream where;
	where <<"strength >= " <<_options->getReportStrengthThreshold();
	Warnings warnings = _db->getWarnings(where.str());
	if (warnings.size() > 0) {
		map<string, vector<int> > tree;
		stringstream urlsContent,
			attUrlsContent;
		vector<BlacklistKeyword> keywords;
		while (warnings.hasMore()) {
			keywords = warnings.getKeywords();
			for(vector<BlacklistKeyword>::iterator it = keywords.begin();
					it != keywords.end(); it++)
			{
				key = it->asString + " (" + it->category + ")";
				tree[key].push_back(warnings.getIndex());
			}
			warnings.next();
		}

		for(map<string, vector<int> >::iterator k = tree.begin();
				k != tree.end(); k++)
		{
			urlsContent.str("");
			attUrlsContent.str("");
			anchorName = k->first;
			getAnchorName.subst(anchorName, "_", RegularExpression::RE_GLOBAL);
			for(vector<int>::iterator id = k->second.begin();
					id != k->second.end(); id++)
			{
				urlsContent <<makeColoredStrength(warnings.getStrength(*id))
						<<" [<a href='http://" <<warnings.getUrl(*id) <<"'>"
						<<_options->getTxt("reportGoToUrl") <<"</a>] "
						<<warnings.getAbbrUrl(*id);
				attUrlsContent <<"['" <<jsContent(warnings.getAbbrUrl(*id))
						<<"', ['http://" <<jsContent(warnings.getUrl(*id)) <<"'],, '"
						<<jsContent(warnings.getDateTime(
								_options->getTxt("dateTimeFormat"),*id))
						<<"', " <<warnings.getStrength(*id) <<"]";
				if (id != k->second.end()) {
					urlsContent <<"<br>";
					attUrlsContent <<",";
				}
			}
			suspicious += "<li><a href='#" + anchorName
					+ "'>" + k->first + "</li>";
			keywordsContent
					+= makeTableBranch(k->first, urlsContent.str(), anchorName);
			attKeywordsContent
					+= makeJavascriptBranch(k->first, attUrlsContent.str());
		}
	}
	else {
		keywordsContent = _options->getTxt("reportNoWarnings");
		attKeywordsContent += "['"
				+ jsContent(_options->getTxt("reportNoWarnings")) + "', ['']]";
	}

	suspicious = "<ul>" + suspicious + "</ul>";
	if (_options->isReportPart("warnings"))
		_body <<makeTableBranch(
				_options->getTxt("reportWarningsTitle"), keywordsContent);
	if (_options->isAttachedReportPart("warnings"))
		_attached <<makeJavascriptBranch(
				jsContent(_options->getTxt("reportWarningsTitle")),
						attKeywordsContent);
}



void Report::makeWhitelistSection() {
	Warnings whitelist = _db->getWhitelist();
	if (whitelist.size() > 0) {
		string urlsContent = "", attUrlsContent = "";
		while (whitelist.hasMore()) {
			urlsContent += "[<a href='http://" + whitelist.getUrl() + "'>"
					+ _options->getTxt("reportGoToUrl") + "</a>] "
					+ whitelist.getAbbrUrl();
			attUrlsContent += "['" + jsContent(whitelist.getAbbrUrl())
					+ "', ['http://" + jsContent(whitelist.getUrl()) + "'],, '"
					+ jsContent(whitelist.getDateTime(
							_options->getTxt("dateTimeFormat"))) + "']";
			whitelist.next();
			if (whitelist.hasMore()) {
				urlsContent += "<br>";
				attUrlsContent += ",\n";
			}

		}
		if (_options->isReportPart("whitelist"))
			_body <<makeTableBranch(
					_options->getTxt("reportWhitelistTitle"), urlsContent);
		if (_options->isAttachedReportPart("whitelist"))
			_attached <<makeJavascriptBranch(
					jsContent(_options->getTxt("reportWhitelistTitle")),
							attUrlsContent);
	}
}



void Report::makeHistorySection() {
	bool doIncludePaths = _options->isAttachedReportPart("history_paths");

	RegularExpression secondLevel("(^\\d+\\.\\d+\\.\\d+\\.\\d+"
			"(:\\d*)?$)|(([^\\.]+\\.)([^\\.]+)$)", 0, true);
	string s = "",
		historyContent = "",
		domainsContent,
		pathsContent;
	set<string> secondLevelDomains;

	vector<string> hostnames = _db->getDistinctHostnames();
	if (hostnames.size() > 0) {
		for (vector<string>::iterator it = hostnames.begin();
				it != hostnames.end(); it++)
		{
			if (secondLevel.extract(*it, s, 0))
				secondLevelDomains.insert(s);
			else
				secondLevelDomains.insert(*it);
		}

		for(set<string>::iterator it = secondLevelDomains.begin();
				it != secondLevelDomains.end(); it++)
		{
			domainsContent = "";
			hostnames = _db->getDistinctHostnames("hostname LIKE '%"
					+ *it + "'", "hostname");
			for (vector<string>::iterator it2 = hostnames.begin();
					it2 != hostnames.end(); it2++)
			{
				if (doIncludePaths) {
					pathsContent = "";

					History history = _db->getHistory("hostname = '"
							+ *it2 + "'", "path");
					while (history.hasMore()) {
						pathsContent += "['" + jsContent(history.getUrl())
								+ "', ['http'],, '"
								+ jsContent(history.getDateTime(
										_options->getTxt("dateTimeFormat")))
								+ "'],\n";
						history.next();
					}

					if (*it == *it2 && hostnames.size() == 1)
						domainsContent += pathsContent;
					else
						domainsContent += makeJavascriptBranch(*it2, pathsContent);
				}
				else
					domainsContent += "['" + jsContent(*it2) + "', ['http',,'folder'],, ''],\n";
			}
			historyContent += makeJavascriptBranch(jsContent(*it), domainsContent);
		}
	}
	else
		historyContent += "['" + jsContent(_options->getTxt("reportNoHistory"))
				+ "', ['']]";
	if (_options->doSaveHistory())
		_attached <<makeJavascriptBranch(jsContent(_options
				->getTxt("reportHistoryTitle")), historyContent);
}



void Report::addTemplate(string suspicious) {
	string bodyContent = _body.str(),
			attachedContent = _attached.str();
	_body.str("");
	_attached.str("");

	_body <<"<a name='top'><h1>" <<_subject <<"</h1></a>" <<endl
			<<_options->getTxt("reportGeneratedBy") <<"<br />" <<endl
			<<_options->getTxt("reportTroubleshooting") <<"<br />" <<endl
			<<"<br />" <<endl
			<<_options->getTxt("reportNoReply") <<"<br />" <<endl
			<<(suspicious != "" ? _options->getTxt("reportSuspiciousInfo")
					+ suspicious : "") <<endl
			<<"<hr />" <<endl

			<<bodyContent;

	_attached << _options->getTxt("attachedReportHeader") <<endl
			<<"var subject = \"" <<_subject <<"\";" <<endl
			<<"var version = \"" <<_options->getVersion() <<"\";" <<endl
			<<"var arrNodes = [['Report', ['',,'folder'], [" <<endl

			<<attachedContent <<endl

			<<"]]];" <<endl
			<< _options->getTxt("attachedReportFooter");
}


void Report::saveAttachedReport() {
	try {
		string date = DateTimeFormatter::format(Timestamp(), "%Y%m%d"),
				iStr, dir(REPORT_DIR);
		int i = 0, tempInt;
		RegularExpression patt("^report_"
				+ date + "_(\\d+)\\.(htm|zip)$", 0, true);
		if (!File(dir).exists())
			File(dir).createDirectory();
		DirectoryIterator it(dir), end;
		while (it != end) {
			if (patt.match(it.name())) {
				iStr = it.name();
				patt.subst(iStr, "$1");
				stringstream ss(iStr);
				ss >> tempInt;
				if (tempInt > i)
					i = tempInt;
			}
			else
				it->remove();
			++it;
		}
		stringstream ss;
		ss <<"report_" <<date <<"_" << ++i;
		string fname(dir + ss.str() + ".htm");
		ofstream htmFile(fname.c_str(), ios::out);
		StreamCopier::copyStream(_attached, htmFile);
		htmFile.close();
		int fsize = File(fname).getSize();
		if (_options->doCompressAttachedReport() || fsize > (10 * 1024 * 1024)) {
			string zfname(dir + ss.str() + ".zip");
			ofstream zipFile(zfname.c_str(), std::ios::binary);
			Compress c(zipFile, true);
			Path fullPath(fname), shortPath(ss.str() + ".htm");
			if (fullPath.isFile()) {
				c.addFile(fullPath, shortPath);
				c.close();
				zipFile.close();
				_attachments.push_back(Path(zfname));
			}
		}
		else
			_attachments.push_back(Path(fname));
	}
	catch (Exception &exc) {
		cout <<exc.displayText() <<endl;
	}

}



string Report::jsContent(string str) {
	size_t pos = str.find("'");
	while (pos != string::npos) {
		str.replace(pos, 1, "\\'");
		pos = str.find("'", pos + 2);
	}
	pos = str.find("\n");
	while (pos != string::npos) {
		str.replace(pos, 1, "\\n");
		pos = str.find("\n", pos + 2);
	}
	return str;
}



string Report::makeColoredStrength(int strength) {
	stringstream colored;
	int red = 0,
		green = 0;
	string hex = "";
	if (strength < 32) {
		green = 255;
		red = strength * 8;
	}
	else {
		red = 255;
		green = 255 - (strength - 32) * 8;
	}
	green = (green < 0 ? 0 : (green > 255 ? 255 : green));
	red = (red < 0 ? 0 : (red > 255 ? 255 : red));
	Poco::NumberFormatter::appendHex(hex, red, 2);
	Poco::NumberFormatter::appendHex(hex, green, 2);
	Poco::NumberFormatter::appendHex(hex, 0, 2);

	colored <<"<b style='background-color: #" <<hex
			<<"'>&nbsp;" <<strength <<"&nbsp;</b>";
	return colored.str();
}



string Report::makeTableBranch(string title, string content, string anchorName){
	return "<table><tr><td colspan='2'><b>" + title + "</b> - <a href='#top'"
			+ (anchorName != "" ? " name='" + anchorName + "'" : "")
			+ ">" + _options->getTxt("reportBackToTop") + "</a></td></tr>\
			<tr><td width='10px'>&nbsp;</td><td>"
			+ content + "</td></tr></table><br>";
}



string Report::makeJavascriptBranch(string title, string content) {
	return "['" + title + "', ['',,'folder'], [\n" + content + "]],\n";
}
