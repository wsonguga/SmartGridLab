#!/bin/sh
# 
# File:   scoreplusinit.bash
# Author: stan
#
# Created on Jul 23, 2013, 5:26:15 PM
#
DEB=no
# Source function library.
if [ -f /etc/init.d/functions ] ; then
  . /etc/init.d/functions
elif [ -f /etc/rc.d/init.d/functions ] ; then
  . /etc/rc.d/init.d/functions
elif [ -f /lib/lsb/init-functions ] ; then
  . /lib/lsb/init-functions
  DEB=yes
else
  exit 1
fi

cored=
for p in /usr/local/sbin \
         /usr/sbin \
         /sbin \
         /usr/local/bin \
         /usr/bin \
         /bin
do
  if [ -e $p/cored ] ; then
    cored=$p/cored
    break
  fi
done


pathmunge () {
    if ! echo $PATH | /bin/egrep -q "(^|:)$1($|:)" ; then
       if [ "$2" = "after" ] ; then
          PATH=$PATH:$1
       else
          PATH=$1:$PATH
       fi
    fi
}

# these lines add to the PATH variable used by CORE and its containers
# you can add your own pathmunge statements to change the container's PATH
pathmunge "/usr/local/sbin"
pathmunge "/usr/local/bin"

RETVAL=0

# the /etc/init.d/functions (RedHat) differs from
#     /usr/lib/init-functions (Debian)
if [ $DEB = yes ]; then
  daemon=start_daemon
  status=status_of_proc
  msg () {
    log_daemon_msg "$@"
  }
  endmsg () {
    echo -n ""
  }
else
  daemon=daemon
  status=status
  msg () {
    echo -n $"$@"
  }
  endmsg () {
    echo ""
  }
fi
start() {
	msg "Starting SCOREPLUS service"
	powersimulation
	powerslave
	export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
	coordinator
	$daemon /usr/bin/python $cored -d
	RETVAL=$?
	endmsg
	return $RETVAL
}	

stop() {
	msg "Shutting down SCOREPLUS service"
	kill `ps h -o pid -C powersimulation`
	kill `ps h -o pid -C powerslave`
	kill `ps h -o pid -C coordinator`
	killproc -p /var/run/cored.pid $cored
	RETVAL=$?
	endmsg
	return $RETVAL
}	

restart() {
	stop
	start
}	



case "$1" in
  start)
  	start
	;;
  stop)
  	stop
	;;
  restart)
  	restart
	;;
  force-reload)
        restart
        ;;
  *)
	msg "Usage: $0 {start|stop|restart}"
	endmsg
	exit 2
esac

exit $?

