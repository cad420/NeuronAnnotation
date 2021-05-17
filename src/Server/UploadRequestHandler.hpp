#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <AnnotationDS.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Mutex.h>
#include "WebSocketRequestHandler.hpp"

class UploadRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  int user_id;
  NeuronPool *neuron_pool;
  WebSocketRequestHandler *render_ws;
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;
};

