#!/bin/bash
RELPATH="$(dirname "$0")"
cd "$RELPATH"
OWNPATH="$(pwd)"
cd "$OWNPATH"

echo "USER = $USER"
echo "HOSTNAME = $HOSTNAME"

echo "normal 1"
>&2 echo "error  1"
echo "normal 2"
>&2 echo "error  2"
echo "normal 3"
>&2 echo "error  3"
echo "normal 4"
>&2 echo "error  4"
echo "normal 5"
>&2 echo "error  5"
echo "normal 6"
>&2 echo "error  6"
echo "normal 7"
>&2 echo "error  7"
echo "normal 8"
>&2 echo "error  8"
echo "normal 9"
>&2 echo "error  9"

exit 0