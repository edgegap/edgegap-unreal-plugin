#!/bin/sh

# variable and function definitions
pid=$$
GAME_PORT=$(echo $ARBITRIUM_PORTS_MAPPING | jq '.ports.gameport.internal')

$(dirname "$0")/<PROJECT_NAME>Server.sh -log -PORT=$GAME_PORT 	separator() {
  printf "%0.s~" {1..80}
}

catch_kill() {
  echo "SIGKILL signal received..."
  separator()
  kill -KILL "$pid" 2>/dev/null
}

catch_term() {
  echo "SIGTERM signal received..."
  separator()
  kill -TERM "$pid" 2>/dev/null
}

catch_quit() {
  echo "SIGTERM signal received..."
  separator()
  kill -QUIT "$pid" 2>/dev/null
}

# custom handling of process termination
trap catch_term SIGTERM
trap catch_kill SIGKILL
trap catch_quit SIGQUIT

# print variables for debugging purposes
separator()
echo ARBITRIUM_PORTS_MAPPING:
echo "$ARBITRIUM_PORTS_MAPPING"

separator()
echo ENVIRONMENT VARS:
env

# initialize game server
separator()
echo Execute command: $(dirname "$0")/<PROJECT_NAME>Server.sh -log -PORT=$GAME_PORT $UE_COMMANDLINE_ARGS
separator()
$(dirname "$0")/<PROJECT_NAME>Server.sh -log -PORT=$GAME_PORT $UE_COMMANDLINE_ARGS

# after game server process terminates
separator()
echo Gameserver exit code: $?

# if server terminated terminated from Unreal, stop deployment
separator()
echo Execute command: curl -s -X DELETE -H "Authorization: ${ARBITRIUM_DELETE_TOKEN}" "${ARBITRIUM_DELETE_URL}" | jq -r '.'
curl -s -X DELETE -H "Authorization: ${ARBITRIUM_DELETE_TOKEN}" "${ARBITRIUM_DELETE_URL}" | jq -r '.'
sleep infinity
