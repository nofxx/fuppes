#! /bin/sh
### BEGIN INIT INFO
# Provides:          fuppesd
# Required-Start:    $local_fs $remote_fs
# Required-Stop:     $local_fs $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      S 0 1 6
# Short-Description: UPnP Media Server
# Description:       Fuppes UPnP media server.
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
DESC="UPnP Media Server"
NAME=fuppesd
DAEMON=`which $NAME`
DAEMON_ARGS="--config-dir /etc/fuppes/ --database-file /var/lib/fuppes/fuppes.db --log-level 1"
SCRIPTNAME=/etc/init.d/$NAME
DAEMONUSER=fuppes
DAEMONGROUP=fuppes

# Time to start and die
STARTTIME=2
DIETIME=10

# Exit if the package is not installed
if [ ! -x "$DAEMON" ]; then
{
        echo "Couldn't find $DAEMON"
        exit 99
}
fi

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions

# Check that the user exists (if we set a user)
# Does the user exist?
if [ -n "$DAEMONUSER" ] ; then
    if getent passwd | grep -q "^$DAEMONUSER:"; then
        # Obtain the uid and gid
        DAEMONUID=`getent passwd |grep "^$DAEMONUSER:" | awk -F : '{print $3}'`
        DAEMONGID=`getent passwd |grep "^$DAEMONUSER:" | awk -F : '{print $4}'`
    else
        log_failure_msg "The user $DAEMONUSER, required to run $NAME does not exist."
        exit 1
    fi
fi

#
# Function that starts the daemon/service
#
do_start()
{
	# Return
	#   0 if daemon has been started
	#   1 if daemon was already running
	#   2 if daemon could not be started
	start-stop-daemon --start --quiet --chuid $DAEMONUID:$DAEMONGID --exec $DAEMON --test > /dev/null \
		|| return 1
	start-stop-daemon --start --quiet --chuid $DAEMONUID:$DAEMONGID --exec $DAEMON -- $DAEMON_ARGS \
		|| return 2
}

#
# Function that stops the daemon/service
#
do_stop()
{
	# Return
	#   0 if daemon has been stopped
	#   1 if daemon was already stopped
	#   2 if daemon could not be stopped
	#   other if a failure occurred
	start-stop-daemon --stop --signal 2 --retry $DIETIME --quiet --name $NAME
	RETVAL="$?"
	[ "$RETVAL" = 2 ] && return 2
	return "$RETVAL"
}

case "$1" in
  start)
	log_daemon_msg "Starting $DESC" "$NAME"
	do_start
  RETVAL="$?"
  [ -n "$STARTTIME" ] && sleep $STARTTIME
	case "$RETVAL" in
		0|1) log_end_msg 0 ;;
		2) log_end_msg 1 ;;
	esac
	;;
  stop)
	log_daemon_msg "Stopping $DESC" "$NAME"
	do_stop
  RETVAL="$?"
  [ -n "$DIETIME" ] && sleep $DIETIME
	case "$RETVAL" in
		0|1) log_end_msg 0 ;;
		2) log_end_msg 1 ;;
	esac
	;;
  restart|force-reload)
	log_daemon_msg "Restarting $DESC" "$NAME"
	do_stop
  RETVAL="$?"
  [ -n "$DIETIME" ] && sleep $DIETIME
	case "$RETVAL" in
	  0|1)
		do_start
    RETVAL="$?"
    [ -n "$STARTTIME" ] && sleep $STARTTIME
		case "$RETVAL" in
			0) log_end_msg 0 ;;
			1) log_end_msg 1 ;; # Old process is still running
			*) log_end_msg 1 ;; # Failed to start
		esac
		;;
	  *)
	  	# Failed to stop
		log_end_msg 1
		;;
	esac
	;;
  *)
	echo "Usage: $SCRIPTNAME {start|stop|restart|force-reload}" >&2
	exit 3
	;;
esac

:
