#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <AnnotationDS.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Mutex.h>
#include "WebSocketRequestHandler.hpp"

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
  static int max_linked_id;
  static map<string,std::shared_ptr<NeuronGraph>> neuronGraphs;
  static map<string,int> userList;
  static map<int,NeuronPool*> neuronPools;
  static std::shared_ptr<VolumeRenderer> block_volume_renderer;
  static std::shared_ptr<VolumeRenderer> lines_renderer;
  static std::map<int, WebSocketRequestHandler* > userWebsocketRequestHandler;
  static bool isInited;
  static std::shared_ptr<Poco::Mutex> volume_render_lock;
public:
  Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
  void initBlockVolumeRender();
  void initLinesRender();
};
