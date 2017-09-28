
#pragma once

namespace wfc{ namespace core{

struct statlog_config
{
  std::string target;
  std::string common_log;
  std::string legend_log;
  std::string table_log;
  int log_metric = 1;
};

}}
