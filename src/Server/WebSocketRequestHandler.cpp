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
#include <AutoPathFind.hpp>
using Poco::Util::Application;

void WebSocketRequestHandler::handleRequest(
        Poco::Net::HTTPServerRequest &request,
        Poco::Net::HTTPServerResponse &response) {

    using WebSocket = Poco::Net::WebSocket;
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

        ws = new WebSocket(request, response);
        
        auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
        ws->setReceiveTimeout(one_hour);
        rapidjson::Document document{};

        do{
            len=ws->receiveFrame(buffer,sizeof(buffer),flags);
            try
            {   
                document.Parse(buffer,len);
                
                if(document.HasParseError() || !document.IsObject())
                {
                    throw std::runtime_error("Parse error");
                }
                auto objects=document.GetObject();
                if(document.HasMember("camera"))
                {
                    auto values=objects["camera"].GetObject();
                    Camera camera;
                    seria::deserialize(camera,values);
                    neuron_pool->setCamera(camera);
                    sendIamgeFrame();
                }
                else if(document.HasMember("click"))
                {
                    auto values=objects["click"].GetObject();
                    QueryPoint query_point;
                    seria::deserialize(query_point,values);
                    //进一步确定当前操作
                    switch(Tools(neuron_pool->getTool())){
                        case Drag:
                            {
                                long id = neuron_pool->selectVertex(query_point.x,query_point.y);
                                std::stringstream  fmt;
                                fmt << "switch to " << id ;
                                if( id != -1 ){
                                    string st =  fmt.str();
                                    sendSuccessFrame(st);
                                    sendIamgeFrame();
                                }
                                break;
                            }

                        case Insert:
                        {
                            volume_render_lock->lock();
                            
                                block_volume_renderer->enter_gl();

                                block_volume_renderer->set_camera(neuron_pool->getCamera());
                                block_volume_renderer->set_mode(neuron_pool->getRenderMode());
                                block_volume_renderer->set_querypoint({query_point.x,query_point.y});
                                
                                block_volume_renderer->render_frame();
                                auto query_res = block_volume_renderer->get_querypoint();
                                auto maptable = block_volume_renderer->get_pos_frame();
                            
                                block_volume_renderer->exit_gl();

                            volume_render_lock->unlock();

                            if( query_res[7] > 0.1f ){
                                if(neuron_pool->getSelectedVertexIndex() == -1 ){
                                // if( true ){
                                    if(neuron_pool->addVertex(query_res[0],query_res[1],query_res[2])){
                                        sendSuccessFrame("添加成功");
                                        sendIamgeFrame();
                                    }else{
                                        sendErrorFrame("添加失败");
                                    }
                                }
                                else{
                                    AutoPathGen *ag = new AutoPathGen();
                                    std::array<int,2> start = neuron_pool->getSelectedVertexXY();
                                    ag->point1 = {(unsigned int)start[1],(unsigned int)start[0]}; //获取起点的坐标
                                    ag->point0 = {query_point.y,query_point.x}; //终点坐标
                                    auto path = ag->GenPath_v1(maptable);

                                    if(neuron_pool->addSegment(&path)){ //涉及到点的添加
                                        sendSuccessFrame("添加成功");
                                        sendIamgeFrame();
                                    }else{
                                        sendErrorFrame("添加失败");
                                    }
                                }
                            }else{
                                printf("%lf Alpha is too low!\n",query_res[7]);
                                sendErrorFrame("选择点透明度低，请重新选择");
                            }
                        break;
                        }
                        case Cut:
                            if(neuron_pool->dividedInto2Lines(query_point.x,query_point.y)){
                                sendSuccessFrame("切分成功");
                                sendIamgeFrame();
                            }else{
                                sendErrorFrame("切分失败");
                            }
                        break;
                        case Select:
                            //矩形框选中 TODO
                        break;
                        case Delete:                            //找到最近的一个叶节点
                                                                //删除这个叶节点相连的线
                            string error;
                            bool res = neuron_pool->deleteVertex(query_point.x,query_point.y,error);
                            if( res ){
                                sendSuccessFrame("删除成功");
                                sendIamgeFrame();
                            }else{
                                sendErrorFrame(error);
                            }

                        break;
                    }
                }
                sendStructureFrame();
            }
            catch (std::exception& error)
            {
                ws->sendFrame(error.what(), std::strlen(error.what()),WebSocket::FRAME_TEXT);
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

void WebSocketRequestHandler::sendErrorFrame(std::string errorMessage){
    using WebSocket = Poco::Net::WebSocket;
    ErrorMessage em(errorMessage);
    std::string str = em.ToJson();
    ws->sendFrame(str.c_str(),str.size(),WebSocket::FRAME_TEXT);
}

void WebSocketRequestHandler::sendSuccessFrame( std::string successMessage){
    using WebSocket = Poco::Net::WebSocket;
    ErrorMessage em(successMessage,"success");
    std::string str = em.ToJson();
    ws->sendFrame(str.c_str(),str.size(),WebSocket::FRAME_TEXT);
}

void WebSocketRequestHandler::sendIamgeFrame(){
    using WebSocket = Poco::Net::WebSocket;
    volume_render_lock->lock();
    
        block_volume_renderer->enter_gl(); //线程进入gl时必须调用

        block_volume_renderer->set_mode(neuron_pool->getRenderMode());
        block_volume_renderer->set_neuronpool(neuron_pool);
        block_volume_renderer->set_camera(neuron_pool->getCamera());
        block_volume_renderer->render_frame();
        auto &image = block_volume_renderer->get_frame();

        block_volume_renderer->exit_gl(); //线程离开gl时必须调用
    
    volume_render_lock->unlock();

    std::cout<<image.width<<" "<<image.height<<std::endl;
    auto encoded = Image::encode(image, Image::Format::JPEG);
    ws->sendFrame(encoded.data.data(), encoded.data.size(),WebSocket::FRAME_BINARY);

}

void WebSocketRequestHandler::sendStructureFrame(){
    using WebSocket = Poco::Net::WebSocket;
    std::string structureInfo = neuron_pool->getLinestoJson();
    ws->sendFrame(structureInfo.c_str(),structureInfo.size(),WebSocket::FRAME_TEXT);
}