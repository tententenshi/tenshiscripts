#! /bin/sh

EDITOR='/usr/bin/emacs -nw'

if [ $# -eq 1 ]; then
        RHOST=$1
	CRON_FILE="$HOME/cron/cron.$RHOST"

	mv -f $CRON_FILE $CRON_FILE.bak
	ssh $RHOST crontab -l > $CRON_FILE
	$EDITOR $CRON_FILE
	ssh $RHOST crontab $CRON_FILE
else
        RHOST=`hostname`
	CRON_FILE="$HOME/cron/cron.$RHOST"

	mv -f $CRON_FILE $CRON_FILE.bak
	crontab -l > $CRON_FILE
	$EDITOR $CRON_FILE
	crontab $CRON_FILE
fi

