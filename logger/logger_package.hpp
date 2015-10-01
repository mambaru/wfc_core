//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/package.hpp>

namespace wfc{

/**
 * \defgroup logger_package Пакет модулей логгирования
 * \ingroup wfcroot
 */

/**
 * \brief Включает модули логгировния 
 * \ingroup wfcroot
 * 
 * Состоит из одного одного модуля logger_module
 * 
 * \see logger_module
 */
class logger_package
  : public ::wfc::package
{
  class impl;
public:
  logger_package();
};

}
