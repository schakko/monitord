#!/bin/sh
# create .tar.gz file for monitord with ActiveMQ plug-in
DIST_NAME=monitord-dist
TARGET_DIR=./$DIST_NAME

# configure with ActiveMQ support
CPPFLAGS=`activemqcpp-config --includes` ./configure --enable-plugins --with-activemq

make clean
make

# create distribution
if [ -d $TARGET_DIR ]; then
	rm -rf $TARGET_DIR 
fi

mkdir $TARGET_DIR
cp monitord/monitord $TARGET_DIR
cp monitord/plugins/.libs/libmplugin_activemq.* $TARGET_DIR
cp monitord/scripts/* $TARGET_DIR

# find libactivemq
LIBACTIVEMQ=`ldconfig -p | grep "activemq" | cut -d\> -f2`
cp $(dirname -- "$LIBACTIVEMQ")/libactivemq* $TARGET_DIR

wget https://gist.githubusercontent.com/schakko/1a3f4997efbf7f243b04/raw/09730b535c0568c70bec0cc80cb27451345994d9/monitord.xml -O $TARGET_DIR/monitord.xml

# comprtess
tar -zcvf $DIST_NAME.tar.gz $TARGET_DIR
