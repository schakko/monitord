#define PLUGINS
#include "../libmplugin_activemq.h"
#include <cxxtest/TestSuite.h>

#define XMLSETTING(xmlString, xmlResults, xmlNode, CHARS) \
		const char *xmlString = CHARS; \
		XMLResults xmlResults;\
		XMLNode xmlNode; \
		xmlNode = XMLNode::parseString(xmlString, NULL, &xmlResults); \

#define XML_PARAMS_DEFAULT "<parameters><brokerUri>tcp://localhost</brokerUri><username>new_username</username><password>new_password</password><clientId>new_client_id</clientId><sendTimeout>2</sendTimeout><closeTimeout>2</closeTimeout><producerWindowSize>2</producerWindowSize><useCompression>1</useCompression><clientAck>1</clientAck>"

#define XML_PARAMS_TOPIC_DEFAULT "<topic><useTopic>1</useTopic><deliveryModePersistent>1</deliveryModePersistent><destUri>activemq_test</destUri></topic>"

using namespace std;
using namespace activemq;
using namespace activemq::commands;
using namespace activemq::core;
using namespace cms;

class MonitorPlugInActiveMqTestSuite : public CxxTest::TestSuite 
{
public:
	void tearDown() {
		TS_TRACE("Shutting down library...");
		activemq::library::ActiveMQCPP::initializeLibrary();
		activemq::library::ActiveMQCPP::shutdownLibrary();	
		TS_TRACE("... library shutdown");
	}


	void testMainConfiguration() {
		TS_TRACE("testMainConfiguration");
		MonitorPlugInActiveMQ mock;
		TS_ASSERT_EQUALS(mock.m_bConnected, false);
		TS_ASSERT_EQUALS(mock.m_bUseCompression, false);
		TS_ASSERT_EQUALS(mock.m_bClientAck, false);
		TS_ASSERT_EQUALS(mock.m_bDeliveryModePersistent, false);

		XMLSETTING(xmlString, xmlResults, xmlNode, XML_PARAMS_DEFAULT);
		mock.initializeConfiguration(xmlNode);

		TS_ASSERT_EQUALS(mock.m_brokerUri, "tcp://localhost");
		TS_ASSERT_EQUALS(mock.m_username, "new_username");
		TS_ASSERT_EQUALS(mock.m_password, "new_password");	
		TS_ASSERT_EQUALS(mock.m_clientId, "new_client_id");	
		TS_ASSERT_EQUALS(mock.m_sendTimeout, 2);
		TS_ASSERT_EQUALS(mock.m_closeTimeout, 2);
		TS_ASSERT_EQUALS(mock.m_producerWindowSize, 2);
		TS_ASSERT_EQUALS(mock.m_bUseCompression, true);
		TS_ASSERT_EQUALS(mock.m_bClientAck, true);
	}

	void testInitialzeConnectionFactory() {
		TS_TRACE("testInitializeConnectionFactory");
		MonitorPlugInActiveMQ mock;

		XMLSETTING(xmlString, xmlResults, xmlNode, XML_PARAMS_DEFAULT);
		mock.initializeConfiguration(xmlNode);
		mock.initializeConnectionFactory();
		std::auto_ptr<ActiveMQConnectionFactory> pCF = mock.m_connectionFactory;

		TS_ASSERT_EQUALS(pCF->getBrokerURI().toString(), "tcp://localhost");
		TS_ASSERT_EQUALS(pCF->getClientId(), "new_client_id");
		TS_ASSERT_EQUALS(pCF->getPassword(), "new_password");
		TS_ASSERT_EQUALS(pCF->getUsername(), "new_username");
		TS_ASSERT_EQUALS(pCF->isUseCompression(), true);
		TS_ASSERT_EQUALS(pCF->getSendTimeout(), 2);
		TS_ASSERT_EQUALS(pCF->getCloseTimeout(), 2);
		TS_ASSERT_EQUALS(pCF->getProducerWindowSize(), 2);
	}

	void testParseTopic() {
		TS_TRACE("testParseTopic");

		MonitorPlugInActiveMQ mock;
		XMLSETTING(xmlString, xmlResults, xmlNode, XML_PARAMS_TOPIC_DEFAULT);
		TopicInfo topicInfo;
		topicInfo.bUseTopic = false;
		topicInfo.bDeliveryModePersistent = false;
		topicInfo.destUri = "";

		mock.parseTopic(xmlNode, topicInfo, topicInfo);

		TS_ASSERT_EQUALS(topicInfo.bUseTopic, true);
		TS_ASSERT_EQUALS(topicInfo.bDeliveryModePersistent, true);
		TS_ASSERT_EQUALS(topicInfo.destUri, "activemq_test");


		XMLSETTING(xmlString2, xmlResults2, xmlNode2, "<topic><deliveryModePersistent>0</deliveryModePersistent><destUri>overwritten</destUri></topic>");
		TopicInfo topicInfo2;

		mock.parseTopic(xmlNode2, topicInfo2, topicInfo);
		TS_ASSERT_EQUALS(topicInfo2.bUseTopic, true); // Taken from first
		TS_ASSERT_EQUALS(topicInfo2.bDeliveryModePersistent, false); // Taken from first
		TS_ASSERT_EQUALS(topicInfo2.destUri, "overwritten"); // overwritten
	}

