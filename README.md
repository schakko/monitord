# monitord-activemq

## Dependency
monitord-activemq depends on activemq-cpp.

## Installation
Just push back this directory into /monitord/trunk.

## Compiling on CentOS 6.2

	yum install autoconf automake pkg-config alsa-lib-devel lua-devel
	aclocal -Im4
	./configure --enable-plugins --with-activemq

## Configuration 
Edit your monitord.xml:

	<dataplugins>
	<plugin name="activemq">
		<file>plugins/libmplugin_activemq.so</file>
		<parameters>
			<logfile>activemq.log</logfile>
			<loglevel>DEBUG</loglevel>
			<brokerUri>tcp://127.0.0.1:61616</brokerUri>
			<username>your_username_or_empty</username>
			<password>your_password_or_empty</password>
			<clientId>your_clientid_or_empty</clientId>
			<sendTimeout>5</sendTimeout>
			<closeTimeout>5</closeTimeout>
			<producerWindowSize>5</producerWindowSize>
			<useCompression>1</useCompression>
			<clientAck>0</clientAck>

			<!-- generic configuration -->
			<useTopic>1</useTopic>
			<deliveryModePersistent>0</deliveryModePersistent>
			<destUri>zabos</destUri>

			<!-- overwrite generic configuration for FMS -->
			<topic type="fms">
				<destUri>zabos.fms</destUri>
			</topic>

			<!-- overwrite gneric configuration for POCSAG -->
			<topic type="pocsag">
				<useTopic>0</useTopic>
				<!-- destUri is still zabos, imported by generic configuration -->
			</topic>
			<!-- ZVEI is not defined, so generic configuration is used -->
		</parameters>
	</plugin>
	</dataplugins> 
