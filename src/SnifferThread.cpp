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
// <SnifferThread> listens for HTTP requests, and makes sure they're getting logged
// Much of this code is inspired by examples provided by TCPDump and Winpcap


#include "SnifferThread.h"





SnifferThread::SnifferThread() {
	_logStream = &Sniffer::getLogStream();
	_options = &MainApplication::getOptions();
	_filter = &Sniffer::getFilter();
	_sniffPattern = (char*)"tcp[20:4] = 0x47455420 or tcp[32:4] = 0x47455420";
}



int SnifferThread::openDevice(string device) {
	pcap_if_t *alldevs,
		*d;
	u_int i = 0;
	struct bpf_program comp;
	bpf_u_int32 net;

	/* Do not check for the switch type ('-s') */
	if ((_fp = pcap_open_live(device.c_str(),	// name of the device
		65536,							// portion of the packet to capture.
										// 65536 grants that the whole packet will be captured on all the MACs.
		0,								// promiscuous mode (nonzero means promiscuous)
		1000,							// read timeout
		_errbuf							// error buffer
		)) == NULL)
	{
		*_logStream <<"Error opening device "
				<<device <<": " <<_errbuf <<endl;
		return -1;
	}

	/* compile the pattern */
	if (pcap_compile(_fp, &comp, _sniffPattern, 0, net) == -1) {
		*_logStream <<"Couldn't parse sniffPattern " <<_sniffPattern
				<<": " <<pcap_geterr(_fp) <<endl;
        return -1;
	}

	/* apply the compiled pattern */
	else if (pcap_setfilter(_fp, &comp) == -1) {
		*_logStream <<"Couldn't install sniffPattern " <<_sniffPattern
				<<": " <<pcap_geterr(_fp) <<endl;
		return -1;
	}

    return 0;
}



void SnifferThread::run() {
	int res,
		urlId;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	BlacklistMatch match;
	HTTPRequest request;
	request.setChunkedTransferEncoding(true);
	bool isMatch,
		isDebugging = Application::instance().config().getBool("debug", false);

	while((res = pcap_next_ex( _fp, &header, &pkt_data)) >= 0) {
		try {
			if (res == 0)
				continue;

			gotPacket(header, pkt_data, request);
			if (request.empty())
				throw Poco::Exception("No message found");

			isMatch = _filter->isMatch(request, match);
			Sniffer::logUrl(request);
			if (isMatch)
				Sniffer::logWarning(match);

			if (isDebugging)
				*_logStream <<isMatch <<endl;
		}
		catch (Poco::Exception &err) {
			if (isDebugging)
				*_logStream <<'-' <<endl;
		}
	}

	if(res == -1) {
		*_logStream <<"Error reading the packets: " <<pcap_geterr(_fp) <<endl;
		return;
	}

	pcap_close(_fp);
	return;
}


void SnifferThread::gotPacket(const struct pcap_pkthdr *header,
		const u_char *packet, HTTPRequest& request)
{
	request.clear();
	/* declare pointers to packet headers */
	const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
	const struct sniff_ip *ip;              /* The IP header */
	const struct sniff_tcp *tcp;            /* The TCP header */
	const char *payload;                    /* Packet payload */

	int size_ip,
		size_tcp,
		size_payload;

	/* define ethernet header */
	ethernet = (struct sniff_ethernet*)(packet);

	/* define/compute ip header offset */
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip) * 4;
	if (size_ip < 20) {
		return;
	}
	if (ip->ip_p == IPPROTO_IP) {
		size_tcp = SIZE_TCP; //Correct number? Always constant?
	}
	else if (ip->ip_p == IPPROTO_TCP) {
		/* define/compute tcp header offset */
		tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
		size_tcp = TH_OFF(tcp)*4;
		if (size_tcp < 20)
			size_tcp = SIZE_TCP;
	}
	else
		return;

	/* define/compute tcp payload (segment) offset */
	try {
		payload = (const char*)(packet + SIZE_ETHERNET + size_ip + size_tcp);
		if (*payload != 'G')		//Try alternative SIZE_TCP = 10
			payload = (const char*)(packet + SIZE_ETHERNET + size_ip + 10);

		istringstream istr(payload);
		request.read(istr);
	}
	catch (Poco::Exception &err) {
		request.clear();
	}
	return;
}
