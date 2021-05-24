#include "MyHTTPRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <Camera.hpp>
#include <Image.hpp>
#include <TransferFunction.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Util/Application.h>
#include <seria/deserialize.hpp>
#include <iostream>
#include <AnnotationDS.hpp>
#include <ErrorMessage.hpp>
#include <SWCP.hpp>
#include <DataBase.hpp>
#include <iostream>
#include <Poco/InflatingStream.h>
#include <Poco/Net/HTMLForm.h>
using Poco::Util::Application;
using WebSocket = Poco::Net::WebSocket;

void MyHTTPRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  

    Application &app = Application::instance();
    
    if( neuron_pool->getSelectedLineIndex() == -1 ){
        neuron_pool->initSelectedLineIndex();
    }
    if( neuron_pool->getSelectedVertexIndex() == -1 ){
        neuron_pool->initSelectedVertexIndex();
    }

    try
    {
        char buffer[4096];
        int flags = 0;
        int len;

        WebSocket ws(request, response);
        auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
        ws.setReceiveTimeout(one_hour);
        rapidjson::Document document{};

        do{
            len=ws.receiveFrame(buffer,sizeof(buffer),flags);
            try
            {   
                document.Parse(buffer,len);
                
                if(document.HasParseError() || !document.IsObject())
                {
                    throw std::runtime_error("Parse error");
                }
                auto objects=document.GetObject();
                if(document.HasMember("modify")){
                    rapidjson::Value &modify_data = document["modify"];
                    int line_id = -1;
                    bool result = true;
                    bool select = false;
                    if( modify_data.HasMember("index") && modify_data["index"].IsInt64() ){
                        line_id = modify_data["index"].GetInt64();
                    }
                    if( modify_data.HasMember("selectedVertexIndex") && modify_data["selectedVertexIndex"].IsInt64() ){
                        neuron_pool->selectVertex(modify_data["selectedVertexIndex"].GetInt64());
                        render_ws->sendIamgeFrame();
                    }
                    if( modify_data.HasMember("selectedLineIndex") && modify_data["selectedLineIndex"].IsInt64() ){
                        neuron_pool->selectLine(modify_data["selectedLineIndex"].GetInt64());
                    }
                    if( modify_data.HasMember("name") && modify_data["name"].IsString() ){
                        result &= neuron_pool->changeName(line_id,modify_data["name"].GetString());
                    }
                    if( modify_data.HasMember("color") && modify_data["color"].IsString() ){
                        result &= neuron_pool->changeColor(line_id,modify_data["color"].GetString());
                        render_ws->sendIamgeFrame();
                    }
                    if( modify_data.HasMember("visible") ){
                        result &= neuron_pool->changeVisible(line_id,modify_data["visible"].GetBool());
                        render_ws->sendIamgeFrame();
                    }
                    if( modify_data.HasMember("selectedTableName") ){
                        result &= neuron_pool->changeTable(modify_data["selectedTableName"].GetString());
                        render_ws->sendIamgeFrame();
                    }
                    if( modify_data.HasMember("selectedRender" ) ){
                        result &= neuron_pool->changeMode(modify_data["selectedRender"].GetString());
                        render_ws->sendIamgeFrame();
                    }
                    if( modify_data.HasMember("selectedTool" ) ){
                        neuron_pool->setTool(modify_data["selectedTool"].GetInt());
                    }
                    if( result ){
                        render_ws->sendSuccessFrame("修改成功");
                    }else{
                        render_ws->sendErrorFrame("修改失败");
                    }
                }
                else if(document.HasMember("addline")){
                    if (neuron_pool->addLine()){
                        render_ws->sendSuccessFrame("添加成功");
                    }else{
                        render_ws->sendErrorFrame("添加失败");
                    }
                }
                else if(document.HasMember("deleteline")){
                    int line_id = -1;
                    if( document.HasMember("index") && document["index"].IsInt64() ){
                        line_id = document["index"].GetInt64();
                    }
                    if (neuron_pool->deleteLine(line_id)){
                        render_ws->sendSuccessFrame("删除成功");
                        render_ws->sendIamgeFrame();
                    }else{
                        render_ws->sendErrorFrame("删除失败");
                    }
                }
                render_ws->sendStructureFrame();
                ws.close();
                return;
            }
            catch (std::exception& error)
            {
                ws.sendFrame(error.what(), std::strlen(error.what()),WebSocket::FRAME_TEXT);
            }
        }while(len>0 && (flags & WebSocket::FRAME_OP_BITMASK) !=WebSocket::FRAME_OP_CLOSE);
    }//try
    catch (Poco::Net::WebSocketException &exc)
    {

        app.logger().log(exc);
        switch (exc.code())
        {
            case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
                // fallthrough
            case WebSocket::WS_ERR_NO_HANDSHAKE:
            case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
            case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                break;
        }//switch
    }//catch
}
