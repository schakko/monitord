/**
 * A monitord plug-in for native sending of POOCSAG/FMS/ZVEI messages from the monitord core to ActiveMQs queues or topics.
 *
 * http://github.com/schakko/monitord
 */
#include "libmplugin_activemq.h"

using namespace std;
using namespace activemq;
using namespace activemq::core;
using namespace decaf::lang::exceptions;
using namespace cms;
using namespace decaf::net;

/**
 * Important: the m_connectionFactory() must be explicitly called. Using the auto_ptr inside the header file does not work anymore.
 */
MonitorPlugInActiveMQ::MonitorPlugInActiveMQ() : m_connectionFactory(new ActiveMQConnectionFactory())
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

	m_bConnected = false;
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
	LOG_ERROR("Exception caught from ActiveMQ: " << ex.getMessage());
	freeTopics();
	freeConnection();
}

/**
 * Interface method; called by the monitord core on arrival of a new message
 */
bool MonitorPlugInActiveMQ::processResult(class ModuleResultBase *pRes)
{
	LOG_INFO("apachemq: processing Result...")
	
	
	std::string type = (*pRes)["typ"];

	if (m_topics.find(type) == m_topics.end()) {
		LOG_ERROR("apachemq: received type " << type << " which is not registered")
		return false;
	}
	
	TopicInfo* topicInfo = (m_topics.find(type))->second;

	LOG_INFO("Preparing Ping message")

	// connection has been lost after/during the send of the last received internal message. The connection could not be recovered
	if (!establishConnection()) {
		LOG_ERROR("Connection could not be established, discarding this message");
		return false;
	}

	// send a test message to enmsure we have a working connection
	TextMessage* ping = m_session->createTextMessage();
	ping->setStringProperty("ping", "pong");
	topicInfo->producer->send(ping);
	delete ping;

	// after the ping has been sent, we ensure that a successful has been established
	if (!establishConnection()) {
		LOG_ERROR("Connection could not be established after failed Ping, discarding this message");
		return false;
	}
	
	
	LOG_INFO("Preparing final alarm message")
	TextMessage* message = m_session->createTextMessage();

	updateTextMessage(*message, *pRes);

	// if sending the messasge item fails, the connection will be recovered on the next processResult call
	topicInfo->producer->send(message);
	
	LOG_INFO("message sent to topic")

	delete message;

	return true;
}

void MonitorPlugInActiveMQ::updateTextMessage(cms::TextMessage& textMessage, class ModuleResultBase& pRes) {
	ResultItemsMap::iterator i;

	for (i = pRes.m_Items.begin(); i != pRes.m_Items.end(); i++) {
		// ZABOS-150: Binary data (encrypted POCSAG messages) must be converted to Base64 so ActiveMQ can decode the data
		if (i->first == "text") {
			textMessage.setStringProperty(
				i->first, 
				base64_encode(
					reinterpret_cast<const unsigned char*>(i->second.c_str()), 
					i->second.length()
				)
			);
		}
		else {
			textMessage.setStringProperty(i->first, i->second);
		}
	}
} 

/**
 * Read the <plugin name="activem">..</plugin> section from the configuration XML file.
 * All parameters are stored as member variables of the MonitorPlugInActiveMQ instance.
 */
void MonitorPlugInActiveMQ::initializeConfiguration(XMLNode config)
{
	LOG_INFO("Initializing ActiveMQ configuration")

	m_brokerUri 	= getNodeText(config, ACTIVEMQ_XMLNODE_BROKERURI, "tcp://127.0.0.1:61616");
	m_username 	= getNodeText(config, ACTIVEMQ_XMLNODE_USERNAME, "");
	m_password 	= getNodeText(config, ACTIVEMQ_XMLNODE_PASSWORD, "");
	m_clientId	= getNodeText(config, ACTIVEMQ_XMLNODE_CLIENTID, "");
	m_sendTimeout 	= getNodeInt(config, ACTIVEMQ_XMLNODE_SENDTIMEOUT, 0);
	m_closeTimeout 	= getNodeInt(config, ACTIVEMQ_XMLNODE_CLOSETIMEOUT, 0);
	m_producerWindowSize 	= getNodeInt(config, ACTIVEMQ_XMLNODE_PRODUCERWINDOWSIZE, 0);
	m_bUseCompression	= getNodeBool(config, ACTIVEMQ_XMLNODE_USECOMPRESSION, false);
	m_bClientAck 	= getNodeBool(config, ACTIVEMQ_XMLNODE_CLIENTACK, false);
	
	LOG_INFO("ActiveMQ configuration initialized")
}

/**
 * Initialize the ActiveMQ connection factory. The connection factory creates new connections.
 */
void MonitorPlugInActiveMQ::initializeConnectionFactory()
{
	LOG_INFO("Initiailizing ActiveMQ connection factory")

	// Set Broker-URI first - otherwise username and password configuration are lost
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
	
	LOG_INFO("Connection factory initialized")
}

