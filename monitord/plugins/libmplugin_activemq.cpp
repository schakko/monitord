/**
 * monitord-activemq
 * http://github.com/schakko/monitord-activemq
 * Contains *untested* prototyped code
 */
#include "libmplugin_activemq.h"

using namespace std;
using namespace activemq;
using namespace activemq::core;
using namespace decaf::lang::exceptions;
using namespace cms;

MonitorPlugInActiveMQ::MonitorPlugInActiveMQ()
{
	m_bUseCompression = false;
	m_bClientAck = false;
	m_bConnected = false;
 	m_brokerUri = "tcp://127.0.0.1:61616";
 	m_username = "";
 	m_password = "";
 	m_clientId = "";
	m_destUri = "";
 	m_sendTimeout = 0;
	m_closeTimeout = 0;
	m_producerWindowSize = 0;

	m_genericTopic.bUseTopic = false;
	m_genericTopic.bDeliveryModePersistent = false;
	m_genericTopic.destUri = "";

	m_bDeliveryModePersistent = false;
	m_bTopicsInitialized = false;

	m_session = NULL;
	m_connection = NULL;

	activemq::library::ActiveMQCPP::initializeLibrary();
}

MonitorPlugInActiveMQ::~MonitorPlugInActiveMQ()
{
	freeTopics();
	freeConnection();
}

void MonitorPlugInActiveMQ::Show()
{
	LOG_INFO("MonitorActiveMQPlugin successfully loaded" )
}

void MonitorPlugInActiveMQ::onException(const CMSException& ex AMQCPP_UNUSED) {
	LOG_ERROR("Something really bad happend to ActiveMQ:" << ex.getMessage());
	freeTopics();
	freeConnection();
}

bool MonitorPlugInActiveMQ::processResult(class ModuleResultBase *pRes)
{
	LOG_DEBUG("apachemq: processing Result...")
	
	
	std::string type = (*pRes)["typ"];

	if (m_topics.find(type) == m_topics.end()) {
		LOG_ERROR("apachemq: received type " << type << " which is not registered")
		return false;
	}
	
	TopicInfo* topicInfo = (m_topics.find(type))->second;

	LOG_INFO("Preparing Ping message")

	// Verbindung wurde durch das Versenden des letzten Alarms getrennt und kann nicht wiederhergestellt werden
	if (!establishConnection()) {
		LOG_ERROR("Connection could not be established, discarding this message");
		return false;
	}

	// Nachricht - falls etwas schief geht, wird der Exception-Listener aufgerufen und löscht die Verbindungen
	TextMessage* ping = m_session->createTextMessage();
	ping->setStringProperty("ping", "pong");
	topicInfo->producer->send(ping);
	delete ping;

	// Ping wurde gesendet, es könnte aber eine Exception aufgetreten sein
	// Also Verbindung noch mal sicherstellen
	if (!establishConnection()) {
		LOG_ERROR("Connection could not be established after failed Ping, discarding this message");
		return false;
	}
	
	
	LOG_INFO("Preparing final alarm message")
	TextMessage* message = m_session->createTextMessage();

	ResultItemsMap::iterator i;

	for (i = (*pRes).m_Items.begin(); i != (*pRes).m_Items.end(); i++) {
		message->setStringProperty(i->first, i->second);
	}

	// Wenn das Senden der Nachricht jetzt fehlschlägt, wird beim nächsten Mal probiert, die Verbindung wieder zu öffnen
	topicInfo->producer->send(message);
	
	LOG_INFO("message sent to topic")

	delete message;

	return true;
}

void MonitorPlugInActiveMQ::initializeConfiguration(XMLNode config)
{	
	m_brokerUri 	= getNodeText(config, ACTIVEMQ_XMLNODE_BROKERURI, "tcp://127.0.0.1:61616");
	m_username 	= getNodeText(config, ACTIVEMQ_XMLNODE_USERNAME, "");
	m_password 	= getNodeText(config, ACTIVEMQ_XMLNODE_PASSWORD, "");
	m_clientId	= getNodeText(config, ACTIVEMQ_XMLNODE_CLIENTID, "");
	m_sendTimeout 	= getNodeInt(config, ACTIVEMQ_XMLNODE_SENDTIMEOUT, 0);
	m_closeTimeout 	= getNodeInt(config, ACTIVEMQ_XMLNODE_CLOSETIMEOUT, 0);
	m_producerWindowSize 	= getNodeInt(config, ACTIVEMQ_XMLNODE_PRODUCERWINDOWSIZE, 0);
	m_bUseCompression	= getNodeBool(config, ACTIVEMQ_XMLNODE_USECOMPRESSION, false);
	m_bClientAck 	= getNodeBool(config, ACTIVEMQ_XMLNODE_CLIENTACK, false);
}

