#include "Poco/Environment.h"
#if defined(POCO_OS_FAMILY_UNIX)
#include "BootHistory.h"

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


BootHistory::BootHistory()
{
	_logger = &Application::instance().logger();
	_oldfmt = 0;		/* Use old libc5 format? */
	_show = (char*)"reboot"; /* Only show entries named reboot */
  char *altufile = NULL;/* Alternate wtmp */


	/* This is how to implement a custom wtmp-file:
		if((altufile = (char*)malloc(strlen(ALT_WTMP)+1)) == NULL) {
			throw; // Out of memory!
			exit(1);
		}
		strcpy(altufile, optarg);
	*/


  /*
   *	Which file do we want to read?
   */

  char *ufile = (char*)WTMP_FILE;
  if (altufile)
	ufile = altufile;
  time(&_lastdate);

  readFile(ufile);
}



void BootHistory::readFile(char *ufile) {
	struct utmplist *utmplist = NULL;
  FILE *fp;		/* Filepointer of wtmp file */


  struct utmp ut;	/* Current utmp entry */
  struct utmp oldut;	/* Old utmp entry to check for duplicates */
  struct utmplist *p;	/* Pointer into utmplist */
  struct utmplist *next;/* Pointer into utmplist */

  time_t lastboot = 0;  /* Last boottime */
  time_t lastrch = _lastdate;	/* Last run level change */
  time_t lastdown = _lastdate;
  time_t until = 0;	/* at what time to stop parsing the file */

  int whydown = 0;	/* Why we went down: crash or shutdown */

  int c, x;		/* Scratch */
//  struct stat st;	/* To stat the [uw]tmp file */
  int quit = 0;		/* Flag */
  int down = 0;		/* Down flag */

  /*
   *	Open the utmp file. Handle exceptions here!
   */
  if ((fp = fopen(ufile, "r")) == NULL) {
	x = errno;
	throw;
	/*fprintf(stderr, "%s: %s: %s\n", _progname, _ufile, strerror(errno));
	if (altufile == NULL && x == ENOENT)
		fprintf(stderr, "Perhaps this file was removed by the "
			"operator to prevent logging %s info.\n", _progname);*/
	exit(1);
  }

  /*
   *	Optimize the buffer size.
   */
  setvbuf(fp, NULL, _IOFBF, UCHUNKSIZE);

  /*
   *	Go to end of file minus one structure
   *	and/or initialize utmp reading code.
   */
  uread(fp, NULL, NULL);

  /*
   *	Read struct after struct backwards from the file.
   */
  while(!quit) {

	if (uread(fp, &ut, &quit) != 1)
		break;

	if (until && until < ut.ut_time)
		continue;

	if (memcmp(&ut, &oldut, sizeof(struct utmp)) == 0) continue;
	memcpy(&oldut, &ut, sizeof(struct utmp));
	_lastdate = ut.ut_time;

	/*
	 *	Set ut_type to the correct type.
	 */
	if (strncmp(ut.ut_line, "~", 1) == 0) {
		if (strncmp(ut.ut_user, "shutdown", 8) == 0)
			ut.ut_type = SHUTDOWN_TIME;
		else if (strncmp(ut.ut_user, "reboot", 6) == 0)
			ut.ut_type = BOOT_TIME;
		else if (strncmp(ut.ut_user, "runlevel", 8) == 0)
			ut.ut_type = RUN_LVL;
	}
#if 1 /*def COMPAT*/
	/*
	 *	For stupid old applications that don't fill in
	 *	ut_type correctly.
	 */
	else {
		if (ut.ut_type != DEAD_PROCESS &&
		    ut.ut_name[0] && ut.ut_line[0] &&
		    strcmp(ut.ut_name, "LOGIN") != 0)
			ut.ut_type = USER_PROCESS;
		/*
		 *	Even worse, applications that write ghost
		 *	entries: ut_type set to USER_PROCESS but
		 *	empty ut_name...
		 */
		if (ut.ut_name[0] == 0)
			ut.ut_type = DEAD_PROCESS;

		/*
		 *	Clock changes.
		 */
		if (strcmp(ut.ut_name, "date") == 0) {
			if (ut.ut_line[0] == '|') ut.ut_type = OLD_TIME;
			if (ut.ut_line[0] == '{') ut.ut_type = NEW_TIME;
		}
	}
#endif

	switch (ut.ut_type) {
		case SHUTDOWN_TIME:
			lastdown = lastrch = ut.ut_time;
			down = 1;
			break;
		case OLD_TIME:
		case NEW_TIME:
			break;
		case BOOT_TIME:
			strcpy(ut.ut_line, "system boot");
			quit = pushBoot(&ut, lastdown, R_REBOOT);
			lastboot = ut.ut_time;
			down = 1;
			break;
		case RUN_LVL:
			x = ut.ut_pid & 255;
			if (x == '0' || x == '6') {
				lastdown = ut.ut_time;
				down = 1;
				ut.ut_type = SHUTDOWN_TIME;
			}
			lastrch = ut.ut_time;
			break;

		case USER_PROCESS:
			/*
			 *	This was a login - show the first matching
			 *	logout record and delete all records with
			 *	the same ut_line.
			 */
			c = 0;
			for (p = utmplist; p; p = next) {
				next = p->next;
				if (strncmp(p->ut.ut_line, ut.ut_line,
				    UT_LINESIZE) == 0) {
					/* Show it */
					if (c == 0) {
						quit = pushBoot(&ut, p->ut.ut_time, R_NORMAL);
						c = 1;
					}
					if (p->next) p->next->prev = p->prev;
					if (p->prev)
						p->prev->next = p->next;
					else
						utmplist = p->next;
					free(p);
				}
			}
			/*
			 *	Not found? Then crashed, down, still
			 *	logged in, or missing logout record.
			 */
			if (c == 0) {
				if (lastboot == 0) {
					c = R_NOW;
					/* Is process still alive? */
					if (ut.ut_pid > 0 &&
					    kill(ut.ut_pid, 0) != 0 &&
					    errno == ESRCH)
						c = R_PHANTOM;
				} else
					c = whydown;
				quit = pushBoot(&ut, lastboot, c);
			}
			/* FALLTHRU */

		/*case DEAD_PROCESS:		DO NOT JUST COMMENT THIS PART OUT!
			*
			 *	Just store the data if it is
			 *	interesting enough.
			 *
			if (ut.ut_line[0] == 0)
				break;
			if ((p = malloc(sizeof(struct utmplist))) == NULL) {
				fprintf(stderr, "%s: out of memory\n",
					progname);
				exit(1);
			}
			memcpy(&p->ut, &ut, sizeof(struct utmp));
			p->next  = utmplist;
			p->prev  = NULL;
			if (utmplist) utmplist->prev = p;
			utmplist = p;
			break;
		*/

	}
	/*
	 *	If we saw a shutdown/reboot record we can remove
	 *	the entire current utmplist.
	 */
	if (down) {
		lastboot = ut.ut_time;
		whydown = (ut.ut_type == SHUTDOWN_TIME) ? R_DOWN : R_CRASH;
		for (p = utmplist; p; p = next) {
			next = p->next;
			free(p);
		}
		utmplist = NULL;
		down = 0;
	}
  }

  fclose(fp);
}



