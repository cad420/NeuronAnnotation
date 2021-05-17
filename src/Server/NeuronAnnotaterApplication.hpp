#pragma once
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/OptionSet.h>

class NeuronAnnotaterApplication : public Poco::Util::ServerApplication {
protected:
  void initialize(Application &self) override;

  void defineOptions(Poco::Util::OptionSet &options) override;

  int main(const std::vector<std::string> &args) override;

  void hanldle_option(const std::string &name, const std::string &value);

private:
  bool m_show_help = false;
  uint32_t m_port = 12121;
};
