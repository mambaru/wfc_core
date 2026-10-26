# Зависимости для WITH_PROMETHEUS (prometheus-cpp::pull + compression).
# На Debian/Ubuntu при отсутствии пакетов — apt install;
# на остальных ОС — сообщение, каких библиотек не хватает.
# Сборка prometheus-cpp — здесь (не через wci_third_party): только submodule
# civetweb, без googletest; ошибки configure/build/install — FATAL.

macro(wfc_prometheus_ensure_deps)
  set(_prom_missing_libs "")
  set(_prom_missing_apt "")

  find_package(Threads QUIET)
  if ( NOT Threads_FOUND )
    list(APPEND _prom_missing_libs "Threads (pthread)")
  endif()

  find_package(ZLIB QUIET)
  if ( NOT ZLIB_FOUND )
    list(APPEND _prom_missing_libs "ZLIB (zlib development headers)")
    list(APPEND _prom_missing_apt "zlib1g-dev")
  endif()

  find_package(CURL QUIET)
  if ( NOT CURL_FOUND )
    set(_prom_curl_hint "libcurl4-openssl-dev")
  endif()

  if ( _prom_missing_libs OR _prom_missing_apt )
    set(_is_debian FALSE)
    if ( EXISTS "/etc/debian_version" )
      set(_is_debian TRUE)
    elseif ( EXISTS "/etc/os-release" )
      file(READ "/etc/os-release" _os_release)
      if ( _os_release MATCHES "ID=(debian|ubuntu)" OR _os_release MATCHES "ID_LIKE=.*debian" )
        set(_is_debian TRUE)
      endif()
    endif()

    if ( _is_debian AND _prom_missing_apt )
      string(REPLACE ";" " " _prom_apt_str "${_prom_missing_apt}")
      message(STATUS "WITH_PROMETHEUS: missing packages on Debian/Ubuntu, installing: ${_prom_apt_str}")

      execute_process(
        COMMAND id -u
        OUTPUT_VARIABLE _prom_uid
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )

      if ( _prom_uid STREQUAL "0" )
        execute_process(
          COMMAND ${CMAKE_COMMAND} -E env DEBIAN_FRONTEND=noninteractive
                  apt-get update -qq
          RESULT_VARIABLE _prom_apt_rc
          OUTPUT_VARIABLE _prom_apt_out
          ERROR_VARIABLE _prom_apt_err
        )
      else()
        execute_process(
          COMMAND ${CMAKE_COMMAND} -E env DEBIAN_FRONTEND=noninteractive
                  sudo apt-get update -qq
          RESULT_VARIABLE _prom_apt_rc
          OUTPUT_VARIABLE _prom_apt_out
          ERROR_VARIABLE _prom_apt_err
        )
      endif()
      if ( NOT _prom_apt_rc EQUAL 0 )
        message(FATAL_ERROR
          "WITH_PROMETHEUS: apt-get update failed (rc=${_prom_apt_rc}).\n"
          "Install manually: sudo apt-get install -y ${_prom_apt_str}\n"
          "${_prom_apt_err}${_prom_apt_out}"
        )
      endif()

      if ( _prom_uid STREQUAL "0" )
        execute_process(
          COMMAND ${CMAKE_COMMAND} -E env DEBIAN_FRONTEND=noninteractive
                  apt-get install -y ${_prom_missing_apt}
          RESULT_VARIABLE _prom_apt_rc
          OUTPUT_VARIABLE _prom_apt_out
          ERROR_VARIABLE _prom_apt_err
        )
      else()
        execute_process(
          COMMAND ${CMAKE_COMMAND} -E env DEBIAN_FRONTEND=noninteractive
                  sudo apt-get install -y ${_prom_missing_apt}
          RESULT_VARIABLE _prom_apt_rc
          OUTPUT_VARIABLE _prom_apt_out
          ERROR_VARIABLE _prom_apt_err
        )
      endif()
      if ( NOT _prom_apt_rc EQUAL 0 )
        message(FATAL_ERROR
          "WITH_PROMETHEUS: failed to install ${_prom_apt_str} (rc=${_prom_apt_rc}).\n"
          "Install manually: sudo apt-get install -y ${_prom_apt_str}\n"
          "${_prom_apt_err}${_prom_apt_out}"
        )
      endif()

      unset(ZLIB_FOUND CACHE)
      unset(ZLIB_LIBRARY CACHE)
      unset(ZLIB_INCLUDE_DIR CACHE)
      find_package(ZLIB REQUIRED)
      message(STATUS "WITH_PROMETHEUS: packages installed, ZLIB found")
    else()
      set(_msg "WITH_PROMETHEUS: missing system libraries:")
      foreach(_lib IN LISTS _prom_missing_libs)
        string(APPEND _msg "\n  - ${_lib}")
      endforeach()
      string(APPEND _msg "\n\nInstall development packages for your OS, for example:")
      string(APPEND _msg "\n  Debian/Ubuntu: sudo apt-get install -y zlib1g-dev")
      string(APPEND _msg "\n  RHEL/Fedora:   sudo dnf install zlib-devel")
      string(APPEND _msg "\n  Arch:          sudo pacman -S zlib")
      if ( DEFINED _prom_curl_hint AND NOT CURL_FOUND )
        string(APPEND _msg "\n  (optional if ENABLE_PUSH=ON: ${_prom_curl_hint} / libcurl-devel)")
      endif()
      message(FATAL_ERROR "${_msg}")
    endif()
  else()
    find_package(ZLIB REQUIRED)
  endif()

  find_package(Threads REQUIRED)