/*
 *	Convert old utmp format to new.
 */
void BootHistory::uconv(struct oldutmp *oldut, struct utmp *utn)
{
	memset(utn, 0, sizeof(struct utmp));
	utn->ut_type = oldut->ut_type;
	utn->ut_pid  = oldut->ut_pid;
	utn->ut_time = oldut->ut_oldtime;
	utn->ut_addr = oldut->ut_oldaddr;
	strncpy(utn->ut_line, oldut->ut_line, OLD_LINESIZE);
	strncpy(utn->ut_user, oldut->ut_user, OLD_NAMESIZE);
	strncpy(utn->ut_host, oldut->ut_host, OLD_HOSTSIZE);
}



#if NEW_UTMP
/*
 *	Read one utmp entry, return in new format.
 *	Automatically reposition file pointer.
 */
int BootHistory::uread(FILE *fp, struct utmp *u, int *quit)
{
	static int utsize;
	static char buf[UCHUNKSIZE];
	char tmp[1024];
	static off_t fpos;
	static int bpos;
	struct oldutmp uto;
	int r;
	off_t o;

	if (quit == NULL && u != NULL) {
		/*
		 *	Normal read.
		 */
		if (_oldfmt) {
			r = fread(&uto, sizeof(uto), 1, fp);
			uconv(&uto, u);
		} else
			r = fread(u, sizeof(struct utmp), 1, fp);
		return r;
	}

	if (u == NULL) {
		/*
		 *	Initialize and position.
		 */
		utsize = _oldfmt ? sizeof(uto) : sizeof(struct utmp);
		fseeko(fp, 0, SEEK_END);
		fpos = ftello(fp);
		if (fpos == 0)
			return 0;
		o = ((fpos - 1) / UCHUNKSIZE) * UCHUNKSIZE;
		if (fseeko(fp, o, SEEK_SET) < 0) {
			throw; // Seek failed!
			return 0;
		}
		bpos = (int)(fpos - o);
		if (fread(buf, bpos, 1, fp) != 1) {
			throw; // Read failed!
			return 0;
		}
		fpos = o;
		return 1;
	}

	/*
	 *	Read one struct. From the buffer if possible.
	 */
	bpos -= utsize;
	if (bpos >= 0) {
		if (_oldfmt)
			uconv((struct oldutmp *)(buf + bpos), u);
		else
			memcpy(u, buf + bpos, sizeof(struct utmp));
		return 1;
	}

	/*
	 *	Oops we went "below" the buffer. We should be able to
	 *	seek back UCHUNKSIZE bytes.
	 */
	fpos -= UCHUNKSIZE;
	if (fpos < 0)
		return 0;

	/*
	 *	Copy whatever is left in the buffer.
	 */
	memcpy(tmp + (-bpos), buf, utsize + bpos);
	if (fseeko(fp, fpos, SEEK_SET) < 0) {
		perror("fseek");
		return 0;
	}

	/*
	 *	Read another UCHUNKSIZE bytes.
	 */
	if (fread(buf, UCHUNKSIZE, 1, fp) != 1) {
		perror("fread");
		return 0;
	}

	/*
	 *	The end of the UCHUNKSIZE byte buffer should be the first
	 *	few bytes of the current struct utmp.
	 */
	memcpy(tmp, buf + UCHUNKSIZE + bpos, -bpos);
	bpos += UCHUNKSIZE;

	if (_oldfmt)
		uconv((struct oldutmp *)tmp, u);
	else
		memcpy(u, tmp, sizeof(struct utmp));

	return 1;
}

