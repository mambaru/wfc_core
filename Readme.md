
# wfc_core: Пакет ядра WFC

* Документация [doxygen](https://mambaru.github.io/wfc_core/index.html).
* Репозитарий на [github.com](https://github.com/mambaru/wfc_core).
* Отчет [coverage](https://mambaru.github.io/wfc_core/cov-report/index.html)


Включает модули: 
* startup - разабор параметров командной строки, запуск системы, демонизация
* config - модуль конфигурации
* logger - логирование
* core - ядро
* workflow - очереди и потоки
* statistics - сбор статистики и отправка в btp

## startup модуль
Конфигурация:
```
{"startup":{}}
```
Параметры командной строки:
* -v [ --version ] - краткая информация о версии, времени, типе сборки, компиляторе и ОС
* -h [ --help ] _component_ - помощь по параметрам командной строки или справка по указанному компоненту (если реализована)
* -i [ --info ] _package_ - расширенная информация о версии и список подключенных пакетов или расширенная информация о версии указанного пакета
* --module-list - список подключенных модулей (группировка по пакетам)
* --component-list - список подключенных компонент (группировка по *пакетам*)
* --check-config _package_ - проверка файла конфигурации
* -G [ --generate ] _component_lits_ - генерация конфигурации для всех компонент или только указанных

Параметры запуска:
* -u [ --user ] name - cменить пользователя 
* -w [ --working-directory ] path - рабочая директория 
* -d [ --daemonize ] - запустить как демон
* -W [ --wait-daemonize ] - дождаться успешного запуска все компонент дочернего процесса перед завершением родительского 
* -a [ --autoup ] _uptime_ (=-1) - запустить мониторящий процесс, который будет перезапускать демон в случае аварийного завершения работы. Можно задать минимальное 
время работы демона в секундах, до которого перезапуск не осуществляется (чтобы избежать "кувыркания" если демон падает сразу после запуска) 
* -A [ --success-autoup ] - перезапускает демон, даже если он корректно завершил работу (только совместно с --autoup)
* -c [ --coredump ] - разрешить запись дампов памяти 
* -n [ --name ] name - уникальное имя экземпляра (если нужно запустить несколько экземпляров с одинаковыми конфигурационными файлами)
* -C [ --config ] path - файл конфигурации
* -P [ --pid-dir ] path - директория для pid-файла.
* -O [ --object-options ] arg     <<object-name>>:arg=value[:arg2=value2...] custom options for instance objects
* -S [ --startup-options ] arg    <<object-name>>:arg=value[:arg2=value2...] custom option for instance objects only for first start (сleaned after restart by autoup)

### Расширенная информация о версии
Информация о версии формируется автоматически на базе доступной информации git-репозитария. В качестве номера версии используется последний доступный тег и локальный счетчик сборок, 
например ```0.7.1-2```. Если на момент сборки текущий коммит не помечен тегом, или есть незафиксированные изменения, то об этом появляется информация в квадратных скобках:
```
0.7.1[17.0.1]-2
──┬──  │ │ │  └── локальный счетчик сборок
  │    │ │ └───── количество измененных, но незакомиченных файлов 
  │    │ └─────── количество измененных, но незапушенных в файлов 
  │    └───────── количество коммитов после последнего тега
  └────────────── версия (последний тег), если отсутствует, то бранч сборки 
```
Расширенную информацию о версии можно посмотреть с использованием параметра --info. Пример для пакета wfc_core:
```
        Enabled: 1
        Name: wfc_core
        Version: 0.7.1[16.0.1]-36
        Compiler: g++-4.8 (SUSE Linux) 4.8.5
        Build Type: Debug
        Build Date: 2018-10-18 11:47:54 +03:00
        Build Flags: g++-4.8  -W -Wall -Werror -pedantic -ftemplate-backtrace-limit=0 -fpic -W -Wall -Werror -pedantic -ftemplate-backtrace-limit=0 -fpic -O0 -g
        Branch: devel
        Commit: 3a66ebe2c7d9787897db422087053902a97bae1b
        Commit Date: 2018-10-18 11:41:52 +03:00
        Commit Author: Vladimir Migashko <migashko@gmail.com>
        Commit Message: CI initial
        All Authors: Vladimir Migashko <migashko@gmail.com> (514), Владимир Мигашко <migashko@wamba.com> (7), Dmitry Saprykin <saprykin.dmitry@gmail.com> (2)
        Initial Author: Vladimir Migashko <migashko@gmail.com>
        Description: WFC core modules
```
* *All Authors* - это список всех авторов, с указанием числа внесенных коммитов
* *Initial Author* - автор первого коммита

### Генерация конфигурации
Используйте генератор конфигурации для получения актуального списка опций со значениями по умолчанию. Если значение по умолчанию вас устраивает, то можно удалить 
это поле для упрощения конфигурации. Конфигурация выводиться без форматирования, чтобы сделать ее более читабельной:
```
demod -G | python -mjson.tool
```
Однако этот метод пересортировывает поля, что не всегда наглядно. С помощью сайта http://jsoneditoronline.org/ это можно отформатировать через браузер с сохранением 
порядка полей (как задумано разработчиком конкретного модуля) просто скопировав сгенерированную конфигурацию. 

Для получения конфигурации конкретного компонента, просто укажите его имя. 
```
demod -G core
```
Генератор по умолчанию не формирует данные для массивов, но некоторые компоненты 
поддерживают расширенную генерацию, заполняя массивы объектами и инициализируют поля недефолтными значениями. Для этого нужно передать режим генерации через 
двоеточие:
```
demod -G workflow:example
```
Расширенную генерацию поддерживают не все компоненты, за уточнением обращайтесь к документации соответствующего компонента. 

### Дополнительные аргументы для объектов

Некоторые опции объектов имеет смысл передавать через параметры командной строки, а не прописывать их в файл конфигурации. Это такие опции, например,
которые нужны только для первоначальной инициализации, для последующих запусков они уже не нужны. Для передачи таких опций конкретному объекту используются 
параметры командной строки:
* --object-options _имя_объекта_:_ключ_=_значение_
* --startup-options _имя_объекта_:_ключ_=_значение_
Разница между ними только при использовании функции автоподнятия (--autoup). В первом случае опции доступны всегда, во втором только до перезапуска контролирующим процессом.
Наборы доступных опций (если они есть), смотрите в описании конкретного компонента. В качестве имени объекта допустимо передавать имена объектов описанных в файле конфигурации.

## Модуль конфигурации
Конфигурация:
```
{
  "config": {
      /* Для отключения объекта */
      "enabled": true,
      /* Перечитать конфигурацию по сигналу SIGHUP */
      "reload_sighup": false,
      /* Перечитать конфигурацию при изменении файла (интервал проверки) */
      "reload_changed_ms": 0
  }
}
```
При старте программы компонент конфигурирования доступен с опциями по умолчанию, которые могут быть изменены после инициализации системы. Отвечает за первичную десериализацию 
и обеспечивает доступ к JSON-конфигурации конкретных объектов. 

Внимание. Реконфигурация на лету может поддерживаться не всеми модулями или быть некорректно реализована. На данный момент не рекомендуется использовать эту функцию на продакшене, 
особенно на больших нагрузках.

## Модуль логирования
Конфигурация:
```
  "logger": {
    "enabled": true,
    "startup_priority": 0,
    "shutdown_priority": 0,
    "stop_with_fatal_log_entry": true
    
    /* Прочие опции логера */
  }

```
Для того чтобы логер запускался раньше остальных компонентов системы, о отключался позже, необходимо задать минимальный приоритет запуска и максимальный для останова, например:
```
{
  "logger": {
    "startup_priority": -1000,
    "shutdown_priority": 1000,
  }
}
```
Опция *stop_with_fatal_log_entry* позволяет отключить автоматическое аварийное завершение при сообщениях типа *FATAL*.
С остальными опциями логирования можно ознакомиться здесь 
https://mambaru.github.io/wlog/index.html

## Модуль ядра
Конфигурация:
```
{
  "core": {
    /* таймер ядра (проверка сигнала на завершение работы, переконфигурацию и пр.) */
    "core_timeout_ms": 1000,
    /* таймер фоновых задач */
    "idle_timeout_ms": 1000,
    /* Ограничение по памяти (для отладки) */
    "rlimit_as_mb": 0,
    /* Отключить сбор статистики для всех модулей */
    "disable_statistics": false,
    
    /* Для отладки */
    "nocall_callback_abort": false,
    "nocall_callback_show": true,
    "double_callback_abort": false,
    "double_callback_show": true,
    
    /* Общий workflow */
    "common-workflow": {
      /* Потоки обработки очереди (0 - в основном потоке приложения)*/
      "threads": 0,
      /* Размер очереди при превышении которой производится запись предупреждения в лог */
      "wrnsize": 0,
      /* Максимальный размер очереди после чего, все входящие задания сбрасываются */
      "maxsize": 0,
      /* Интервал проверки счетчиков очередей */
      "control_ms": 0,
      /* Отладочный режим (больше информации в лог) */
      "debug": false,
      /* Ядра CPU для потоков обработки (если не заданно, то wfc-cpu)*/
      "cpu": []
    },
    /* Ядра CPU для потоков WFC по умолчанию */
    "wfc-cpu": [],
    /* Ядра CPU для всех остальных потоков  */
    "sys-cpu": []
  }
}
```
## workflow
Конфигурация:
```
{
  "workflow": [
    {
      "name": "workflow1",
      "enabled": true,
      "startup_priority": 0,
      "shutdown_priority": 0,
      /* Ядра CPU для потоков обработки (если не заданно, то wfc-cpu)*/
      "cpu": [],
      /* Потоки обработки очереди (0 - в основном потоке приложения)*/
      "threads": 0,
      /* Размер очереди при превышении которой производится запись предупреждения в лог */
      "wrnsize": 0,
      /* Максимальный размер очереди после чего, все входящие задания сбрасываются */
      "maxsize": 0,
      /* Интервал проверки счетчиков очередей */
      "control_ms": 0,
      /* Отладочный режим (больше информации в лог) */
      "debug": false,
      /* Задержка обработки для всех входящих заданий */
      "post_delay_ms": 0,
      /* Ограничение на скорость обработки (заданий в секунду) */
      "rate_limit": 0
    }
  ]
}
```
В двух словах workflow это очередь и пул потоков ее обрабатывающих. Один и тот-же workflow могут использовать несколько компонет (имя workflow указывается в настройках компонентов, 
и как правило, поле так и называется *workflow*). Может использоваться как для обработки входящего потока сообщений так и для всевозможных таймеров. Как правило для всех таймеров 
достаточно *common-workflow* ядра, а для высоконагруженных сервисов необходим свой workflow. Для отладки есть возможность искусственно увеличить минимальное время обработки заданий, 
или наложить ограничение на максимальную скорость (отработка будет идти не пачками с паузой, а более или менее равномерно)

## Модуль статистики

Доступно если собрано с опцией *WFC_ENABLE_STAT=ON*. Включает следующие компоненты:
* statistics_aggregator - агрегатор статистики 
* statistics_gateway - шлюз для отправки в BTP (хранилище)
* statistics_log - запись агрегированной статистики в лог
* statistics_service - интерфейс для получения агрегированной статистики 
* system_statistics - сбор системной статистики (utime, stime, vsize, rss)