endmacro()

# Клонирует prometheus-cpp (если нужно) и инициализирует только 3rdparty/civetweb
# (без googletest — ENABLE_TESTING=OFF).
macro(wfc_prometheus_prepare_civetweb)
  set(_prom_src "${CMAKE_BINARY_DIR}/third_party_src/prometheus-cpp")

  if ( NOT EXISTS "${_prom_src}/CMakeLists.txt" )
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/third_party_src")
    set(_prom_clone_ok FALSE)
    foreach(_repo "." "https://gitlab.mamba.ru/cpp/" "https://github.com/jupp0r/")
      if ( _repo STREQUAL "." )
        set(_prom_uri "../prometheus-cpp")
      else()
        set(_prom_uri "${_repo}prometheus-cpp")
      endif()
      message(STATUS "WITH_PROMETHEUS: cloning ${_prom_uri}")
      execute_process(
        COMMAND git clone ${_prom_uri} prometheus-cpp
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/third_party_src"
        RESULT_VARIABLE _prom_clone_rc
        ERROR_VARIABLE _prom_clone_err
      )
      if ( _prom_clone_rc EQUAL 0 AND EXISTS "${_prom_src}/CMakeLists.txt" )
        set(_prom_clone_ok TRUE)
        break()
      endif()
      file(REMOVE_RECURSE "${_prom_src}")
    endforeach()
    if ( NOT _prom_clone_ok )
      message(FATAL_ERROR
        "WITH_PROMETHEUS: failed to clone prometheus-cpp.\n${_prom_clone_err}"
      )
    endif()
  endif()

  if ( NOT EXISTS "${_prom_src}/3rdparty/civetweb/include/civetweb.h"
       AND NOT EXISTS "${_prom_src}/3rdparty/civetweb/include/CivetServer.h" )
    message(STATUS "WITH_PROMETHEUS: git submodule update --init 3rdparty/civetweb")
    execute_process(
      COMMAND git submodule update --init -- 3rdparty/civetweb
      WORKING_DIRECTORY "${_prom_src}"
      RESULT_VARIABLE _civet_rc
      ERROR_VARIABLE _civet_err
    )
    if ( NOT _civet_rc EQUAL 0 )
      message(FATAL_ERROR
        "WITH_PROMETHEUS: failed to init 3rdparty/civetweb (rc=${_civet_rc}).\n"
        "${_civet_err}"
      )
    endif()
  endif()
endmacro()

