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
using namespace activemq::core;
using namespace cms;

class MonitorPlugInActiveMqTestSuite : public CxxTest::TestSuite 
{
public:
	void testMainConfiguration() {
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
		MonitorPlugInActiveMQ mock;

		auto_ptr<ActiveMQConnectionFactory> connectionFactory(new ActiveMQConnectionFactory());
		
		XMLSETTING(xmlString, xmlResults, xmlNode, XML_PARAMS_DEFAULT);
		mock.initializeConfiguration(xmlNode);
		ActiveMQConnectionFactory *pCF = connectionFactory.get();
		mock.initializeConnectionFactory(pCF);

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
		MonitorPlugInActiveMQ mock;
		Topics topics = mock.getTopics();
		TS_ASSERT_EQUALS(topics.size(), 0);
	}

	void testParseTopics() {
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

	void testInitializeActiveMqConnection() {
		MonitorPlugInActiveMQ mock;
		XMLSETTING(xmlString, xmlResults, xmlNode, "<destUri>default</destUri>\
<brokerUri>tcp://192.168.1.101:61616</brokerUri>");
		mock.initializeConfiguration(xmlNode);

		bool success = mock.initializeActiveMqConnection();		
		TS_ASSERT_EQUALS(success, false);
		
		XMLSETTING(xmlString2, xmlResults2, xmlNode2, "<destUri>default</destUri>\
<brokerUri>tcp://192.168.1.100:61616</brokerUri>");
		mock.initializeConfiguration(xmlNode2);

		success = mock.initializeActiveMqConnection();		
		TS_ASSERT_EQUALS(success, true);
	}

	void testInitializeTopics() {
		MonitorPlugInActiveMQ mock;
		ModuleResultBase mrb;
		TopicInfo referenceTopic;
		Topics topics = mock.getTopics();
		// Daten initalisieren
		referenceTopic.bDeliveryModePersistent = false;
		referenceTopic.bUseTopic = false;

		XMLSETTING(xmlString, xmlResults, xmlNode, "<destUri>default</destUri>\
<brokerUri>tcp://192.168.1.100:61616</brokerUri>");
		mock.initializeConfiguration(xmlNode);

		bool success = mock.initializeActiveMqConnection();		
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
		mock.setTopics(topics);
		TS_ASSERT_EQUALS(mock.getTopics().size(), 3);

		success = mock.processResult(&mrb);
		TS_TRACE("all initialized");
		TS_ASSERT_EQUALS(success, false); // could not send, unknown type

		mrb.set("typ", "fms");
		TS_ASSERT_EQUALS(mrb.get("typ"), "fms");
		TS_ASSERT_EQUALS(mock.processResult(&mrb), true);
				
	}
};
