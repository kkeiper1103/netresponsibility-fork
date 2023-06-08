//
// Library: Net Responsibility
// Package: Core
// Module:  BootHistory
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
// <BootHistory> processes the computers boot history to determine whether or
// not Net Responsibility have been shut down manually.
// Almost all code is taken from the Linux command "last", so credit goes to
// Miquel van Smoorenburg, miquels@cistron.nl

#ifndef BOOTHISTORY_H
#define BOOTHISTORY_H

#include "Poco/Environment.h"
#include <map>
#include <set>

#if defined(POCO_OS_FAMILY_UNIX)

#include <time.h>
#include <utmp.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

using namespace std;

#include "Poco/Timestamp.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

using Poco::Timestamp;
using Poco::Logger;
using Poco::Util::Application;

#define OLD_LINESIZE		12
#define OLD_NAMESIZE		8
#define OLD_HOSTSIZE		16


#ifndef SHUTDOWN_TIME
#  define SHUTDOWN_TIME 254
#endif

#define CHOP_DOMAIN	0	/* Define to chop off local domainname. */
#define NEW_UTMP	1	/* Fancy & fast utmp read code. */
#define UCHUNKSIZE	16384	/* How much we read at once. */

/* Types of listing */
#define R_CRASH		1 /* No logout record, system boot in between */
#define R_DOWN		2 /* System brought down in decent way */
#define R_NORMAL	3 /* Normal */
#define R_NOW		4 /* Still logged in */
#define R_REBOOT	5 /* Reboot record. */
#define R_PHANTOM	6 /* No logout record but session is stale. */
#define R_TIMECHANGE	7 /* NEW_TIME or OLD_TIME */

struct utmplist {
	/// Double linked list of struct utmp's
	struct utmp ut;
	struct utmplist *next;
	struct utmplist *prev;
};

struct oldutmp {
	short	ut_type;
	int	ut_pid;
	char	ut_line[OLD_LINESIZE];
	char	ut_id[4];
	long	ut_oldtime;
	char	ut_user[OLD_NAMESIZE];
	char	ut_host[OLD_HOSTSIZE];
	long	ut_oldaddr;
};


class BootHistory
	/// BootHistory processes the computer's boot history to determine whether or
	/// not Net Responsibility have been shut down manually.
	///
	/// Almost all code is taken from the Linux command "last", so credit goes to
	/// Miquel van Smoorenburg, miquels@cistron.nl
{
	public:
		BootHistory();
		int getBootTime();
			/// Get the unix timestamp of when the current session booted.

		int getHaltTime(int bootTime);
			/// Get the unix timestamp of when the session with the given bootTime
			/// halted or 0 on failure.

		map<int, int> getOmittedSessions(set<int> logged);
			/// Returns all sessions that are not logged for some reason.
			/// Only the sessions with a boot time later than the first element
			/// of logged.

	private:

		void readFile(char*);
		void uconv(struct oldutmp *oldut, struct utmp *utn);
		int uread(FILE *fp, struct utmp *u, int *quit);
		int pushBoot(struct utmp *p, time_t haltTime, int what);


		int _oldfmt;		/* Use old libc5 format? */
		time_t _lastdate;	/* Last date we've seen */
		char *_show;
		map<time_t, time_t> _boots;
		Logger *_logger;


};

#else // NOT UNIX
//This is only a fake class for Windows and other platforms, not yet implemented
class BootHistory
	/// BootHistory processes the computer's boot history to determine whether or
	/// not Net Responsibility have been shut down manually.
	///
	/// Almost all code is taken from the Linux command "last", so credit goes to
	/// Miquel van Smoorenburg, miquels@cistron.nl
{
	public:
		BootHistory() {}
		int getBootTime() { return 0; }
			/// Get the unix timestamp of when the current session booted.

		int getHaltTime(int bootTime) { return 0; }
			/// Get the unix timestamp of when the session with the given bootTime
			/// halted.

		map<int, int> getOmittedSessions(set<int> logged) { return map<int, int>; }
			/// Returns all sessions that are not logged for some reason.
			/// Only the sessions with a boot time later than the first element
			/// of logged.
};
#endif // UNIX
#endif // BOOTHISTORY_H
