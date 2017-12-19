# Модуль логирования 

Управляет системой логированя WFC. 

```
2017-12-19 18:37:03.049 CONFIG BEGIN Configure component 'server-tcp'...
2017-12-19 18:37:03.049 CONFIG MESSAGE Instance 'server-tcp1' is initial configured
2017-12-19 18:37:03.049 CONFIG END Configure component 'server-tcp'...Done
```
Каждая строка лога систоит:

* Дата 
* Время + доли секунды
* Имя лога:
  * CONFIG - конфигурирование и инициализация системы
  * DOMIAN - прикладной лог (важные сообщения)
  * COMMON - общий лог (все остальное)
  * SYSLOG - системый лог плюс запись в syslog (критические сообщения)
  * DEBUG  - лог отладки (отключаеться в релиз-версии на уровне компилятора )
  * JSONRPC - лог jsonrpc движка
  * IOW     - лог сетевого движка
* ID сообщения:
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

* enabled - при отключении модуля, запись в clog без форматирования и отметок времени ("имя" "тип" "сообщение")
* startup_priority  - приоритет запуска должен быть минимальный, чтобы модуль запускался раньше других объектов
* shutdown_priority - приоритет останова должен быть максимальный, чтобы модуль останавливался позже других объектов
* limit - ограничение на размер файла лога в байтах. При достижении предела файл обнуляеться. 0 - без ограничений
* milliseconds - включить отметки миллисекунд (например 17:17:59.629)
* stdout - вывод помимо файла. Возможные значения (clog, cout, cerr или пустая строка (без вывода) ).
* path - перфикс пути файла лога (см. single)
* single - если true то общий файл для всех типов логов (path+".log"), иначе для каждого типа свой лог (path+"-"+имя+".log")
* syslog - идентификатор syslog'a для ::openlog. Разрешает паралельную запись в системный лог, для логов syslog 
* deny - запрещает запись логов или типов сообщений. Например ["iow", "DEBUG"], запрещает запись всех iow-логов и DEBUG сообщений для остальных логов. Имя задаеться в нижнем регистре, а тип верхним

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