#!/bin/bash
# 
#    Copyright 2020 CentraleSupelec, 
#    Author : Jérémy Fix
# 
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.


usage_m="Usage : $0 <start|stop>

Script to start or stop the zookeeper and kafka servers

For start, you need to provide the zookeeper config and server configs,e.g.

	$0 start --zooconfig \$KAFKA_PATH/config/zookeeper.properties --serverconfig \$KAFKA_PATH/config/server.properties 
"

GREEN="\\e[1;32m"
NORMAL="\\e[0;39m"
RED="\\e[1;31m"
BLUE="\\e[1;34m"
MAGENTA="\\e[1;35m"

function display_info() {
    echo -e "$BLUE $1 $NORMAL"
}
function display_wait() {
    echo -e "$MAGENTA $1 $NORMAL"
}
function display_success() {
    echo -e "$GREEN $1 $NORMAL"
}
function display_error() {
    echo -e "$RED $1 $NORMAL"
}

function run_command() {
    display_wait "$1..."
    MSG=`$2 2>&1`
    ERR=$?
    if [ $ERR = 0 ]; then
	$3
	display_success "Done"
    else
	display_error "Error: \"$MSG\""
    fi
    return $?
}

if [ -z $KAFKA_PATH ]; then
    display_error "The environment variable KAFKA_PATH must be set"
fi

# Parse the command line arguments
ACTION=
ZOOKEEPER_CONFIG=
SERVER_CONFIG=

while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -h|--help)
            exec echo "$usage_m";;
        --zooconfig)
            ZOOKEEPER_CONFIG="$2"
            shift # pass argument
            shift # pass value
            ;;
        --serverconfig)
            SERVER_CONFIG="$2"
            shift
            shift
            ;;
        -w|--walltime)
            WALLTIME="$2"
            shift
            shift
            ;;
	start|stop)
	    ACTION=$1
	    shift
	    ;;
	*)
	    exec echo "Unrecognized option $key"
    esac
done

case $ACTION in 
    start)
	if [ -z $ZOOKEEPER_CONFIG ]; then
	    display_error "You must provide a zookeeper config with the --zooconfig option"
	    exit -1
	fi
	if [ -z $SERVER_CONFIG ]; then
	    display_error "You must provide a server config with the --serverconfig option"	
	    exit -1
	fi

	run_command "Starting the zookeeper server" "$KAFKA_PATH/bin/zookeeper-server-start.sh -daemon $ZOOKEEPER_CONFIG" "sleep 2"

	run_command "Starting the kafka server" "$KAFKA_PATH/bin/kafka-server-start.sh -daemon $SERVER_CONFIG" "sleep 2"
	;;
    stop)
	# Stop the kafka server
	run_command "Stopping the kafka server" "$KAFKA_PATH/bin/kafka-server-stop.sh" "sleep 2"

	# Stop the zookeeper daemon
	run_command "Stopping the zookeeper server" "$KAFKA_PATH/bin/zookeeper-server-stop.sh"
	;;
    *)
	display_error "You must specify an action, see $0 --help"
esac
