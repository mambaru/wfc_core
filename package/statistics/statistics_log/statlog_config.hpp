//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>
#include <map>

namespace wfc{ namespace core{


struct table_format_options
{
  typedef std::map< std::string, int > width_map_t;
  typedef std::map<std::string, std::string> field_aliases_t;
  typedef std::vector<std::string> sequence_of_list_t;

  std::string table_log;

  bool show_legend = true;

  std::string    sep = "|";

  int         default_field_width = 0;

  field_aliases_t aliases;

  // Определяет порядок вывода столбцов в таблице.
  // Неуказанные имена переносятся в конец в порядке по умолчанию
  // Имя "#" указывает на то, что поля не указанные в списке отключаются
  // (лучше ставить в конце списка, но не зависит от позиции. Интерпретируется как флаг )
  // $ - Поле по дефолтному порядку без изменений
  sequence_of_list_t sequence_of_list;

  // Ключ имя или псевдоним
  // -1 = отключено
  // 0  = field_width
  // N  = ширина поля
  // Если в order_list указан "#" то любое поле в списке width_map с шириной не равной -1, принудительно его включает
  // Аналогично, значение -1, отключает поле, независимо указано оно в order_list или нет
  width_map_t width_map;

};

struct statlog_config
{
  std::string target;
  std::string common_log;
  int log_metric = -1;
  table_format_options table_format;
};

}}
