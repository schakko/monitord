#!/bin/bash
# wrapper file for executing the cxxtests

if [ $# -eq 0 ]; then
	echo "You must provide the path to the cxxtest project as first argument"
	exit 1
fi


CXXTEST_DIR=$1
CXXTEST=$CXXTEST_DIR/bin/cxxtestgen
REPO_DIR=$(dirname $0)/../../..

if [ ! -d $CXXTEST_DIR ]; then
	echo "cxxtest directory $1 does not exist"
	exit 1
fi

if [ ! -e $CXXTEST ]; then
	echo "$CXXTEST binary does not exist"
	exit 1;
fi

if [ ! `which activemqcpp-config` ]; then
	echo "activemqcpp-config is required to determine the include path for ActiveMQ"
	exit 1;
fi

$CXXTEST --error-printer -o runner.cpp MonitorPluginActiveMqTestSuite.h
echo Runner for unit tests generated
mkdir -p "o"
cd ../../../
echo "Compiling ..."
make
cd -
find ../../../ -name "*.o" | xargs -i cp -v {} o/

# remove duplicate dependencies 
rm o/monitord_plugins_libmplugin_activemq_la-mplugin.o
rm o/monitord_plugins_libmplugin_activemq_la-xml*.o
rm o/monitord_plugins_libmplugin_audiorecorder_la-*
#  main() entfernen
rm o/monitord_monitord-Monitor.o
g++ `activemqcpp-config --includes` -I $CXXTEST_DIR  -I $REPO_DIR -I/usr/include -llua -lasound -ldl -lpthread -lm -lactivemq-cpp -o runner o/*.o runner.cpp
echo "Unittest compiled"
# libactivemq-cpp importieren
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

./runner -v