	void testGetTopics() {
		TS_TRACE("testGetTopics");
		MonitorPlugInActiveMQ mock;
		TS_ASSERT_EQUALS(mock.m_topics.size(), 0);
	}

	void testParseTopics() {
		TS_TRACE("testParseTopics");
		MonitorPlugInActiveMQ mock;
		TopicInfo referenceTopic;
		// Daten initalisieren
		referenceTopic.bDeliveryModePersistent = false;
		referenceTopic.bUseTopic = false;
		Topics topics;

		XMLSETTING(xmlString, xmlResults, xmlNode, "\
<destUri>default</destUri>\
<topic type=\"fms\">\
	<deliveryModePersistent>1</deliveryModePersistent>\
	<useTopic>1</useTopic>\
	<destUri>fms</destUri>\
</topic>\
<topic type=\"pocsag\">\
	<useTopic>0</useTopic>\
	<destUri>pocsag</destUri>\
</topic>");
		mock.parseTopic(xmlNode, referenceTopic, referenceTopic);
		TS_ASSERT_EQUALS(referenceTopic.destUri, "default");
		mock.parseTopics(xmlNode, topics, referenceTopic);
		TS_ASSERT_EQUALS(topics.size(), 3);
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_FMS))->second).destUri, "fms");
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_FMS))->second).bDeliveryModePersistent, true);
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_FMS))->second).bUseTopic, true);
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_POCSAG))->second).destUri, "pocsag");
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_POCSAG))->second).bDeliveryModePersistent, false);
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_POCSAG))->second).bUseTopic, false);
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_ZVEI))->second).destUri, "default");
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_ZVEI))->second).bUseTopic, false);
		TS_ASSERT_EQUALS((*(topics.find(ACTIVEMQ_KEY_ZVEI))->second).bDeliveryModePersistent, false);
	}

	void testInitializeConnection() {
		TS_SKIP("Skipping integration test");
		TS_TRACE("testInitializeConnection");
		MonitorPlugInActiveMQ mock;
		XMLSETTING(xmlString, xmlResults, xmlNode, "<destUri>default</destUri>\
<sendTimeout>1</sendTimeout><closeTimeout>1</closeTimeout><brokerUri>tcp://127.0.0.1:61616?soConnectTimeout=3&amp;transport.tcpTracingEnabled=true</brokerUri>");
		mock.initializeConfiguration(xmlNode);

		bool success = mock.initializeConnection();		
		TS_ASSERT_EQUALS(success, false);
		TS_TRACE("Connection attempt failed");		
		XMLSETTING(xmlString2, xmlResults2, xmlNode2, "<destUri>default</destUri>\
<brokerUri>tcp://192.168.1.100:61616</brokerUri>");
		mock.initializeConfiguration(xmlNode2);

		success = mock.initializeConnection();		
		TS_ASSERT_EQUALS(success, true);
	}

	void testUpdateTextMessage() {
		TS_TRACE("testUpdateTextMessage");
		MonitorPlugInActiveMQ mock;
		std::auto_ptr<ActiveMQTextMessage> textMessage(new ActiveMQTextMessage());
		std::auto_ptr<ModuleResultBase> result(new ModuleResultBase());

		result->set("key", "value");
		result->set("text", "base64");

		mock.updateTextMessage(*textMessage, *result); //moduleResultBase);

		TS_ASSERT_EQUALS(textMessage->getStringProperty("key"), "value");
		// ensure that the "text" property has been encoded as base64
		TS_ASSERT_EQUALS(textMessage->getStringProperty("text"), "YmFzZTY0");
		TS_TRACE("text property has been base64 encoded");
	}

	void testInitializeTopics() {
		TS_SKIP("Skipping integration test");
		TS_TRACE("testInitializeTopics");

		MonitorPlugInActiveMQ mock;
		ModuleResultBase mrb;
		TopicInfo referenceTopic;
		Topics topics = mock.m_topics;
		// Daten initalisieren
		referenceTopic.bDeliveryModePersistent = false;
		referenceTopic.bUseTopic = false;

		XMLSETTING(xmlString, xmlResults, xmlNode, "<destUri>default</destUri>\
<brokerUri>tcp://192.168.1.100:61616</brokerUri>");
		mock.initializeConfiguration(xmlNode);

		bool success = mock.initializeConnection();		
		TS_ASSERT_EQUALS(success, true);

		mrb.set("key1", "val1");
		mrb.set("key2", "val2");
		mrb.set("typ", "unknown");

		TS_ASSERT_EQUALS(mrb.get("key1"), "val1");		
		TS_ASSERT_EQUALS(mrb.get("key2"), "val2");
		
		// fail -> not initialized
		TS_ASSERT_THROWS_ANYTHING(mock.processResult(&mrb));

		// load default topic
		mock.parseTopic(xmlNode, referenceTopic, referenceTopic);
		// parse all topics
		mock.parseTopics(xmlNode, topics, referenceTopic);
		mock.initializeTopics(topics);

		TS_ASSERT_EQUALS(topics.size(), 3);
		mock.m_topics = topics;
		TS_ASSERT_EQUALS(mock.m_topics.size(), 3);

		success = mock.processResult(&mrb);
		TS_TRACE("all initialized");
		TS_ASSERT_EQUALS(success, false); // could not send, unknown type

		mrb.set("typ", "fms");
		TS_ASSERT_EQUALS(mrb.get("typ"), "fms");
		TS_ASSERT_EQUALS(mock.processResult(&mrb), true);
				
	}
};