/**
 * Initialize the connection and its session instance
 * @return cms::Connection
 */
bool MonitorPlugInActiveMQ::initializeConnection()
{
	LOG_INFO("Initializing new ActiveMQ connection")

	// create a connection
	try {
		m_connection = dynamic_cast<activemq::core::ActiveMQConnection*>(m_connectionFactory->createConnection());
		LOG_INFO("Connection prepared")
		m_connection->start();
		LOG_INFO("Connection started")

		if (m_bClientAck) {
			LOG_DEBUG("Setting: client acknowledge required")
			m_session = dynamic_cast<activemq::core::ActiveMQSession*>(m_connection->createSession(Session::CLIENT_ACKNOWLEDGE));
		} 
		else {
			LOG_DEBUG("Setting: auto acknowledge enabled")
			m_session = dynamic_cast<activemq::core::ActiveMQSession*>(m_connection->createSession(Session::AUTO_ACKNOWLEDGE));
		}

		m_bConnected = true;
	} 
	catch (CMSException& e) {
		LOG_ERROR("Could not connect to message queue \"" << m_brokerUri << "\" with username=\"" << m_username << "\"")
		m_bConnected = false;
	}

	LOG_INFO("Connection initialized")

	return m_bConnected;
}

void MonitorPlugInActiveMQ::freeConnection() {
	LOG_INFO("Freeing connection")

	// close open resources
	try {
		if (m_session != NULL) {
			LOG_DEBUG("Closing session..")
			m_session->close();
		}

		if (m_connection != NULL) {
			LOG_DEBUG("Closing connection...")
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
	LOG_INFO("Parsing generic topic configuration");
	parseTopic(config, m_genericTopic, m_genericTopic);

	// read inherited topic configuration
	LOG_INFO("Parsing individual topic configuration");
	parseTopics(config, m_topics, m_genericTopic);

	initializeConfiguration(config);
	initializeConnectionFactory();
	bool r = establishConnection();

	return r;
}

bool MonitorPlugInActiveMQ::establishConnection() {
	// if some connection issue has occured, try to reconnect to the broker
	try {
		if (m_connection != NULL) {
			LOG_DEBUG("Checking for closed connection")
			m_connection->checkClosed();
		}

		if (m_session != NULL && (false == m_session->isStarted())) {
			throw CMSException("Connection open but session not started");
		}
		
	}
	catch (CMSException& ex) {
		LOG_ERROR("Connection is broke:" << ex.getMessage())
		LOG_INFO("Freeing resources for reconnecting...")
		freeConnection();
	}

	if (m_bConnected == false) {
		LOG_DEBUG("m_bConnected is false: \"" << m_bConnected << "\"")
	
		if (initializeConnection()) {
			initializeTopics(m_topics);
		}
	}

	return m_bConnected;
}

/**
 * Iterate over every topic and initialize the topic/queue for the ActiveMQ connection
 */
void MonitorPlugInActiveMQ::initializeTopics(Topics &topics)
{
	LOG_INFO("Initializing topics")

	if (m_bConnected == false) {
		throw RuntimeException(__FILE__, __LINE__, "Tried to initialize topics without an established ActiveMQ connection. Call initializeActiveMqConnection() first.");
	}

	Topics::iterator i;
	TopicInfo *pTopicInfo;

	LOG_INFO("Number of topics to initialize: " << topics.size())

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

/**
 * Read the <topic type="...">...</topic> tag instances and produce a new Topic instance for every tag
 */
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
	
	// an ActiveMQ topic/queue must be mapped to valid soundcard channel
	for (unsigned int i = 0, m = channels.size(); i < m; i++)
	{
		pTopicInfo = new TopicInfo;
		LOG_INFO("Initializing topic for channel num #" << i) 
		initializeTopic(*pTopicInfo, referenceTopic);
		topics.insert(PairMapping(channels.at(i), pTopicInfo));
	}

	int nTopic = config.nChildNode(ACTIVEMQ_XMLNODE_TOPIC);
	LOG_INFO("Available topics for channel: " << nTopic)

	for (int num = 0; num < nTopic ; ++num)
	{
		if (!((topicNode = config.getChildNode(ACTIVEMQ_XMLNODE_TOPIC, num))).isEmpty()) {
			std::string type = topicNode.getAttribute(ACTIVEMQ_XMLATTR_TYPE) ;
			
			if ((type == ACTIVEMQ_KEY_POCSAG) || (type == ACTIVEMQ_KEY_FMS) || (type == ACTIVEMQ_KEY_ZVEI)) {
				LOG_INFO("Topic definition has valid type \"" << type << "\"")
				pTopicInfo = (topics.find(type))->second;
				parseTopic(topicNode, *pTopicInfo, referenceTopic);
			}
		}
	}
}

void MonitorPlugInActiveMQ::parseTopic(XMLNode config, TopicInfo &topicInfo, TopicInfo &referenceTopic)
{
	initializeTopic(topicInfo, referenceTopic);
	LOG_INFO("Parsing topic definition")

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
