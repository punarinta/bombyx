#!/bin/sh

cd "$(dirname "$0")"

echo "Building CLI version..."
./build-console.sh

echo "Building web version..."
./build-web.sh

echo "Building cocoons..."
cd cocoons

for f in *
do
    if [ -d "$f" ]
    then
        echo "* $f";
        ./$f/build.sh
    fi
done

cd ..
