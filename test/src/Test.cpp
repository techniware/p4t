//
// O3SServer.cpp
//
#include <iostream>
#include "Poco/Environment.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/LoggingSubsystem.h"
#include "Poco/OSP/OSPSubsystem.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Thread.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/Delegate.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/NetworkInterface.h"
#include "Poco/Net/UDPChannel.h"
#include "Poco/DNSSD/DNSSDResponder.h"
#include "Poco/DNSSD/DNSSDBrowser.h"
#include "Poco/DNSSD/DNSSDException.h"
#if POCO_OS == POCO_OS_LINUX && !defined(POCO_DNSSD_USE_BONJOUR)
#include "Poco/DNSSD/Avahi/Avahi.h"
#else
#include "Poco/DNSSD/Bonjour/Bonjour.h"
#endif
#include "Poco/Remoting/ORB.h"
#include "Poco/Remoting/SoapLite/TransportFactory.h"
#include "Poco/Remoting/SoapLite/Listener.h"

#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/Util/LoggingConfigurator.h"
#include "Poco/Semaphore.h"


using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::OSP::OSPSubsystem;
using Poco::OSP::ServiceRegistry;

using namespace std;

class O3SServerApplication: public Poco::Util::ServerApplication {  
public:

	O3SServerApplication():
		_errorHandler(*this),
		_pOSP(new OSPSubsystem),
		_showHelp(false),
		_masterIPResolved(false) {
		setUnixOptions(true);
		Poco::ErrorHandler::set(&_errorHandler);

	}
	
	ServiceRegistry& serviceRegistry() {
		return _pOSP->serviceRegistry();
	}

protected:
	class ErrorHandler: public Poco::ErrorHandler {
	public:
		ErrorHandler(O3SServerApplication& app):
			_app(app) {
		}
		
		void exception(const Poco::Exception& exc) {
			log(exc.displayText());
		}

		void exception(const std::exception& exc) {
			log(exc.what());
		}

		void exception() {
			log("unknown exception");
		}
		
		void log(const std::string& message) {
			_app.logger().error("A thread was terminated by an unhandled exception: " + message);
		}
		
	private:
		O3SServerApplication& _app;
	};

	void initialize(Poco::Util::Application& self)	{
		// load default configuration files, if present
		loadConfiguration();
        config().setString("application.logger", config().getString("node.name")+ " node");
        Poco::Util::Application::initialize(self);

		// Adding OSP subsystem
		addSubsystem(_pOSP);

		// Initializing ZeroConf services
		initializeZeroConf();
        
        // Register soap remoting
		std::string wsdlPath   = config().getString("o3s.codeCache","codeCache");
        _dnssdConfiguration["localhost:9999"] = config().getString("node.ip")+":"+config().getString("node.soap.port","9999");
		Poco::Remoting::SoapLite::TransportFactory::registerFactory();
		Poco::Remoting::SoapLite::Listener *listener = new Poco::Remoting::SoapLite::Listener(wsdlPath,config().getInt("node.soap.port",9999),_dnssdConfiguration);
		Poco::Remoting::ORB::instance().registerListener(listener);
        Poco::Util::Application::initialize(self);

	}