#else /* NEW_UTMP */

/*
 *	Read one utmp entry, return in new format.
 *	Automatically reposition file pointer.
 */
int BootHistory::uread(FILE *fp, struct utmp *u, int *quit)
{
	struct oldutmp uto;
	off_t r;

	if (u == NULL) {
		r = oldfmt ? sizeof(struct oldutmp) : sizeof(struct utmp);
		fseek(fp, -1 * r, SEEK_END);
		return 1;
	}

	if (!oldfmt) {
		r = fread(u, sizeof(struct utmp), 1, fp);
		if (r == 1) {
			if (fseeko(fp, -2 * sizeof(struct utmp), SEEK_CUR) < 0)
				if (quit) *quit = 1;
		}
		return r;
	}
	r = fread(&uto, sizeof(struct oldutmp), 1, fp);
	if (r == 1) {
		if (fseeko(fp, -2 * sizeof(struct oldutmp), SEEK_CUR) < 0)
			if (quit) *quit = 1;
		uconv(&uto, u);
	}

	return r;
}
#endif



/*
 *	Maybe this method is a little overkill?
 */
int BootHistory::pushBoot(struct utmp *p, time_t haltTime, int what)
{
	char	      utline[UT_LINESIZE+1];
	char        *show = (char*)"reboot";
	time_t   first, second;
	/*
	 *	Is this something we wanna show? E.g. is it "reboot"
	 */
	if (strncmp(p->ut_name, show, UT_NAMESIZE) == 0
		|| strcmp(utline, show) == 0
		|| (strncmp(utline, "tty", 3) == 0 && strcmp(utline + 3, show) == 0))
	{
		 first = (time_t)p->ut_time;   // Time booted
		 second = haltTime;            // Time halted
		 _boots.insert(pair<time_t, time_t>(first, second));
	}



	/* HANDLE THESE DIFFERENT TYPES ACCORDINGLY
	switch(what) {
		case R_CRASH:
			sprintf(logouttime, "- crash");
			break;
		case R_DOWN:
			sprintf(logouttime, "- down ");
			break;
		case R_NOW:
			length[0] = 0;
			if (fulltime)
				sprintf(logouttime, "  still logged in");
			else {
				sprintf(logouttime, "  still");
				sprintf(length, "logged in");
			}
			break;
		case R_PHANTOM:
			length[0] = 0;
			if (fulltime)
				sprintf(logouttime, "  gone - no logout");
			else {
				sprintf(logouttime, "   gone");
				sprintf(length, "- no logout");
			}
			break;
		case R_REBOOT:
			break;
		case R_TIMECHANGE:
			logouttime[0] = 0;
			length[0] = 0;
			break;
		case R_NORMAL:
			break;
 	}
 	*/

	return 0;
}



int BootHistory::getBootTime() {
	time_t nowTime;
	time(&nowTime);
	for (map<time_t, time_t>::reverse_iterator it = _boots.rbegin();  // Iterate from the back
			it != _boots.rend(); it++)
	{
		// Check if this is the running session
		if (nowTime - it->second < 5) {
			return (int)it->first;
		}
	}
	return 0;
}



int BootHistory::getHaltTime(int bootTime) {
	// Returns the halt time according to bootTime or 0 on failure.
	time_t boot = (time_t)bootTime,
		halt;
	// Only store if the halt time is found, but not this current session
	if (_boots.find(boot) != _boots.end() && bootTime != getBootTime()) {
		halt = _boots[boot];
		for (map<time_t, time_t>::reverse_iterator it = _boots.rbegin();
				it != _boots.rend(); it++)
		{
			// Check if there is any later session with
			// the same halt time. Then this session crashed
			if (it->second == halt && it->first > boot) {
				return 0;
			}
		}
		return (int)_boots[boot];
	}
	return 0;
}



map<int, int> BootHistory::getOmittedSessions(set<int> logged) {
	map<int, int> omitted;
	bool isFreshEnough = false;
	for (map<time_t, time_t>::iterator it = _boots.begin();
			it != _boots.end(); it++)
	{
		int boot = (int)it->first;
		int halt = (int)it->second;
		if (isFreshEnough && logged.find(boot) == logged.end()
				&& boot != getBootTime())
		{
			omitted.insert(pair<int, int>(boot, halt));
		}
		else if (!isFreshEnough && logged.find(boot) != logged.end())
			isFreshEnough = true;
	}
	return omitted;
}





#endif // UNIX
