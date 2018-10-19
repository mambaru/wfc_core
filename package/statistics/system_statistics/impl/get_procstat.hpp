//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "procstat.hpp"
#include "unistd.h"

namespace wfc{ namespace core{

int get_procstat(procstat* pinfo);
int get_procstat(pid_t pid, procstat* pinfo);


}}
