#!/bin/sh

cd "$(dirname "$0")"

echo "Building CLI version..."
./build-console.sh

echo "Building web version..."
./build-web.sh

echo "Building cocoons..."
cocoons/web/build.sh
cocoons/text/build.sh
cocoons/json/build.sh
cocoons/mysql/build.sh
