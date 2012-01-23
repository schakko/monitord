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

	m_connection = NULL;
}

void MonitorPlugInActiveMQ::Show()
{
	FILE_LOG(logINFO) << "MonitorActiveMQPlugin successfully loaded" ;
}

bool MonitorPlugInActiveMQ::processResult(class ModuleResultBase *pRes)
{
	FILE_LOG(logDEBUG) << "apachemq: processing Result...";

	if (m_bConnected == false) {
		FILE_LOG(logERROR) << "apachmq: ignoring message 'cause no active connection";
		return false;
	}

	if (m_bTopicsInitialized == false) {
		throw RuntimeException(__FILE__, __LINE__, "processResult must be called AFTER initializeTopics()");
	}


	std::string type = (*pRes)["typ"];

	if (m_topics.find(type) == m_topics.end()) {
		FILE_LOG(logERROR) << "apachemq: received type " << type << " which is not registered";
		return false;
	}
	
	FILE_LOG(logINFO) << "Preparing new message";
	
	TopicInfo* topicInfo = (m_topics.find(type))->second;
	TextMessage* message = m_session->createTextMessage();

	ResultItemsMap::iterator i;

	for (i = (*pRes).m_Items.begin(); i != (*pRes).m_Items.end(); i++) {
		message->setStringProperty(i->first, i->second);
	}


	topicInfo->producer->send(message);
	
	FILE_LOG(logINFO) << "message sent";

	delete message;

	return true;
}

void MonitorPlugInActiveMQ::initializeConfiguration(XMLNode config)
{	
	FILE_LOG(logDEBUG) << "Reading broker URI";
	m_brokerUri 	= getNodeText(config, ACTIVEMQ_XMLNODE_BROKERURI, "tcp://127.0.0.1:61616");
	FILE_LOG(logDEBUG) << "Reading username:";
	m_username 		= getNodeText(config, ACTIVEMQ_XMLNODE_USERNAME, "");
	FILE_LOG(logDEBUG) << "Reading password";
	m_password 		= getNodeText(config, ACTIVEMQ_XMLNODE_PASSWORD, "");
	FILE_LOG(logDEBUG) << "Reading clientId";
	m_clientId 		= getNodeText(config, ACTIVEMQ_XMLNODE_CLIENTID, "");
	FILE_LOG(logDEBUG) << "Reading sendTimeout";
	m_sendTimeout 	= getNodeInt(config, ACTIVEMQ_XMLNODE_SENDTIMEOUT, 0);
	FILE_LOG(logDEBUG) << "Reading closeTimeout";
	m_closeTimeout 	= getNodeInt(config, ACTIVEMQ_XMLNODE_CLOSETIMEOUT, 0);
	FILE_LOG(logDEBUG) << "Reading producerWindowSize";
	m_producerWindowSize 	= getNodeInt(config, ACTIVEMQ_XMLNODE_PRODUCERWINDOWSIZE, 0);
	FILE_LOG(logDEBUG) << "Reading bUseCompression";
	m_bUseCompression 		= getNodeBool(config, ACTIVEMQ_XMLNODE_USECOMPRESSION, false);
	FILE_LOG(logDEBUG) << "Reading bClientAck";
	m_bClientAck 	= getNodeBool(config, ACTIVEMQ_XMLNODE_CLIENTACK, false);
	FILE_LOG(logDEBUG) << "Reading logFile";
	std::string logFile		= getNodeText(config, ACTIVEMQ_XMLNODE_LOGFILE, "screen");
	FILE_LOG(logDEBUG) << "Reading logLevel";
	std::string logLevel	= getNodeText(config, ACTIVEMQ_XMLNODE_LOGLEVEL, "INFO");

	#ifdef WIN32
	if (!(logFile == "screen")) {
		FILE* pFile = fopen(logFile.c_str(), "a");
		Output2FILE::Stream() = pFile;
	}

	FILELog::ReportingLevel() = FILELog::FromString(logLevel);
	FILE_LOG(logINFO) << "logging started";
	#endif
}

Topics MonitorPlugInActiveMQ::getTopics()
{
	return m_topics;
}

void MonitorPlugInActiveMQ::setTopics(Topics& topics)
{
	m_topics = topics;
}

void MonitorPlugInActiveMQ::initializeConnectionFactory(ActiveMQConnectionFactory *connectionFactory)
{
	// Set Broker-URI first - otherwise username and password is lost
	connectionFactory->setBrokerURI(m_brokerUri);

	if (!m_username.empty()) {
		connectionFactory->setUsername(m_username);
	}

	if (!m_password.empty()) {
		connectionFactory->setPassword(m_password);
	}

	if (!m_clientId.empty()) {
		connectionFactory->setClientId(m_clientId);
	}

	connectionFactory->setUseCompression(m_bUseCompression);
	connectionFactory->setSendTimeout(m_sendTimeout);
	connectionFactory->setCloseTimeout(m_closeTimeout);
	connectionFactory->setProducerWindowSize(m_producerWindowSize);
	
	FILE_LOG(logDEBUG) << "Connection factory initialized";
}

bool MonitorPlugInActiveMQ::initializeActiveMqConnection()
{
	auto_ptr<ActiveMQConnectionFactory> connectionFactory(new ActiveMQConnectionFactory());
	initializeConnectionFactory(connectionFactory.get());

	// create a connection
	try {
        m_connection = connectionFactory->createConnection();
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
		FILE_LOG(logERROR) << "Could not connect to messaging queue \"" << m_brokerUri << "\" with username=\"" << m_username << "\"";
		m_bConnected = false;
	}

	FILE_LOG(logDEBUG) << "Connection initialized";

	return m_bConnected;
}

bool MonitorPlugInActiveMQ::initProcessing(class MonitorConfiguration* configPtr, XMLNode config)
{
	// initialize ActiveMQ
	activemq::library::ActiveMQCPP::initializeLibrary();

	// read default configuration for topics
	parseTopic(config, m_genericTopic, m_genericTopic);

	initializeConfiguration(config);

	if (initializeActiveMqConnection()) {
		parseTopics(config, m_topics, m_genericTopic);
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
		
		FILE_LOG(logINFO) << "Topic destination \"" << pTopicInfo->destination << "\" created";
	}

	FILE_LOG(logINFO) << "Topics initialized";
		
	m_bTopicsInitialized = true;
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

void MonitorPlugInActiveMQ::initializeTopic(TopicInfo &topicInfo, TopicInfo &referenceTopic)
{
	topicInfo.bUseTopic = referenceTopic.bUseTopic;
	topicInfo.bDeliveryModePersistent = referenceTopic.bDeliveryModePersistent;
	topicInfo.destUri = referenceTopic.destUri;
	topicInfo.destination = NULL;
	topicInfo.producer = NULL;
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
