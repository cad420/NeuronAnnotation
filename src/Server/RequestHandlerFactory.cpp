#include "RequestHandlerFactory.hpp"
#include "MyHTTPRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include "UploadRequestHandler.hpp"
#include <AnnotationDS.hpp>
#include <SWCP.hpp>
#include <iostream>
#include <VolumeRenderer.hpp>
#include <TransferFunction.hpp>
#include "DownloadRequestHandler.hpp"


std::shared_ptr<Poco::Mutex> RequestHandlerFactory::volume_render_lock =  make_shared<Poco::Mutex>();
int RequestHandlerFactory::max_linked_id = 0;
map<string,std::shared_ptr<NeuronGraph>> RequestHandlerFactory::neuronGraphs;
map<int,NeuronPool*> RequestHandlerFactory::neuronPools;
map<string,int> RequestHandlerFactory::userList;
std::shared_ptr<VolumeRenderer> RequestHandlerFactory::block_volume_renderer;
std::shared_ptr<VolumeRenderer> RequestHandlerFactory::lines_renderer;
bool RequestHandlerFactory::isInited = false;
std::map<int, WebSocketRequestHandler*> RequestHandlerFactory::userWebsocketRequestHandler;


void RequestHandlerFactory::initBlockVolumeRender(){
    std::cout<<"loading render backend..."<<std::endl;
    block_volume_renderer = make_shared<VolumeRenderer>("BlockVolumeRenderer");
#ifdef _WINDOWS
    block_volume_renderer->set_volume("D:/mouse_23389_29581_10296_9p2_lod3.h264");
#else
    block_volume_renderer->set_volume("/media/wyz/Workspace/mouse_23389_29581_10296_512_2_lod3/mouse_23389_29581_10296_9p2_lod3.h264");
#endif
    TransferFunction default_tf;
    default_tf.points.emplace_back(0);
    default_tf.colors.emplace_back(std::array<double ,4>{0.0,0.1,0.6,0.0});
    default_tf.points.emplace_back(30);
    default_tf.colors.emplace_back(std::array<double ,4>{0.25, 0.5, 1.0, 0.9});
    default_tf.points.emplace_back(64);
    default_tf.colors.emplace_back(std::array<double ,4>{0.75,0.75,0.75,0.9});
    default_tf.points.emplace_back(224);
    default_tf.colors.emplace_back(std::array<double ,4>{1.0,0.5,0.25,0.9});
    default_tf.points.emplace_back(225);
    default_tf.colors.emplace_back(std::array<double ,4>{0.6,0.1,0.0,1.0});
    block_volume_renderer->set_transferfunc(default_tf);
}

void RequestHandlerFactory::initLinesRender(){
    std::cout<<"loading lines render backend..."<<std::endl;
    lines_renderer = make_shared<VolumeRenderer>("LinesRenderer");
}

Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) {
    if( !isInited ){
        //neuronGraphs["test"] = make_shared<NeuronGraph>("./test.swc","test"); //读取本地swc,并且转换成test集合
        neuronGraphs["test"] = make_shared<NeuronGraph>("test",0);
        initBlockVolumeRender();
        initLinesRender();
        isInited = true;
    }
    auto &uri = request.getURI();
    auto address =  request.clientAddress();
    auto host = address.host();
    std::cout << "Who are you?? I am" <<uri << std::endl;

    if (uri == "/render") {
        std::cout << "create WebSocketRequestHandler" << std::endl;
        WebSocketRequestHandler *n = new WebSocketRequestHandler();
        n->block_volume_renderer = block_volume_renderer;
        n->volume_render_lock = volume_render_lock;        
        if( userList.find(host.toString()) != userList.end() ){ //已有
            n->user_id = userList[host.toString()];
            n->neuron_pool = neuronPools[n->user_id];
            userWebsocketRequestHandler[n->user_id] = n; //绑定每个用户的WebSocket
        }else{
            n->user_id = ++max_linked_id;
            userList[host.toString()] = n->user_id;
            n->neuron_pool = new NeuronPool();
            n->neuron_pool->setGraph(neuronGraphs["test"]);
            n->neuron_pool->setGraphPool(&neuronGraphs);
            n->neuron_pool->setUserId(n->user_id);
            neuronPools[n->user_id] = n->neuron_pool;
            userWebsocketRequestHandler[n->user_id] = n; //绑定每个用户的WebSocket
        }
        return n;
    }

    if (uri == "/info"){
        std::cout << "create MyHTTPRequestHandler" << std::endl;
        MyHTTPRequestHandler *n = new MyHTTPRequestHandler();
        if( userList.find(host.toString()) != userList.end() ){ //已有
            n->user_id = userList[host.toString()];
            n->neuron_pool = neuronPools[n->user_id];
        }else{
            n->user_id = ++max_linked_id;
            userList[host.toString()] = n->user_id;
            n->neuron_pool = new NeuronPool();
            n->neuron_pool->setGraph(neuronGraphs["test"]);
            n->neuron_pool->setGraphPool(&neuronGraphs);
            n->neuron_pool->setUserId(n->user_id);
            neuronPools[n->user_id] = n->neuron_pool;
        }
        n->render_ws = userWebsocketRequestHandler[n->user_id];
        return n;
    }

    if (uri == "/upload"){
        std::cout << "create UploadRequestHandler" << std::endl;
        UploadRequestHandler *n = new UploadRequestHandler();
        if( userList.find(host.toString()) != userList.end() ){ //已有
            n->user_id = userList[host.toString()];
            n->neuron_pool = neuronPools[n->user_id];
        }else{
            n->user_id = ++max_linked_id;
            userList[host.toString()] = n->user_id;
            n->neuron_pool = new NeuronPool();
            n->neuron_pool->setGraph(neuronGraphs["test"]);
            n->neuron_pool->setGraphPool(&neuronGraphs);
            n->neuron_pool->setUserId(n->user_id);
            neuronPools[n->user_id] = n->neuron_pool;
        }
        n->render_ws = userWebsocketRequestHandler[n->user_id];
        return n;
    }

    if (uri == "/download"){
        std::cout << "create DownloadRequestHandler" << std::endl;
        DownloadRequestHandler *n = new DownloadRequestHandler();
        if( userList.find(host.toString()) != userList.end() ){ //已有
            n->user_id = userList[host.toString()];
            n->neuron_pool = neuronPools[n->user_id];
        }else{
            n->user_id = ++max_linked_id;
            userList[host.toString()] = n->user_id;
            n->neuron_pool = new NeuronPool();
            n->neuron_pool->setGraph(neuronGraphs["test"]);
            n->neuron_pool->setGraphPool(&neuronGraphs);
            n->neuron_pool->setUserId(n->user_id);
            neuronPools[n->user_id] = n->neuron_pool;
        }
        return n;
    }
}
