//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/component.hpp>

namespace wfc{

/**
 * \brief Синглетон логгера.
 * \ingroup logger_module
 * 
 * Управляет объектом logger
 * 
 * \see logger
 */
class logger_singleton
  : public ::wfc::component
{
public:
  logger_singleton();
};

}
