#!/bin/sh

cd "$(dirname "$0")"

./build-console.sh
./build-web.sh
cocoons/web/build.sh
cocoons/text/build.sh
cocoons/json/build.sh