	void defineOptions(OptionSet& options)	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<O3SServerApplication>(this, &O3SServerApplication::handleHelp)));

		options.addOption(
			Option("config-file", "c", "load configuration data from a file")
				.required(false)
				.repeatable(true)
				.argument("file")
				.callback(OptionCallback<O3SServerApplication>(this, &O3SServerApplication::handleConfig))
		);

		options.addOption(
			Option("node-name", "n", "name of the O3S node")
				.required(false)
				.repeatable(false)
				.argument("node")
				.binding("o3s.node")
		);
				

		options.addOption(
			Option("server-name", "s", "name of the O3S server")
				.required(false)
				.repeatable(false)
				.argument("server")
				.binding("o3s.server") 
		);

		options.addOption(
			Option("master-ip", "m", "Master address IP")
				.required(false)
				.repeatable(false)
				.argument("file")
				.binding("master-ip")
				.callback(OptionCallback<O3SServerApplication>(this, &O3SServerApplication::handleMasterIPConfig))
		);

		options.addOption(
			Option("local-ip", "l", "Local address IP in master network")
				.required(false)
				.repeatable(false)
				.argument("file")
				.binding("local-ip")
				.callback(OptionCallback<O3SServerApplication>(this, &O3SServerApplication::handleLocalIPConfig))
		);
	}
	
	void handleHelp(const std::string& name, const std::string& value) {
		_showHelp = true;
		displayHelp();
		stopOptionsProcessing();
		_pOSP->cancelInit();
	}

	void handleNodeConfig(const std::string& name, const std::string& value){
		_nodeName = value;
	}

	
	void handleMasterIPConfig(const std::string& name, const std::string& value){
		_masterIP = value;
	}

		
	void handleLocalIPConfig(const std::string& name, const std::string& value){
		_localIP = value;
	}

	void handleConfig(const std::string& name, const std::string& value){
		loadConfiguration(value);
	}

	void displayHelp() {
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A container for Open System Simulation Solution bundles.");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string>& args){
		if (!_showHelp) {
			waitForTerminationRequest();
		}
		return Poco::Util::Application::EXIT_OK;
	}

	void onServiceFound(const void* sender, const Poco::DNSSD::DNSSDBrowser::ServiceEventArgs& args) {
		logger().information("On service found");
		reinterpret_cast<Poco::DNSSD::DNSSDBrowser*>(const_cast<void*>(sender))->resolve(args.service);
		logger().information("Resolved");
	}

	void onServiceRemoved(const void* sender, const Poco::DNSSD::DNSSDBrowser::ServiceEventArgs& args) {
	}

	void onServiceResolved(const void* sender, const Poco::DNSSD::DNSSDBrowser::ServiceEventArgs& args) {
		logger().notice("Resolving host");
		if ( args.service.properties().get("master", "UNKNOWN").compare("true") == 0 ) {
			config().setString("master.name",        	  	 args.service.properties().get("name",       "UNKNOWN"));
			config().setString("master.ip",          	  	 args.service.properties().get("ip"      ,   "UNKNOWN"));
			config().setString("master.http.port",   	  	 args.service.properties().get("httpPort",   "UNKNOWN"));
			config().setString("master.soap.port",   	  	 args.service.properties().get("soapPort",   "UNKNOWN"));
            config().setString("master.status.port",   	  	 args.service.properties().get("statusPort", "40004"));
			config().setString("d3s.synchronizer.group",  	 args.service.properties().get("mGroup",     "UNKNOWN"));
			config().setString("d3s.synchronizer.port",	  	 args.service.properties().get("mPort",      "UNKNOWN"));
            config().setString("d3s.command.port",	 	  	 args.service.properties().get("cPort",      "UNKNOWN"));
            config().setString("d3s.synchronizer.period", 	 args.service.properties().get("syncPeriod", "10"));
            config().setString("d3s.synchronizer.checkdate", args.service.properties().get("checkDate",  "0"));
            config().setString("master.hostname", 		  	 args.service.properties().get("hostname",   "UNKNOWN"));
            config().setString("master.os",   			  	 args.service.properties().get("os",	      "UNKNOWN"));
            config().setString("master.os.version",   	  	 args.service.properties().get("osVersion",  "UNKNOWN"));
            config().setString("master.os.architecture",  	 args.service.properties().get("arch",		  "UNKNOWN"));
            config().setString("master.processors",   	  	 args.service.properties().get("proc",       "UNKNOWN"));

            _masterName  = config().getString("master.name");
            _masterIP    = config().getString("master.ip");
            _brokerGroup = config().getString("d3s.synchronizer.group");
            _dictionaryPort  = config().getString("d3s.synchronizer.port");
            _d3sCommandPort   = config().getString("d3s.command.port");
            _masterIPResolved = true;
            logger().information("Master node resolved");
            logger().information(" -> name                    : " + config().getString("master.name"));
            logger().information(" -> ip                      : " + config().getString("master.ip"));
            logger().information(" -> http.port               : " + config().getString("master.http.port"));
            logger().information(" -> soap.port               : " + config().getString("master.soap.port"));
            logger().information(" -> master.status.port      : " + config().getString("master.status.port"));
            logger().information(" -> broker.group            : " + config().getString("d3s.synchronizer.group"));
            logger().information(" -> broker.port             : " + config().getString("d3s.synchronizer.port"));
            logger().information(" -> d3s.command.port        : " + config().getString("d3s.command.port"));
            logger().information(" -> d3s.synchronizer.period : " + config().getString("d3s.synchronizer.period"));
            logger().information(" -> d3s.synchronizer.checkdate: " + config().getString("d3s.synchronizer.checkdate"));
        } else {
            std::string nodeName = args.service.properties().get("name", "UNKNOWN");
            std::string nodeList = config().getString("nodeList","");
            std::string nodeIP   = config().getString(nodeName+".ip","");
            if (nodeIP == "" ) {
				if (nodeList != "") {
					nodeList = nodeList+";" ;
				}
	            nodeList = nodeList + nodeName;
            }
            config().setString(nodeName+".hostname", 		args.service.properties().get("hostName",  "UNKNOWN"));
            config().setString(nodeName+".os",   			args.service.properties().get("os",        "UNKNOWN"));
            config().setString(nodeName+".os.version",   	args.service.properties().get("osVersion", "UNKNOWN"));
            config().setString(nodeName+".os.architecture", args.service.properties().get("arch",      "UNKNOWN"));
            config().setString(nodeName+".processors",   	args.service.properties().get("proc",      "UNKNOWN"));
            config().setString(nodeName+".name",	    	args.service.properties().get("name",      "UNKNOWN"));
            config().setString(nodeName+".ip",          	args.service.properties().get("ip",        "UNKNOWN"));
            config().setString(nodeName+".http.port",   	args.service.properties().get("httpPort",  "UNKNOWN"));
            config().setString(nodeName+".soap.port",   	args.service.properties().get("soapPort",  "UNKNOWN"));
            config().setString(nodeName+".status.port",   	args.service.properties().get("statusPort",  "40005"));

            logger().information(nodeName+" RTS resolved");
            logger().information(" -> ip           : " + config().getString(nodeName+".ip"));
            logger().information(" -> http.port    : " + config().getString(nodeName+".http.port"));
            logger().information(" -> soap.port    : " + config().getString(nodeName+".soap.port"));

            config().setString("nodeList",         nodeList);

        }
		logger().notice("Resolving host done");
	}

	void onHostResolved(const void* sender, const Poco::DNSSD::DNSSDBrowser::ResolveHostEventArgs& args) {
	}

	void registerNodePresentationService() {
		logger().information("Registering DNSSD presentation service for "+_nodeName+": "+_localIP);

		try {
			Poco::Net::IPAddress			 address(_localIP);
			Poco::Net::NetworkInterface		 networkInterface = Poco::Net::NetworkInterface::forAddress(address);
			Poco::Int32						 interface		  = networkInterface.index();

			logger().information("Creating DNSSD service properties for "+_nodeName+"("+address.toString()+":35091)");
			_properties.add("hostname", Poco::Environment::nodeName());
			_properties.add("os",   	Poco::Environment::osName());
			_properties.add("osVersion",  Poco::Environment::osVersion());
			_properties.add("arch", 	Poco::Environment::osArchitecture());
			_properties.add("proc",     Poco::format("%?d",Poco::Environment::processorCount()));
			_properties.add("name",	    _nodeName);
			_properties.add("ip",	    address.toString());
			_properties.add("httpPort", _httpPort);
			_properties.add("soapPort", _soapPort);
			_properties.add("statusPort", _statusPort);

			if ( _isMaster ) {
				_properties.add("master",   "true");
				_properties.add("mGroup",   _brokerGroup);
				_properties.add("mPort",	_dictionaryPort);
				_properties.add("cPort",	  _d3sCommandPort);
				_properties.add("syncPeriod", config().getString("d3s.synchronizer.period",    "10"));
				_properties.add("checkDate",  config().getString("d3s.synchronizer.checkdate", "0"));

			} else {
				_properties.add("master",   "false");
			}
			std::string serverName = config().getString("node.http.name", _nodeName);
			int port               = config().getInt("node.http.port", 22080);
			_dnssdService 		   = new Poco::DNSSD::Service (interface,_nodeName,"","_o3s._tcp","","",35091,_properties);
			_dnssdServiceHandle    = _dnssdResponder.registerService(*_dnssdService);
			_httpService           = new Poco::DNSSD::Service(interface, serverName,"","_http._tcp","","", port);
			_dnssdResponder.registerService(*_httpService);

		} catch (Poco::Exception &e ) {
			logger().error(e.displayText());
			throw e;
		} catch (std::exception &e ) {
			logger().error( e.what() );
			throw e;
		}
	}

	void registerGetMasterInfoService() {
		_dnssdResponder.browser().serviceResolved  += Poco::delegate(this, &O3SServerApplication::onServiceResolved);
		_dnssdResponder.browser().serviceFound     += Poco::delegate(this, &O3SServerApplication::onServiceFound);
		_dnssdResponder.browser().serviceRemoved   += Poco::delegate(this, &O3SServerApplication::onServiceRemoved);
		_dnssdResponder.browser().hostResolved     += Poco::delegate(this, &O3SServerApplication::onHostResolved);
	}

	void initializeZeroConf() {
		try {
			// Reading common properties
			_nodeName	= config().getString("node.name");
			_httpPort   = config().getString("node.http.port","22080");
			_soapPort   = config().getString("node.soap.port","9999");
			if  ( config().getInt("node.simulator",1) == 0) {
				_isMaster       = true;
				_masterName 	= _nodeName;
                try {
                    // Check if IP has been forced
				    _localIP    	= config().getString("node.ip");
                } catch (Poco::NotFoundException &) {
                    #if defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
                        std::string ifName = config().getString("node.interface","eth0");
                    #elif defined(_WIN32)
                        std::string ifName = config().getString("node.interface","none");
                        // No interface specified
                        if (ifName.compare("none") == 0 ) {
                            // Looking for first interface with ip V4 assigned
                            Poco::Net::NetworkInterface::List interfaces = Poco::Net::NetworkInterface::list();
                            for (auto netInterface : interfaces ) {
                                std::string ipAddress= netInterface.address().toString();
                                if (     netInterface.supportsIPv4()  
                                      && (ipAddress.compare("127.0.0.1")!=0 ) ) {
                                    ifName = netInterface.name();
                                    logger().warning("Automatically selecting interface "+ ifName+": "+netInterface.address().toString());
                                    break;
                                }
                            }
                        }

                    #else 
                        std::string ifName = config().getString("node.interface","en1");
                    #endif
                    Poco::Net::NetworkInterface netInterface = Poco::Net::NetworkInterface::forName(ifName);
                    _localIP = netInterface.address().toString();
                    config().setString("node.ip",_localIP);
                }
				_masterIP   	= _localIP;
				// Getting D3S configuration
                _d3sCommandIP   = config().getString("d3s.command.ip",         _localIP);
                _d3sCommandPort = config().getString("d3s.command.port",       "12350");
  				_brokerGroup 	= config().getString("d3s.synchronizer.group", "239.255.1.2");
				_dictionaryPort	= config().getString("d3s.synchronizer.port",  "12345");
				_statusPort		= config().getString("node.status.port",       "40005");
				config().setString("master.name",        	_nodeName);
				config().setString("master.ip",          	_masterIP);
				config().setString("master.http.port",   	_httpPort);
				config().setString("master.soap.port",   	_soapPort);
				config().setString("d3s.synchronizer.group",_brokerGroup);
				config().setString("d3s.synchronizer.port",	_dictionaryPort);
				config().setString("d3s.command.port",		_d3sCommandPort);
				logger().information("Starting DNDSD Responder for master :"+_masterName);
				registerNodePresentationService();
				registerGetMasterInfoService();
				_dnssdResponder.start();
				_dnssdResponder.browser().browse("_o3s._tcp", "", 0, 0);
			} else {
				_isMaster       = false;
				_statusPort		= config().getString("node.status.port",       "40004");
				logger().information("Starting DNDSD Responder for "+_nodeName);
				registerGetMasterInfoService();
				_dnssdResponder.start();
				_dnssdResponder.browser().browse("_o3s._tcp", "", 0, 0);
				waitForMasterIPResolution();
				logger().information("Master node ip resolved :"+_masterIP);
				Poco::Net::IPAddress masterAddress(_masterIP);
				logger().information("Looking for local address in master network");
				Poco::Net::IPAddress localAddress =  masterAddress.getLocalAddressInNetwork();
				_localIP = localAddress.toString();
				config().setString("node.ip",_localIP);
				logger().information("Local IP choosen :"+_localIP);
				registerNodePresentationService();
			}
		} catch (Poco::DNSSD::DNSSDException &e) {
			logger().error("Error while starting DNSSD responder:"+e.displayText());
			throw e;
		} catch (Poco::Net::NoAddressFoundException &e) {
			logger().error("No local IP found for "+_masterIP);
			logger().error(e.displayText());
			_localIP	=  _masterIP;
            config().setString("node.ip",_localIP);
            logger().information("Local IP choosen :"+_localIP);
            registerNodePresentationService();

		} catch (Poco::Exception &e) {
			logger().error(e.displayText());
			throw e;
		}
	}

	void waitForMasterIPResolution() {
		while ( ! _masterIPResolved ) {
			logger().information("Waiting for Master IP resolution");
			Poco::Thread::sleep(1000);
		}
		logger().information("Received master IP information");
	}

