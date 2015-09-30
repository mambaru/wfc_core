# Модуль логгирования 

Управляет системой логиррованя WFC. 

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