void MonitorPlugInActiveMQ::initializeConnectionFactory()
{
	// Set Broker-URI first - otherwise username and password is lost
	m_connectionFactory->setBrokerURI(m_brokerUri);

	if (!m_username.empty()) {
		m_connectionFactory->setUsername(m_username);
	}

	if (!m_password.empty()) {
		m_connectionFactory->setPassword(m_password);
	}

	if (!m_clientId.empty()) {
		m_connectionFactory->setClientId(m_clientId);
	}

	m_connectionFactory->setUseCompression(m_bUseCompression);
	m_connectionFactory->setSendTimeout(m_sendTimeout);
	m_connectionFactory->setCloseTimeout(m_closeTimeout);
	m_connectionFactory->setProducerWindowSize(m_producerWindowSize);
	
	LOG_DEBUG("Connection factory initialized")
}

bool MonitorPlugInActiveMQ::initializeConnection()
{

	// create a connection
	try {
		m_connection = m_connectionFactory->createConnection();
		m_connection->start();

		if (m_bClientAck) {
			m_session = m_connection->createSession(Session::CLIENT_ACKNOWLEDGE);
		} 
		else {
			m_session = m_connection->createSession(Session::AUTO_ACKNOWLEDGE);
		}

		m_bConnected = true;
	} 
	catch (CMSException& e) {
		LOG_ERROR("Could not connect to messaging queue \"" << m_brokerUri << "\" with username=\"" << m_username << "\"")
		m_bConnected = false;
	}

	LOG_DEBUG("Connection initialized")

	return m_bConnected;
}

void MonitorPlugInActiveMQ::freeConnection() {
	// Close open resources.
	try {
		if (m_session != NULL) {
			m_session->close();
		}

		if (m_connection != NULL) {
			m_connection->close();
		}
	}
	catch (CMSException& e) {
		e.printStackTrace(); 
	}

	try {
		if (m_session != NULL) {
			delete m_session;
		}
	}
	catch (CMSException& e) { 
		e.printStackTrace(); 
	}

	m_session = NULL;

	try {
		if (m_connection != NULL) {
			delete m_connection;
		}
	} 
	catch (CMSException& e) { 
		e.printStackTrace(); 
	}

	m_bConnected = false;
	m_connection = NULL;
}

bool MonitorPlugInActiveMQ::initProcessing(class MonitorConfiguration* configPtr, XMLNode config)
{
	// initialize ActiveMQ
	activemq::library::ActiveMQCPP::initializeLibrary();

	// read default configuration for topics
	parseTopic(config, m_genericTopic, m_genericTopic);
	// read inherited topic configuration
	parseTopics(config, m_topics, m_genericTopic);

	initializeConfiguration(config);
	initializeConnectionFactory();
	bool r = establishConnection();

	return r;
}

bool MonitorPlugInActiveMQ::establishConnection() {
	if (initializeConnection()) {
		initializeTopics(m_topics);
	}

	return m_bConnected;
}

void MonitorPlugInActiveMQ::initializeTopics(Topics &topics)
{
	if (m_bConnected == false) {
		throw RuntimeException(__FILE__, __LINE__, "Tried to initialize topics without established ActiveMQ connection. Call initializeActiveMqConnection() first.");
	}

	Topics::iterator i;
	TopicInfo *pTopicInfo;

	// create new producers
	for (i = topics.begin(); i != topics.end(); i++)
	{
		pTopicInfo = i->second;
		
		if (pTopicInfo->bUseTopic) {
			pTopicInfo->destination = m_session->createTopic(pTopicInfo->destUri);
		} 
		else {
			pTopicInfo->destination = m_session->createQueue(pTopicInfo->destUri);
		}

		pTopicInfo->producer = m_session->createProducer(pTopicInfo->destination);

		if (pTopicInfo->bDeliveryModePersistent) {
			pTopicInfo->producer->setDeliveryMode(DeliveryMode::PERSISTENT);
		} 
		else {
			pTopicInfo->producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
		}
		
		LOG_INFO("Topic destination \"" << pTopicInfo->destination << "\" created")
	}

	LOG_INFO("Topics initialized")
		
	m_bTopicsInitialized = true;
}

