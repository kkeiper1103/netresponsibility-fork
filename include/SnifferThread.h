//
// Library: Net Responsibility
// Package: Core
// Module:  SnifferThread
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
// <SnifferThread> listens for HTTP requests, and makes sure they're getting logged
// Much of this code is inspired by examples provided by TCPDump and Winpcap



#ifndef SNIFFERTHREAD_H
#define SNIFFERTHREAD_H

#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
#define _CRT_SECURE_NO_WARNINGS
#endif


#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sstream>

#include "Poco/RegularExpression.h"
#include "Poco/SharedPtr.h"
#include "Poco/Runnable.h"
#include "Poco/Logger.h"
#include "Poco/LogStream.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Exception.h"

#include "Database.h"
#include "Options.h"
#include "Filter.h"
#include "Blacklist.h"
#include "Sniffer.h"

#include <pcap.h>

#if defined(POCO_OS_FAMILY_WINDOWS)
#include <winsock2.h>
#elif defined (POCO_OS_FAMILY_UNIX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif //POCO_OS_FAMILY_


#define LINE_LEN 32

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* This TCP header length seems to work. */
#define SIZE_TCP 22

#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

using Poco::RegularExpression;
using Poco::SharedPtr;
using Poco::Logger;
using Poco::LogStream;
using Poco::Net::HTTPRequest;
using Poco::Exception;
using namespace std;

struct sniff_ethernet;
struct sniff_ip;
struct sniff_tcp;

class MainApplication;

class SnifferThread: public Poco::Runnable
	/// SnifferThread is a thread that listens for HTTP requests on a specific
	/// interface, and makes sure they're getting logged. It is invoked by Sniffer.
	/// Much of this code is inspired by examples provided by TCPDump and Winpcap
{
	public:
		SnifferThread();
			/// Constructs SnifferThread and sets up some default values

		virtual void run();
			/// Run the SnifferThread.

		int openDevice(string device);
			/// Open the given device.

	private:
		pcap_t *_fp;
		char _errbuf[PCAP_ERRBUF_SIZE];
		char *_sniffPattern;
		Options *_options;
		Filter *_filter;
		LogStream *_logStream;

		void gotPacket(const struct pcap_pkthdr*, const u_char*, HTTPRequest&);
};

/* Ethernet header */
struct sniff_ethernet {
	u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
	u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
	u_short ether_type;                     /* IP? ARP? RARP? etc */
};



/* IP header */
struct sniff_ip {
	u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
	u_char  ip_tos;                 /* type of service */
	u_short ip_len;                 /* total length */
	u_short ip_id;                  /* identification */
	u_short ip_off;                 /* fragment offset field */
#define IP_RF 0x8000            /* reserved fragment flag */
#define IP_DF 0x4000            /* dont fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
	u_char  ip_ttl;                 /* time to live */
	u_char  ip_p;                   /* protocol */
	u_short ip_sum;                 /* checksum */
	struct  in_addr ip_src,ip_dst;  /* source and dest address */
};

struct sniff_tcp {
	u_short th_sport;               /* source port */
	u_short th_dport;               /* destination port */
	tcp_seq th_seq;                 /* sequence number */
	tcp_seq th_ack;                 /* acknowledgement number */
	u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
	u_char  th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS   (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;                 /* window */
	u_short th_sum;                 /* checksum */
	u_short th_urp;                 /* urgent pointer */
};

#include "MainApplication.h"
#endif // SNIFFERTHREAD_H
