# Модуль логирования 

Управляет системой логированя WFC. 

```
2015-09-30 17:17:56.640 CONFIG END       core::start: module 'startup'...Done!
2015-09-30 17:17:56.640 SYSLOG MESSAGE   daemon ./demod started!
2015-09-30 17:17:56.640 DOMAIN MESSAGE   Daemon reconfigured!
2015-09-30 17:17:59.628 CONFIG BEGIN     Reload Configuration And Reconfigure
2015-09-30 17:17:59.629 CONFIG END       Reload Configuration And Reconfigure
2015-09-30 17:17:59.629 CONFIG MESSAGE   ----------- configuration -------------
2015-09-30 17:17:59.629 CONFIG BEGIN     core::configure: object 'client-tcp'...
```
Каждая строка лога систоит:

* Дата 
* Время 
* Имя 
- CONFIG
- DOMIAN
- COMMON
- SYSLOG
- DEBUG
* Тип
- ERROR
- WARNING
- MESSAGE
- FATAL
- BEGIN
- END
- DEBUG
- TRACE
- PROGRESS
* Сообщение


## Настройки 

```json
{
  "logger":{
      "enabled":true,
      "startup_priority":-1000,
      "shutdown_priority":1000,

      "limit":0,
      "milliseconds":false,
      "single":true,
      "stdout":"clog",
      "path":"./demo",
      "syslog":"",

      "deny":[],
      "custom":{}
  }
}
```

### Кастомные настройки логов