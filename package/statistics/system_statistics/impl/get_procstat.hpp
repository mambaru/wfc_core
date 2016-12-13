#pragma once

#include "procstat.hpp"
#include "unistd.h"

namespace wfc{ namespace core{

int get_procstat(procstat* pinfo);
int get_procstat(pid_t pid, procstat* pinfo);


}}