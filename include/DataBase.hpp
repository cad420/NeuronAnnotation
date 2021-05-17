#ifndef NEURONANNOTATION_DATABASE_H
#define NEURONANNOTATION_DATABASE_H
#include <string.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <Poco/MongoDB/Array.h>
#include <Poco/MongoDB/Connection.h>
#include <Poco/MongoDB/Cursor.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/Document.h>
// #include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/ObjectId.h>
#include <Poco/MongoDB/PoolableConnectionFactory.h>
#include <Poco/UUIDGenerator.h>
#include "AnnotationDS.hpp"

#define MAX_LINE_SIZE 4096

class DataBase{
private:
    static std::string name;
    static std::string host;
    static std::string port;
	static bool connected;

public:
	static std::string getName();
	static std::string getHost();
	static std::string getPort();
	static void setName(const std::string &value);
	static void setHost(const std::string &value);
	static void setPort(const std::string &value);

public:

	DataBase(){};

	static void connect();

	/**
	 * @return show the DataBase whether is connected or not.
	 * */

	static bool isConnected();

	/**
	 * @param swc : a swc document which is already in Database
	 * @param tableName : table which need to be searched
	 * @return success:true error:false
	 **/
	static bool modifySWC(const NeuronSWC &swc, const std::string &tableName);

	/**
	 * @param swcs : swc documents which are already in Database\
	 * @param tableName : table which need to be searched
	 * @return success:true error:false
	 **/
	static bool modifySWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName);

	/**
	 * @param swc : add a new Neuron by swc
	 * @param tableName : table which need to be searched
	 * @return success:true error:false
	 **/
	static bool insertSWC(const NeuronSWC &swc, const std::string &tableName);

	/**
	 * @param swcs : add new Neurons by swc
	 * @param tableName : table which need to be searched
	 * @return success:true error:false
	 **/
	static bool insertSWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName);

	/**
	 * @param swc : delete a Neuron by swc
	 * @param tableName : table which need to be searched
	 * @return success:true / error:false
	 **/
	static bool deleteSWC(const NeuronSWC &swc, const std::string &tableName);

	/**
	 * @param swcs : delete Neurons by swc
	 * @param tableName : table which need to be searched
	 * @return success:true error:false
	 **/
	static bool deleteSWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName);


	/**
	 * @param tableName : table which need to be parsed into .swc file
	 * @return swc file string content
	 **/
	static std::string getSWCFileStringFromTable(const std::string &tableName);

	/**
	 * @param tableName : table which need to be found.
	 * @return is in database:true  / not in database:false
	 **/
	static bool findTable(const std::string &tableName);

	/**
	 * @param swc : swc which need to be found.
	 * @param tableName : table which need to be searched
	 * @return is in database:true  / not in database:false
	 **/
	static bool findSWC(const NeuronSWC &swc, const std::string &tableName);
	
	
	/**
	 *  @return tables in database json format
	 **/
	static std::string showAllTables();

private:
	// typedefs
	typedef Poco::PoolableObjectFactory<Poco::MongoDB::Connection,
					Poco::MongoDB::Connection::Ptr>
	MongoDBConnectionFactory;
	typedef std::unique_ptr<MongoDBConnectionFactory>
	MongoDBConnectionFactoryPtr;

	typedef Poco::ObjectPool<Poco::MongoDB::Connection,
				 Poco::MongoDB::Connection::Ptr>
	MongoDBConnectionPool;
	typedef std::unique_ptr<MongoDBConnectionPool> MongoDBConnectionPoolPtr;

	// variables & objects
	static bool isPoolRunning;
	static size_t poolCapacity;
	static size_t poolPeakCapacity;
	static MongoDBConnectionFactoryPtr g_connectionFactory;
	static MongoDBConnectionPoolPtr g_connectionPool;
	static Poco::MongoDB::Database g_db;

private:
	static Poco::MongoDB::PooledConnection takeConnection();
	/**
	 * @brief extractInt64
	 * @param d
	 * @param name
	 * @return
	 */

	/**
	 * @brief verifyResponse
	 * @param response
	 * @param expectOK
	 */
	static bool verifyResponse(const Poco::MongoDB::Document &response,
					  bool expectOK = true);


};

#endif