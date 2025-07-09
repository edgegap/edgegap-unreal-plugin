#!/bin/bash

pid=$$
EXECUTABLE_PATH="$(dirname "$0")"
: ${TARGET_FILE_NAME:="LyraServer"} # edit to match your .target.cs file name

# variable and function definitions
separator() {
  printf "%0.s~" {1..80}
}

catch_term() {
  echo "TERM signal received..."
  separator
  kill -TERM "$pid" 2>/dev/null
}

catch_quit() {
  echo "QUIT signal received..."
  separator
  kill -QUIT "$pid" 2>/dev/null
}

# custom handling of process termination
trap catch_term TERM
trap catch_quit QUIT

# print variables for debugging purposes
separator
echo ARBITRIUM_PORTS_MAPPING:
echo "$ARBITRIUM_PORTS_MAPPING"

separator
echo ENVIRONMENT VARS:
env

# initialize game server
separator
echo Execute command: $EXECUTABLE_PATH/$TARGET_FILE_NAME.sh -log -PORT=$ARBITRIUM_PORT_GAMEPORT_INTERNAL $UE_COMMANDLINE_ARGS
separator
$EXECUTABLE_PATH/$TARGET_FILE_NAME.sh -log -PORT=$ARBITRIUM_PORT_GAMEPORT_INTERNAL $UE_COMMANDLINE_ARGS

# after game server process terminates
separator
echo Gameserver exit code: $?

# if server terminated terminated from Unreal, stop deployment
separator
echo Execute command: curl -s -X DELETE -H "Authorization: ${ARBITRIUM_DELETE_TOKEN}" "${ARBITRIUM_DELETE_URL}" | jq -r '.'
curl -s -X DELETE -H "Authorization: ${ARBITRIUM_DELETE_TOKEN}" "${ARBITRIUM_DELETE_URL}" | jq -r '.'
sleep infinity