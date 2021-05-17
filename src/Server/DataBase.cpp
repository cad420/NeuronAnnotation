#include <DataBase.hpp>
#include <memory>

using Poco::Int64;

std::string DataBase::port("27017");
std::string DataBase::host("127.0.0.1");
std::string DataBase::name("Test");
bool DataBase::connected(false);
bool DataBase::isPoolRunning(false);
size_t DataBase::poolCapacity(16);
size_t DataBase::poolPeakCapacity(256);
DataBase::MongoDBConnectionFactoryPtr DataBase::g_connectionFactory;
DataBase::MongoDBConnectionPoolPtr DataBase::g_connectionPool;
Poco::MongoDB::Database DataBase::g_db(DataBase::name);

void DataBase::connect()
{
    std::cout << "Conneted to :" << host << ":" << port << endl;
    string connetsionStr = host + std::string(":") + port;
	g_connectionFactory.reset(new MongoDBConnectionFactory(connetsionStr));
	g_connectionPool.reset(new MongoDBConnectionPool(
	*g_connectionFactory, poolCapacity, poolPeakCapacity));
    connected = true;
}

bool DataBase::isConnected(){
    return connected;
}

std::string DataBase::getName(){
    return name;
}

std::string DataBase::getHost(){
    return host;
}

std::string DataBase::getPort(){
    return port;
}

void DataBase::setName(const std::string &value){
    name = value;
}

void DataBase::setHost(const std::string &value){
    host = value;
}

void DataBase::setPort(const std::string &value){
    port = value;
}

bool DataBase::modifySWC(const NeuronSWC &swc, const std::string &tableName){
    std::cout << "[modifySWC]" << std::endl;
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    Poco::MongoDB::Document::Ptr swcObj(new Poco::MongoDB::Document());
    swcObj->add("_id",swc.id);
    swcObj->add("name",swc.name);
    swcObj->add("line_id",swc.line_id);
    swcObj->add("x",swc.x);
    swcObj->add("y",swc.y);
    swcObj->add("z",swc.z);
    swcObj->add("parent",swc.pn);
    swcObj->add("color",swc.color);
    swcObj->add("user_id",swc.user_id);
    swcObj->add("timestamp",swc.timestamp);
    swcObj->add("seg_id",swc.seg_id);
    swcObj->add("seg_in_id",swc.seg_in_id);
    swcObj->add("seg_size",swc.seg_size);
    swcObj->add("type",int(swc.type));
    swcObj->add("radius",swc.radius);
    std::cout << "[modifySWC] MODIFY:";
    std::cout << swcObj->toString() << std::endl;

    Poco::MongoDB::Document::Ptr document(new Poco::MongoDB::Document());
    document->add("$set", swcObj);

    Poco::MongoDB::Document::Ptr query(new Poco::MongoDB::Document());
    query->add("_id", swc.id);

    Poco::MongoDB::Document::Ptr update(new Poco::MongoDB::Document());
    update->add("q", query); //.add("limit", 1);
    update->add("u", document);

    Poco::MongoDB::Array::Ptr updates(new Poco::MongoDB::Array());
    updates->add(std::to_string(0), update);

    auto updateCMD = g_db.createCommand();
    updateCMD->selector().add("update",tableName).add("updates",updates);
    Poco::MongoDB::ResponseMessage response;

    c->sendRequest(*updateCMD,response);
    auto doc = *(response.documents()[0]);
    std::cout << doc.toString() << std::endl;

    if( doc.getInteger("ok") == 1 ) return true;
    return false;
}

bool DataBase::modifySWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName){
    std::cout << "[modifySWCs]" << std::endl;
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);    
    Poco::MongoDB::Array::Ptr updates(new Poco::MongoDB::Array());

    int index = 0;
    for( auto swc = swcs.begin() ; swc != swcs.end() ; swc ++ ){
        Poco::MongoDB::Document::Ptr swcObj(new Poco::MongoDB::Document());
        swcObj->add("_id",(*swc)->id);
        swcObj->add("name",(*swc)->name);
        swcObj->add("line_id",(*swc)->line_id);
        swcObj->add("x",(*swc)->x);
        swcObj->add("y",(*swc)->y);
        swcObj->add("z",(*swc)->z);
        swcObj->add("parent",(*swc)->pn);
        swcObj->add("color",(*swc)->color);
        swcObj->add("user_id",(*swc)->user_id);
        swcObj->add("timestamp",(*swc)->timestamp);
        swcObj->add("seg_id",(*swc)->seg_id);
        swcObj->add("seg_in_id",(*swc)->seg_in_id);
        swcObj->add("seg_size",(*swc)->seg_size);
        swcObj->add("type",int((*swc)->type));
        swcObj->add("radius",(*swc)->radius);

        Poco::MongoDB::Document::Ptr document(new Poco::MongoDB::Document());
        document->add("$set", swcObj);

        Poco::MongoDB::Document::Ptr query(new Poco::MongoDB::Document());
        query->add("_id", (*swc)->id);

        Poco::MongoDB::Document::Ptr update(new Poco::MongoDB::Document());
        update->add("q", query);//.add("limit", 1);
        update->add("u", document);

        updates->add(std::to_string(index++), update);
    }

    auto updateCMD = g_db.createCommand();
    updateCMD->selector().add("update",tableName).add("updates",updates);
    Poco::MongoDB::ResponseMessage response;

    c->sendRequest(*updateCMD,response);
    auto doc = *(response.documents()[0]);
    std::cout << doc.toString() << std::endl;

    if( doc.getInteger("ok") == 1 ) return true;
    return false;
}

