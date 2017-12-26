# Модуль логирования 

Управляет системой логированя WFC. 

```bash
2017-12-19 18:37:03.049 CONFIG BEGIN Configure component 'server-tcp'...
2017-12-19 18:37:03.049 CONFIG MESSAGE Instance 'server-tcp1' is initial configured
2017-12-19 18:37:03.049 CONFIG END Configure component 'server-tcp'...Done
```
Каждая строка лога систоит:

* Дата 
* Время + доли секунды
* Имя лога:
  * SYSTEM - конфигурирование и инициализация системы
  * DOMIAN - прикладной лог (важные сообщения)
  * COMMON - общий лог (все остальное)
  * SYSLOG - системый лог плюс запись в syslog (критические сообщения)
  * DEBUG  - лог отладки (отключаеться в релиз-версии на уровне компилятора )
  * JSONRPC - лог jsonrpc движка
  * IOW     - лог сетевого движка
* Тип сообщения:
  * ERROR   - ошибки, после которых система сохраняет работоспособность и консистентность 
  * WARNING - предупреждения, например о превышении размера очереди. 
  * MESSAGE - прочие уведомления
  * FATAL   - фатальная ошибка. Обычно далее следует останов демона 
  * BEGIN   - начало какого либо процесса (например загрузка БД)
  * END     - окончание какого либо процесса (например загрузка БД)
  * DEBUG   - отладочные сообщения (отключаеться в релиз-версии на уровне компилятора )
  * TRACE   - трассировка ввода/вывода (отключаеться в релиз-версии на уровне компилятора )
  * PROGRESS - трассировка прогресса длительных операций без перевода строки (например процент загрузки БД)
* Сообщение. Произвольный формат

### Использование
```cpp
#include <wfc/logger.hpp>
```
Доступнен следующий набор макросов :
```
Имя_LOG_Тип("Это " << 10 << " сообщение" )
```
Например:
```cpp
COMMON_LOG_MESSAGE("Это " << 10 << " сообщение" )
```
В прикладных модулях рекомендутся использовать DOMIAN, COMMON и DEBUG или определить свой набор макросов, например так
```cpp
#define DEMO_LOG_ERROR(X)    WFC_LOG_ERROR( "DEMO", X )
#define DEMO_LOG_WARNING(X)  WFC_LOG_WARNING( "DEMO", X )
#define DEMO_LOG_MESSAGE(X)  WFC_LOG_MESSAGE( "DEMO", X )
#define DEMO_LOG_FATAL(X)    WFC_LOG_FATAL( "DEMO", X )
#define DEMO_LOG_BEGIN(X)    WFC_LOG_BEGIN( "DEMO", X )
#define DEMO_LOG_END(X)      WFC_LOG_END( "DEMO", X )
#define DEMO_LOG_DEBUG(X)    WFC_LOG_DEBUG( "DEMO", X )
#define DEMO_LOG_TRACE(X)    WFC_LOG_TRACE( "DEMO", X )
#define DEMO_LOG_PROGRESS(X) WFC_LOG_PROGRESS( "DEMO", X )
```
Все макросы автоматически добавляют перевод строки после сообщения, за исключением ```*_PROGRESS``` который использует возврат каретки.
Следующий набор логов отключаются в Release версии препроцессором:
```
DEBUG_LOG_*
*_LOG_DEBUG
*_LOG_TRACE
```
При любых ```*_FATAL``` система корректно пытаеться завершить работу (можно отлючить работу). Т.к. процедура может быть длительной и 
сопровождаться большим количеством записей в лог, последним сообщением, система логгирование дублирует это сообщение, следующим образом:
```bash
2017-12-21 17:15:17 FINAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
2017-12-21 17:15:17 FINAL ---------- Abnormal Shutdown! ----------
2017-12-21 17:15:17 FINAL Date: 2017-12-21
2017-12-21 17:15:17 FINAL Time: 17:15:17
2017-12-21 17:15:17 FINAL Name: COMMON
2017-12-21 17:15:17 FINAL Message: Сообщение об ошибке, вызвавшее останов системы
2017-12-21 17:15:17 FINAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
```
## Настройки 
Значения по умолчанию:
```cpp
{
  "logger": {
    "enabled": true,
    /* Рекоменуемое значение -1000 */
    "startup_priority": 0,
    /* Рекоменуемое значение 1000  */
    "shutdown_priority": 0,
    /* Остаов системе при регистрации *_FATAL сообщения */
    "stop_with_fatal_log_entry": true,
    /* Список доступных логов и типов сообщений */
    "allow": [],
    /* Список запрещенных логов и типов сообщений */
    "deny": [],
    /* Путь к файлу лога */
    "path": "",
    /* Синхронная запись (открыть-записать-закрыть) */
    "sync": true,
    
    /* --- Опции ротации --- */

    /* Начинать лог с начала файла (старый лог сохраняется, если задан rotation) */
    "startup_rotate": false,
    /* Ограничение лога по размеру (в байтах) */
    "size_limit": 0,
    /* Ограничение лога по времени (в секундах) */
    "time_limit": 0,
    /* Включить сохранение логов при ротации (макс. кол-во файлов) */
    "rotation": 0,
    
    /* --- Опции форматирования --- */
    
    /* Точность отображения времени (от наносекунд до года)  */
    "resolution": "seconds",
    /* Перечисление скрываемых частей сообщения */
    "hide": "",
    /* Локаль */
    "locale": "",
    /* Формат представления даты и времени в формате strftime (игнорируется если заданы resolution или hide) */
    "datetime_format": "",
    /* Включить цветовую схему при записи в файл */
    "colorized": "",
    /* Карта цветов*/
    "color_map": {},
    
    /* --- Кастомизация --- */
    
    /* Настройки стандартного вывода */
    "stdout": { },
    /* Настройки записи в системный лог */
    "syslog": { },
    /* Индивидуальные настройки для логов и типов сообщений */
    "customize": []
  }
}
```
Для минимальной конфигурации достаточно указать путь к файлу лога и, желательно, приоритеты запуска и останова, 
чтобы модуль логгирования запускался раньше, а останавливался позже остальных систем.
```cpp
{
  "startup_priority": -1000,
  "shutdown_priority": 1000,
  "path": "/path/to/log/file.log"
}
```
Отключение синхронной записи (`sync`) увеличит скорость логгирования до 3-х раз,но возможны потери последних записей при аварийном завершении. 

