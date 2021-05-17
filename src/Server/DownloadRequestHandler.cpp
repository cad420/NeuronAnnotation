#include "DownloadRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <Camera.hpp>
#include <Image.hpp>
#include <TransferFunction.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Exception.h>
#include <Poco/Util/Application.h>
#include <seria/deserialize.hpp>
#include <iostream>
#include <AnnotationDS.hpp>
#include <ErrorMessage.hpp>
#include <SWCP.hpp>
#include <DataBase.hpp>
#include <iostream>
using Poco::Util::Application;
void DownloadRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  
    char buffer[4096];
    int flags = 0;
    int len;
    rapidjson::Document document{};

    //std::ostream &responseStream = response.send();
    std::shared_ptr<NeuronGraph> pG = neuron_pool->getGraph();
    std::string output = DataBase::getSWCFileStringFromTable(pG->tableName);
    std::string filePath = "./";
    std::string fileName = "ccecece";//pG->tableName;

    filePath.append(fileName);
    fileName.append(".swc");
    filePath.append(".swc");

    std::ofstream o(filePath);
    o.write(output.c_str(),output.size());
    o.close();
    response.set("Content-Disposition","attachment");
    response.set("filename",fileName);
    response.sendFile(filePath,"text/plain"); //文件格式可能有问题，现在下载是download.txt
}