bool DataBase::insertSWC(const NeuronSWC &swc, const std::string &tableName){
    if( findSWC(swc,tableName) ) return false;

    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);
    Poco::MongoDB::Document::Ptr swcObj(new Poco::MongoDB::Document());
    swcObj->add("_id",swc.id);
    swcObj->add("name",swc.name);
    swcObj->add("line_id",swc.line_id);
    swcObj->add("x",swc.x);
    swcObj->add("y",swc.y);
    swcObj->add("z",swc.z);
    swcObj->add("parent",swc.pn);
    swcObj->add("color",swc.color);
    swcObj->add("user_id",swc.user_id);
    swcObj->add("timestamp",swc.timestamp);
    swcObj->add("seg_id",swc.seg_id);
    swcObj->add("seg_in_id",swc.seg_in_id);
    swcObj->add("seg_size",swc.seg_size);
    swcObj->add("type",int(swc.type));
    swcObj->add("radius",swc.radius);
    std::cout << "[insertSWC] INSERT:";
    std::cout << swcObj->toString() << std::endl;

    Poco::MongoDB::Array::Ptr swcList(new Poco::MongoDB::Array());
	swcList->add(std::to_string(0), swcObj);

    auto insert = g_db.createCommand();
    insert->selector().add("insert",tableName).add("documents",swcList);
    Poco::MongoDB::ResponseMessage response;

    c->sendRequest(*insert,response);
    auto doc = *(response.documents()[0]);
    std::cout << doc.toString() << std::endl;

    if( doc.getInteger("ok") == 1 ) return true;
    return false;
}

bool DataBase::findSWC(const NeuronSWC &swc, const std::string &tableName){
    std::cout << "[findSWC] IN TABLE " << tableName << std::endl;
    // take connection from pool
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto queryPtr = g_db.createQueryRequest(tableName);
    queryPtr->selector().add("_id", swc.id);

    // limit return numbers
    queryPtr->setNumberToReturn(1);
    Poco::MongoDB::ResponseMessage response;

    // send request to server
    c->sendRequest(*queryPtr, response);
    if (response.documents().empty()) {
        return false;
    }
    std::cout << "[findSWC] FIND:";
    auto doc = *(response.documents()[0]);
    std::cout << doc.toString() << std::endl;
    return true;
}

bool DataBase::insertSWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName){
    Poco::MongoDB::Array::Ptr swcList(new Poco::MongoDB::Array());
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);
    int index = 0;
    for( auto swc = swcs.begin() ; swc != swcs.end() ; swc ++ ){
        if( findSWC(**swc,tableName) ) return false;

        Poco::MongoDB::Document::Ptr swcObj(new Poco::MongoDB::Document());
        swcObj->add("_id",(*swc)->id);
        swcObj->add("name",(*swc)->name);
        swcObj->add("line_id",(*swc)->line_id);
        swcObj->add("x",(*swc)->x);
        swcObj->add("y",(*swc)->y);
        swcObj->add("z",(*swc)->z);
        swcObj->add("parent",(*swc)->pn);
        swcObj->add("color",(*swc)->color);
        swcObj->add("user_id",(*swc)->user_id);
        swcObj->add("timestamp",(*swc)->timestamp);
        swcObj->add("seg_id",(*swc)->seg_id);
        swcObj->add("seg_in_id",(*swc)->seg_in_id);
        swcObj->add("seg_size",(*swc)->seg_size);
        swcObj->add("type",int((*swc)->type));
        swcObj->add("radius",(*swc)->radius);
        swcList->add(std::to_string(index++),swcObj);
    }
    std::cout << "[insertSWCs] INSERT:";
    std::cout << swcList->toString() << std::endl;
    auto insert = g_db.createCommand();

    insert->selector().add("insert",tableName).add("documents",swcList);

    Poco::MongoDB::ResponseMessage response;

    c->sendRequest(*insert,response);
    auto doc = *(response.documents()[0]);

    if( doc.getInteger("ok") == 1 ) return true;
    return false;
}

