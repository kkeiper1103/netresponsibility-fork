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
// <ReportBase> sets up basic functionality for making/sending reports. Must be
// inherited by <Report>



#include "ReportBase.h"



ReportBase::ReportBase()
{
	_logger = &Application::instance().logger();
	_db = &MainApplication::getDatabase();
	_options = &MainApplication::getOptions();
	_contentType = "text/plain";
	_subject = _options->getName() + "'s Net Responsibility Report";

	_db->logReportStart(_reportId);
}



ReportBase::ReportBase(const ReportBase &r)
{
	_db = r._db;
	_options = r._options;
	_attachments = r._attachments;
	_contentType = r._contentType;
	_subject = r._subject;
	_reportId = r._reportId;
}



ReportBase::~ReportBase()
{
	//dtor
}



string ReportBase::getBody() const {
	return _body.str();
}



string ReportBase::getSubject() const {
	return _subject;
}



string ReportBase::getContentType() const {
	return _contentType;
}



void ReportBase::logFinish() {
	_db->rotateLog(_reportId);
	_db->logReportFinish(_reportId);
}



void ReportBase::install() {
	string txt = _options->getTxt("reportInstall");
	replaceVar(txt, "name", _options->getName());
	_body << txt;
	_subject = _options->getName() + " has installed Net Responsibility";
	_contentType = "text/html";
}



void ReportBase::uninstall() {
	string txt = _options->getTxt("reportUninstall");
	replaceVar(txt, "name", _options->getName());
	_body << txt;
	_subject = _options->getName() + " has uninstalled Net Responsibility";
	_contentType = "text/html";
}



int ReportBase::send(bool receiveCopy) {
	try {
		MailMessage message;
		message.setSender(_options->getName() + " <report@netresponsibility.com>");
		message.set("Reply-To", _options->getEmailFrom());
		vector<string> recipients = _options->getEmailTo();
		if (receiveCopy)
			recipients.push_back(_options->getEmailFrom());
		for (vector<string>::iterator it = recipients.begin();
				it != recipients.end(); it++)
		{
			message.addRecipient(
					MailRecipient(MailRecipient::PRIMARY_RECIPIENT, *it));
		}
		message.setSubject(_subject);
		message.addContent(new StringPartSource(getBody(), _contentType));
		for (vector<Path>::iterator it = _attachments.begin();
				it != _attachments.end(); it++)
		{
			string mimeType;
			if (it->getExtension() == "htm")
				mimeType = "text/html";
			else if (it->getExtension() == "zip")
				mimeType = "application/zip";
			else
				mimeType = "application/octet-stream";
			message.addAttachment(it->getFileName(),
					new FilePartSource(it->toString(), mimeType));
		}

		SMTPClientSession session("send.one.com", 2525);
		session.login(SMTPClientSession::AUTH_LOGIN,
				"report@netresponsibility.com",
				"407298f00758c47a635065f7bfa1954d");
		session.sendMessage(message);
		session.close();
		sendImprovementData();
	}
	catch (Poco::Net::NoAddressFoundException& exc) {
		return 1;
	}
	catch (Exception& exc) {
		_logger->warning(exc.displayText());
		return 2;
	}
	return 0;
}


void ReportBase::sendCout() {
	//sendImprovementData();

	cout <<endl <<"--------------SENDING REPORT-------------" <<endl
		<<getBody() <<endl
		<<"--------------END OF REPORT--------------" <<endl;
}



void ReportBase::sendToFile(char* filename) {
	ofstream f(filename, ios::out);
	StreamCopier::copyStream(_body, f);
	f.close();
	string msg(filename);
	msg = "Saved the report in " + msg;
	_logger->information(msg);
}



void ReportBase::test() {
	string txt = _options->getTxt("reportTest");
	replaceVar(txt, "name", _options->getName());
	_body << txt;
	_subject = _options->getName() + "'s Test Report";
	_contentType = "text/html";
	send(true);
}



void ReportBase::sendImprovementData() {
	Warnings warnings = _db->getWarnings();
	if (_options->doSendImprovementData() && warnings.size() > 0) {
		string impData = "";
		stringstream strength;
		vector<BlacklistKeyword> keywords;
		while (warnings.hasMore()) {
			keywords = warnings.getKeywords();
			for(vector<BlacklistKeyword>::iterator it = keywords.begin();
					it != keywords.end(); it++)
			{
				impData += it->category + "¤" + it->asString + "¤";
				URI::encode(warnings.getBoldUrl(), "&'\"<>", impData);
				impData += "¤";
				URI::encode(warnings.getUrl(), "&'\"<>", impData);
				impData += "¤";
				URI::encode(warnings.getAbbrUrl(), "&'\"<>", impData);
				strength.str("");
				strength <<"¤" <<warnings.getStrength();
				impData += strength.str();
				if (it + 1 != keywords.end())
					impData += "\n";
			}
			warnings.next();
			if (warnings.hasMore())
				impData += "\n";
		}
		_logger->information(Request::sendImprovementData(_options, impData));
	}
}



void ReportBase::replaceVar(string &subject, string var, string replacement) {
	var = "{" + var + "}";
	int length = var.length(),
		pos = subject.find(var);
	while (pos != string::npos) {
		subject.replace(pos, length, replacement);
		pos = subject.find(var);
	}
}
