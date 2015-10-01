//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/module.hpp>

namespace wfc{
  
/**
 * \defgroup logger_module Модуль логгирования. 
 * \ingroup logger_package 
 */

/**
 * \brief Включает компоненты модуля логгировния. 
 * \ingroup logger_package
 * 
 * Состоит из одного синглетона logger_singleton
 * 
 * \see logger_singleton
 */
class logger_module
  : public ::wfc::module
{
  class impl;
public:
  logger_module();
};

}