void MonitorPlugInActiveMQ::freeTopics() {
	Topics::iterator i ;
	TopicInfo *pTopicInfo;
	
  	// Destroy resources.
	for (i = m_topics.begin(); i != m_topics.end(); i++)
	{
		pTopicInfo = i->second;

		try {
			if (pTopicInfo->destination != NULL) { 
				delete pTopicInfo->destination;
			}
       		}
		catch (CMSException& e) { 
			e.printStackTrace(); 
		}
			
		pTopicInfo->destination = NULL;

		try {
			if (pTopicInfo->producer != NULL) {
				delete pTopicInfo->producer;
			}
		}
		catch (CMSException& e) { 
			e.printStackTrace(); 
		}
		
		pTopicInfo->producer = NULL;
	}
}


bool MonitorPlugInActiveMQ::quitProcessing() 
{
	return true;
}

void MonitorPlugInActiveMQ::parseTopics(XMLNode config, Topics &topics, TopicInfo &referenceTopic)
{
	XMLNode topicNode;
	Topics::iterator i;
	TopicInfo *pTopicInfo;
	
	// defaults
	vector<string> channels;
	channels.push_back(ACTIVEMQ_KEY_POCSAG);
	channels.push_back(ACTIVEMQ_KEY_ZVEI);
	channels.push_back(ACTIVEMQ_KEY_FMS);
	
	for (unsigned int i = 0, m = channels.size(); i < m; i++)
	{
		pTopicInfo = new TopicInfo;
		initializeTopic(*pTopicInfo, referenceTopic);
		topics.insert(PairMapping(channels.at(i), pTopicInfo));
	}

	int nTopic = config.nChildNode(ACTIVEMQ_XMLNODE_TOPIC);
	
	for (int num = 0; num < nTopic ; ++num)
	{
		if (!((topicNode = config.getChildNode(ACTIVEMQ_XMLNODE_TOPIC, num))).isEmpty()) {
			std::string type = topicNode.getAttribute(ACTIVEMQ_XMLATTR_TYPE) ;
			
			if ((type == ACTIVEMQ_KEY_POCSAG) || (type == ACTIVEMQ_KEY_FMS) || (type == ACTIVEMQ_KEY_ZVEI)) {
				pTopicInfo = (topics.find(type))->second;
				parseTopic(topicNode, *pTopicInfo, referenceTopic);
			}
		}
	}
}

void MonitorPlugInActiveMQ::parseTopic(XMLNode config, TopicInfo &topicInfo, TopicInfo &referenceTopic)
{
	initializeTopic(topicInfo, referenceTopic);

	if (config.nChildNode(ACTIVEMQ_XMLNODE_USETOPIC) >= 1) {
		topicInfo.bUseTopic = getNodeBool(config, ACTIVEMQ_XMLNODE_USETOPIC, false);
	}

	if (config.nChildNode(ACTIVEMQ_XMLNODE_DELIVERYMODEPERSISTENT) >= 1) {
		topicInfo.bDeliveryModePersistent = getNodeBool(config, ACTIVEMQ_XMLNODE_DELIVERYMODEPERSISTENT, false);
	}

	if (config.nChildNode(ACTIVEMQ_XMLNODE_DESTURI) >= 1) {
		topicInfo.destUri = getNodeText(config, ACTIVEMQ_XMLNODE_DESTURI, "monitord");
	}
}

void MonitorPlugInActiveMQ::initializeTopic(TopicInfo &topicInfo, TopicInfo &referenceTopic)
{
	topicInfo.bUseTopic = referenceTopic.bUseTopic;
	topicInfo.bDeliveryModePersistent = referenceTopic.bDeliveryModePersistent;
	topicInfo.destUri = referenceTopic.destUri;
	topicInfo.destination = NULL;
	topicInfo.producer = NULL;
}


MonitorPlugInActiveMQFactory::MonitorPlugInActiveMQFactory()
{
}

MonitorPlugInActiveMQFactory::~MonitorPlugInActiveMQFactory()
{
}

MonitorPlugIn * MonitorPlugInActiveMQFactory::CreatePlugIn()
{
	return new MonitorPlugInActiveMQ();
}


DLL_EXPORT void * factory0( void )
{
	return new MonitorPlugInActiveMQFactory;
}
