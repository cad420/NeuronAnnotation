#include "UploadRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <Camera.hpp>
#include <Image.hpp>
#include <TransferFunction.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Util/Application.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/PartHandler.h>
#include<Poco/Net/MessageHeader.h>
#include<Poco/Net/HTMLForm.h>
#include <seria/deserialize.hpp>
#include <iostream>
#include <AnnotationDS.hpp>
#include <ErrorMessage.hpp>
#include <SWCP.hpp>
#include <DataBase.hpp>
#include <iostream>
#include "RequestHandlerFactory.hpp"

using Poco::Util::Application;

class MyPartHandler : public Poco::Net::PartHandler{
public:
    void handlePart(const Poco::Net::MessageHeader &header,std::istream &stream){
        if (header.has("Content-Disposition")){
            std::string disp;
            Poco::Net::NameValueCollection params;
            Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
            _disp = header["Content-Disposition"];
            _type = header["Content-Type"];
            _name = params.get("name", "(unnamed)");
            _filename = params.get("filename", "(unnamed)");
            Poco::StreamCopier::copyToString(stream, _buffer);
            _length = _buffer.size();
        }
    }
    string buffer(){
        return _buffer;
    }
    string filename(){
        return _filename;
    }
    string name(){
        return _name;
    }
    int length(){
        return _length;
    }
private:
    int _length;
    std::string _disp;
    std::string _type;
    std::string _name;
    std::string _filename;
    std::string _buffer;
};

void UploadRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  
    Application &app = Application::instance();
    std::cout << request.getMethod() << endl;
    response.add("Access-Control-Allow-Origin", "*");
    response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE");
    response.add("Access-Control-Max-Age", "3600");
    response.add("Access-Control-Allow-Headers", "x-requested-with,authorization");
    if (request.getMethod() == "OPTIONS")
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.send();
        return;
    }
    int len = request.getContentLength();
    std::istream &i = request.stream();
    MyPartHandler partHandler;
    Poco::Net::HTMLForm form(request, request.stream(), partHandler);
    std::cout << "buffer:" << partHandler.buffer() << endl;
    std::cout << "file name:" << partHandler.filename() << endl;
    std::cout << "file size:" << partHandler.length() << endl;
    
    
    string tableName = partHandler.filename();
    string fix = ".swc";
    int pos = tableName.find(fix);
    tableName = tableName.erase(pos,fix.size());
    if( RequestHandlerFactory::neuronGraphs.find(tableName) != RequestHandlerFactory::neuronGraphs.end() ){
        response.setStatus(Poco::Net::HTTPResponse::HTTP_CONFLICT);
        response.send();
    }
    else{
        string filePath = "./" + tableName;
        std::ofstream of(filePath);
        of.clear();
        of.write(partHandler.buffer().c_str(),partHandler.buffer().size());
        of.close();
        RequestHandlerFactory::neuronGraphs[tableName] = std::make_shared<NeuronGraph>(filePath.c_str(),tableName.c_str());
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.send();
        render_ws->sendSuccessFrame("上传成功");
        render_ws->sendStructureFrame();
    }
    return;



    // try
    // {

        // char buffer[4096];
        // int flags = 0;

        // std::istream &i = request.stream();
        // int len = request.getContentLength();
        // i.read(buffer, len);
        
        // std::cout <<request.getContentType() << std::endl;
        // std::cout << buffer << std::endl;
    //     auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
    //     ws.setReceiveTimeout(one_hour);
    //     rapidjson::Document document{};

    //     do{
    //         len=ws.receiveFrame(buffer,sizeof(buffer),flags);
    //         try
    //         {   
    //             std::cout << std::string(buffer,len) << std::endl;
    //             document.Parse(buffer,len);
    //             if(document.HasParseError() || !document.IsObject())
    //             {
    //                 throw std::runtime_error("Parse error");
    //             }
    //             auto objects=document.GetObject();
    //             std::string structureInfo = neuron_pool->getLinestoJson();
    //             ws.sendFrame(structureInfo.c_str(),structureInfo.size(),WebSocket::FRAME_TEXT);
    //         }
    //         catch (std::exception& error)
    //         {
    //             ws.sendFrame(error.what(), std::strlen(error.what()),WebSocket::FRAME_TEXT);
    //         }
    //     }while(len>0 && (flags & WebSocket::FRAME_OP_BITMASK) !=WebSocket::FRAME_OP_CLOSE);
    // }//try
    // catch (Poco::Net::WebSocketException &exc)
    // {

    //     app.logger().log(exc);
    //     switch (exc.code())
    //     {
    //         case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
    //             response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
    //             // fallthrough
    //         case WebSocket::WS_ERR_NO_HANDSHAKE:
    //         case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
    //         case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
    //             response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    //             response.setContentLength(0);
    //             response.send();
    //             break;
    //     }//switch
    // }//catch
}