Чтобы быстро запретить запись сообщений конкретного лога или типов сообщений всех логов, нужно перечислить их в массиве `deny`, например:
```cpp
{
  "path": "/path/to/log/file.log",
  "deny": ["TRACE"]
}
```
позволяет отключить трассировку ввода/вывода в Debug режиме. Для того, чтобы разрешить только конкретый лог или тип сообщения, 
то их нужно записать в массив `allow`. Для того чтобы разрешить или запретить конкретные сообщения для конкретного лога, необходимо 
исползовать кастомные настройки для этого лога (см.далее).

### Настройки ротации

По умолчанию запись происходит в один и тот же файл, без ограничений. Чтобы при каждом запуске запись происходила с начала файла:
```cpp
{
  "path": "/path/to/log/file.log",
  "startup_rotate": true
}
```
Чтобы при этом сохрянялся лог от предыдущего запуска в файле `/path/to/log/file.log.old-0`:
```cpp
{
  "path": "/path/to/log/file.log",
  "startup_rotate": true, 
  "rotation": 1
}
```
Здесь, без указанных ограничений на размер и время, значения `rotation` больше еденицы не дают эффекта.
Для обнуления файла при достижении заданного размера (1МБ):
```cpp
{
  "path": "/path/to/log/file.log",
  "size_limit": 1048576
}
```
В следующий конфигурации при запуске, запись будет продолженна в указанный лог, но при достижении размера 
в 1МБ файл будет переименован '/path/to/log/file.log.old-0' и запись начнется с начала файла:
```cpp
{
  "path": "/path/to/log/file.log",
  "size_limit": 1048576, 
  "rotation": 1
}
```
Запись в новый файл при запуске и сохрании истории в 10 файлов, с рамером не более 1 МБ каждый: 
```cpp
{
  "path": "/path/to/log/file.log",
  "startup_rotate": true,
  "size_limit": 1048576, 
  "rotation": 10
}
```
Посуточная ротация файлов с историей за месяц:
```cpp
{
  "path": "/path/to/log/file.log",
  "time_limit": 86400, 
  "rotation": 30
}
```
Посекундные срезы (например при трассировании трафика при большой нагрузе ): 
```cpp
{
  "path": "/path/to/log/file.log",
  "startup_rotate": true,
  "time_limit": 1, 
  "rotation": 1024
}
```

### Настройки форматирования

Для отображения долей секунд используеться опция `resolution` со значенями `seconds` (без долей), `deciseconds`, `centiseconds`, `milliseconds`, 
`microseconds` или `nanoseconds`, например:
```cpp
{
  "path": "/path/to/log/file.log",
  "resolution": "milliseconds"
}
```
Также допустимы значения `minutes`, `hours`, `days`, `month`, `year` которые влияют на отображение информации также как опция `hide`.
Опция `hide` позволяет скрыть несколько частей сообщения в любой комбинации. Таблица возможных значений:

| значение | что скрывает |
| -------- | ------------ |
| date     | все поле даты  |
| time     | все поле времени |
| year     | год в поле даты |
| month    | месяц в поле даты |
| weekday  | день недели в поле даты |
| days     | день месяца в поле даты|
| hours    | часы в поле времени |
| minutes  | минуты в поле времени |
| seconds  | секунды в поле времени|
| fraction | доли секунды |
| name     | имя лога |
| ident    | идентификатор сообщения |
| message  | текст сообщения |

Значения частично скрывающие поля даты и времени влияют на формат отображения этих полей. Возможные комбинации с примерами

| hide                     | resolution         | Пример отображения |
| ------------------------ | -------------      | ------------------ |
|                          | milliseconds       | 18:36:12.805       |
| fraction                 | seconds            | 18:36:12           |
| fraction'seconds         | minutes            | 18:36              |
| fraction'seconds'minutes | hours              | 18h                |

| hide                     | Пример отображения |
| ------------------------ | ------------------ |
| hours                    | 01m 55s.405        |
| hours'minutes            | 1514304173.456     |
| hours'minutes'seconds    | .456               |
| hours'seconds            | 26m.37642          |
| minutes'seconds          | 19h.1631487        |





### Кастомные настройки логов

Позволяет настроить каждый лог индивидульно. Ассациотивный массив, где ключем имя лога в нижнем регистре:

```json
{
  "logger":{
      /*....*/
      "custom":{
        "debug":{
            "limit":0,
            "milliseconds":false,
            "stdout":"clog",
            "path":"./demo",
            "deny":[]
        }
      }
  }
}
```

* path - если не задан задан, то берется из основных настроек
* deny - расширяет список для типов сообщений основных настроек.