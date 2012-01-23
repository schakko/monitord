#!/bin/bash
# Dreckiges Script zum Ausfuehren der Unittests
CXXTEST=~/dev/app/cxxtest
$CXXTEST/cxxtestgen.pl --error-printer -o runner.cpp MonitorPluginActiveMqTestSuite.h
echo Unittest generated

cd ../../../
echo "Compiling ..."
make
cd -
find ../../../ -name "*.o" | xargs -i cp -v {} o/
# Doppelte Abhaengigkeiten eentfernen
rm o/monitord_plugins_libmplugin_activemq_la-mplugin.o
rm o/monitord_plugins_libmplugin_activemq_la-xml*.o
rm o/monitord_plugins_libmplugin_audiorecorder_la-xml*.o
rm o/monitord_plugins_libmplugin_audiorecorder_la-m*.o
# main() entfernen
rm o/monitord_monitord-Monitor.o
g++ -I /usr/local/include/activemq-cpp-3.4.0 -I $CXXTEST -I ~/dev/app/monitord/trunk -lasound -ldl -lpthread -lm -lactivemq-cpp -o runner o/*.o runner.cpp
echo "Unittest compiled"
# libactivemq-cpp importieren
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

./runner
