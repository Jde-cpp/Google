#!/bin/bash
debug=${1:-1}
clean=${2:-0}
echo "Build Drive.Google"
cd "${0%/*}"
if [ $clean -eq 1 ]; then
	make clean DEBUG=$debug
	if [ $debug -eq 1 ]; then
		ccache g++-8 -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_DRIVE_GOOGLE_EXPORTS  -I.obj/debug -I../../.. -O0 -fsanitize=address -fno-omit-frame-pointer ./pc.h -o .obj/debug/stdafx.h.gch -I/home/duffyj/code/libraries/json/include -I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/boostorg/boost_1_68_0
	else
		ccache g++-8 -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_DRIVE_GOOGLE_EXPORTS  -I.obj/release -I../../.. -march=native -DNDEBUG -O3 ./pc.h -o .obj/release/stdafx.h.gch -I/home/duffyj/code/libraries/json/include -I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/boostorg/boost_1_68_0
	fi;
	if [ $? -eq 1 ]; then
		exit 1
	fi;
fi
make -j7 DEBUG=$debug
cd -
exit $?