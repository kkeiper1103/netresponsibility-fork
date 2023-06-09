#!/bin/sh
# Start/stop/reset the Net Responsibility daemon.
#
### BEGIN INIT INFO
# Provides:          net-responsibility
# Required-Start:    $syslog $time
# Required-Stop:     $syslog $time
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Logs all URLs visited
# Description:       Net Responsibility is a program that logs all URLs
#                    visited for the purpose of accountability. It
#                    periodically creates a report and e-mails it to
#                    any number of user-configured e-mail addresses.
### END INIT INFO

. /lib/lsb/init-functions

PIDDIR=/var/run


start() {
        echo 'Starting net-responsibility' | logger -sp daemon.info -t '/etc/init.d/net-responsibility'
        net-responsibility --daemon
}

stop() {
        echo 'Stopping net-responsibility' | logger -sp daemon.info -t '/etc/init.d/net-responsibility'
        if [ -f $PIDDIR/net-responsibility.pid ]; then
          pid=`cat $PIDDIR/net-responsibility.pid`
          kill -TERM "$pid"
        fi
}

reset() {
        # signal 10 == SIGUSR1
        if ! [ -f $PIDDIR/net-responsibility.pid ]; then
          start
          sleep 1
        fi
        pid=`cat $PIDDIR/net-responsibility.pid`
        kill -USR1 "$pid"
}

case "$1" in
start)	log_daemon_msg "Starting the Net Responsibility daemon" "net-responsibility"
        start
        status=$?
        log_end_msg $status
        exit $status
	;;
stop)	log_daemon_msg "Stopping the Net Responsibility daemon" "net-responsibility"
        stop
        status=$?
        log_end_msg $status
        exit $status
        ;;
restart) log_daemon_msg "Restarting the Net Responsibility daemon" "net-responsibility"
        stop
        sleep 1
        start
        log_end_msg $?
        ;;
reload|force-reload) log_daemon_msg "Resetting the Net Responsibility daemon" "net-responsibility"
        reset
        log_end_msg 0
        ;;
*)	log_action_msg "Usage: /etc/init.d/net-responsibility {start|stop|restart|reload|force-reload}"
        exit 2
        ;;
esac
exit 0