bool DataBase::deleteSWC(const NeuronSWC &swc, const std::string &tableName){
    std::cout << "[deleteSWC]" << std::endl;
    // take connection
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    // create query fro finding book
    Poco::MongoDB::Document::Ptr query(new Poco::MongoDB::Document());
    query->add("_id", swc.id);

    Poco::MongoDB::Document::Ptr del(new Poco::MongoDB::Document());
    del->add("q", query).add("limit", 1);

    Poco::MongoDB::Array::Ptr deletes(new Poco::MongoDB::Array());
    deletes->add(std::to_string(0), del);

    // create command
    auto deleteCmd = g_db.createCommand();
    deleteCmd->selector()
    .add("delete", tableName)
    .add("deletes", deletes);

    Poco::MongoDB::ResponseMessage response;
    c->sendRequest(*deleteCmd, response);
    auto doc = *(response.documents()[0]);

    if( doc.getInteger("ok") == 1 ) return true;
    return false;

    std::cout << doc.toString() << std::endl;    
    // for (auto i : response.documents()) {
    //     return i->toString(2);
    // }
    // return true;
}

bool DataBase::deleteSWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName){
    // take connection
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);
    int index = 0;
    Poco::MongoDB::Array::Ptr deletes(new Poco::MongoDB::Array());
    for ( auto swc = swcs.begin() ; swc != swcs.end() ; swc ++ ){
        // create query fro finding book
        Poco::MongoDB::Document::Ptr query(new Poco::MongoDB::Document());
        query->add("_id", (*swc)->id);

        Poco::MongoDB::Document::Ptr del(new Poco::MongoDB::Document());
        del->add("q", query).add("limit", 1);
        deletes->add(std::to_string(index++), del);
    }
    // create command
    auto deleteCmd = g_db.createCommand();
    deleteCmd->selector()
    .add("delete", tableName)
    .add("deletes", deletes);

    Poco::MongoDB::ResponseMessage response;
    c->sendRequest(*deleteCmd, response);
    auto doc = *(response.documents()[0]);

    if( doc.getInteger("ok") == 1 ) return true;
    return false;
}

std::string DataBase::getSWCFileStringFromTable(const std::string &tableName){
    // take connection from pool
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto queryPtr = g_db.createQueryRequest(tableName);
    queryPtr->selector();

    Poco::MongoDB::ResponseMessage response;

    // send request to server
    c->sendRequest(*queryPtr, response);
    if (response.documents().empty()) {
        return "";
    }
    // else return true;
    std::stringstream sstream;
    for( int i = 0 ; i < response.documents().size() ; i ++ ){
        char buff[MAX_LINE_SIZE];
        sprintf(buff, " %lld %d %.15g %.15g %.15g %.7g %lld #name:%s color:%s line_id:%d seg_id:%d seg_size:%d seg_in_id:%d user_id:%d timestamp:%lld\n",
                stoll(response.documents()[i]->get("_id")->toString()),
                stoi(response.documents()[i]->get("type")->toString()),
                stod(response.documents()[i]->get("x")->toString()),
                stod(response.documents()[i]->get("y")->toString()),
                stod(response.documents()[i]->get("z")->toString()),
                stod(response.documents()[i]->get("radius")->toString()),
                stoll(response.documents()[i]->get("parent")->toString()),
                response.documents()[i]->get("name")->toString().c_str(),
                response.documents()[i]->get("color")->toString().c_str(),
                stoi(response.documents()[i]->get("line_id")->toString()),
                stoi(response.documents()[i]->get("seg_id")->toString()),
                stoi(response.documents()[i]->get("seg_size")->toString()),
                stoi(response.documents()[i]->get("seg_in_id")->toString()),
                stoi(response.documents()[i]->get("user_id")->toString()),
                stoll(response.documents()[i]->get("timestamp")->toString()));
        sstream << buff;
    }

    return sstream.str();
}

bool DataBase::findTable(const std::string &tableName){
    // take connection from pool
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto queryPtr = g_db.createQueryRequest(tableName);
    queryPtr->selector();

    // limit return numbers
    queryPtr->setNumberToReturn(1);
    Poco::MongoDB::ResponseMessage response;

    // send request to server
    c->sendRequest(*queryPtr, response);
    if (response.documents().empty()) {
        return false;
    }
    else return true;
}

std::string DataBase::showAllTables(){
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto deleteCmd = g_db.createCommand();
    deleteCmd->selector().add("listCollections","1.0").add("nameOnly","true");

    Poco::MongoDB::ResponseMessage response;
    c->sendRequest(*deleteCmd, response);
    auto doc = *(response.documents()[0]);
    return doc.toString();
}

Poco::MongoDB::PooledConnection DataBase::takeConnection()
{
	static std::mutex connectionPoolLock;
	std::lock_guard<std::mutex> l(connectionPoolLock);

	Poco::MongoDB::PooledConnection pooledConnection(*g_connectionPool);
	auto c = static_cast<Poco::MongoDB::Connection::Ptr>(pooledConnection);

	if (!c) {
		// Connection pool can return null if the pool is full
		// TODO: Gracefully handle this here or implement
		// ObjectPool::borrowObjectWithTimeout
	}
	return std::move(pooledConnection);
}