private:
	ErrorHandler								  _errorHandler;
	OSPSubsystem*								  _pOSP;
	Poco::Thread				    			  _thread;
	Poco::RunnableAdapter<O3SServerApplication> * _runnableAdapter;
	bool										  _showHelp;
	bool										  _isMaster;
	bool										  _masterIPResolved;
	std::string							  		  _nodeName;
	std::string							  		  _masterName;
	std::string							  		  _masterIP;
	std::string							  		  _localIP;
    std::string							  		  _d3sCommandIP;
    std::string							  		  _d3sCommandPort;
	std::string							  		  _brokerGroup;
	std::string				                      _dictionaryPort;
	std::string				                      _soapPort;
	std::string				                      _httpPort;
	std::string				                      _statusPort;
	std::map<string,std::string>				  _dnssdConfiguration;
	Poco::DNSSD::DNSSDResponder			  		  _dnssdResponder;
	Poco::DNSSD::Service*						  _dnssdService;
	Poco::DNSSD::Service*						  _httpService;
	Poco::DNSSD::ServiceHandle			  		  _dnssdServiceHandle ;
	Poco::DNSSD::Service::Properties 			  _properties;
};

int main(int argc, char** argv) {
	try {
		Poco::Net::UDPChannel::registerChannel();
		Poco::DNSSD::initializeDNSSD();
		O3SServerApplication().run(argc, argv);
		return Poco::Util::Application::EXIT_OK;
	} catch( Poco::DNSSD::DNSSDException &e ) {
		Poco::Logger::get("O3S").error(e.displayText());
		return Poco::Util::Application::EXIT_CONFIG;
	} catch( std::exception &e ) {
		Poco::Logger::get("O3S").error(e.what());
		return Poco::Util::Application::EXIT_CONFIG;
	}
}