# Ищет prometheus-cpp-targets.cmake под third_party (lib, lib64, multiarch).
macro(wfc_prometheus_find_targets_dir)
  set(_prom_targets_dir "")
  set(_prom_prefix "${CMAKE_BINARY_DIR}/third_party")
  set(_prom_candidates
    "${_prom_prefix}/lib/cmake/prometheus-cpp"
    "${_prom_prefix}/lib64/cmake/prometheus-cpp"
  )
  if ( DEFINED CMAKE_LIBRARY_ARCHITECTURE AND NOT "${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "" )
    list(APPEND _prom_candidates
      "${_prom_prefix}/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/prometheus-cpp")
  endif()
  file(GLOB _prom_glob_candidates
    "${_prom_prefix}/lib/*/cmake/prometheus-cpp"
  )
  list(APPEND _prom_candidates ${_prom_glob_candidates})

  foreach(_dir IN LISTS _prom_candidates)
    if ( EXISTS "${_dir}/prometheus-cpp-targets.cmake" )
      set(_prom_targets_dir "${_dir}")
      break()
    endif()
  endforeach()
endmacro()

# Clone/civetweb + configure/build/install prometheus-cpp; подключает targets.
macro(wfc_prometheus_build_and_import)
  wfc_prometheus_prepare_civetweb()

  set(_prom_src "${CMAKE_BINARY_DIR}/third_party_src/prometheus-cpp")
  set(_prom_prefix "${CMAKE_BINARY_DIR}/third_party")
  file(MAKE_DIRECTORY "${_prom_prefix}")

  wfc_prometheus_find_targets_dir()
  if ( _prom_targets_dir STREQUAL "" )
    message(STATUS "WITH_PROMETHEUS: configuring prometheus-cpp")
    # -S/-B: CMake >= 3.13; вывод не глушим — иначе CI молчит при ошибке
    execute_process(
      COMMAND ${CMAKE_COMMAND}
        -S "${_prom_src}"
        -B "${_prom_src}/build"
        -DENABLE_PUSH=OFF
        -DENABLE_TESTING=OFF
        -DENABLE_COMPRESSION=ON
        -DUSE_THIRDPARTY_LIBRARIES=ON
        -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
        -DCMAKE_INSTALL_PREFIX=${_prom_prefix}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DFAIL_ON_WARNINGS=OFF
      RESULT_VARIABLE _prom_rc
    )
    if ( NOT _prom_rc EQUAL 0 )
      message(FATAL_ERROR
        "WITH_PROMETHEUS: prometheus-cpp configure failed (rc=${_prom_rc}). "
        "See log above."
      )
    endif()

    message(STATUS "WITH_PROMETHEUS: building prometheus-cpp")
    execute_process(
      COMMAND ${CMAKE_COMMAND} --build "${_prom_src}/build" --parallel 4
      RESULT_VARIABLE _prom_rc
    )
    if ( NOT _prom_rc EQUAL 0 )
      message(FATAL_ERROR
        "WITH_PROMETHEUS: prometheus-cpp build failed (rc=${_prom_rc}). "
        "See log above."
      )
    endif()

    message(STATUS "WITH_PROMETHEUS: installing prometheus-cpp to ${_prom_prefix}")
    execute_process(
      COMMAND ${CMAKE_COMMAND} --install "${_prom_src}/build"
      RESULT_VARIABLE _prom_rc
    )
    if ( NOT _prom_rc EQUAL 0 )
      # cmake --install с 3.15; fallback
      execute_process(
        COMMAND ${CMAKE_COMMAND} --build "${_prom_src}/build" --target install
        RESULT_VARIABLE _prom_rc
      )
    endif()
    if ( NOT _prom_rc EQUAL 0 )
      message(FATAL_ERROR
        "WITH_PROMETHEUS: prometheus-cpp install failed (rc=${_prom_rc}). "
        "See log above."
      )
    endif()

    wfc_prometheus_find_targets_dir()
  else()
    message(STATUS "WITH_PROMETHEUS: using existing ${_prom_targets_dir}")
  endif()

  if ( _prom_targets_dir STREQUAL "" )
    message(FATAL_ERROR
      "WITH_PROMETHEUS: prometheus-cpp-targets.cmake not found under ${_prom_prefix}.\n"
      "Expected e.g. ${_prom_prefix}/lib/cmake/prometheus-cpp/prometheus-cpp-targets.cmake"
    )
  endif()

  list(APPEND CMAKE_PREFIX_PATH "${_prom_prefix}")
  list(APPEND CMAKE_MODULE_PATH "${_prom_targets_dir}")
  find_package(prometheus-cpp REQUIRED CONFIG PATHS "${_prom_targets_dir}" NO_DEFAULT_PATH)
  message(STATUS "WITH_PROMETHEUS: imported prometheus-cpp from ${_prom_targets_dir}")
endmacro()
