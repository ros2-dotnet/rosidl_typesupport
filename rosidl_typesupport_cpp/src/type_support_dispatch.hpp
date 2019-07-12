// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TYPE_SUPPORT_DISPATCH_HPP_
#define TYPE_SUPPORT_DISPATCH_HPP_

#include <cstddef>
#include <cstdio>
#include <cstring>

#include <list>
#include <string>

#ifdef ROSIDL_TYPESUPPORT_CPP_USE_POCO
#include "Poco/SharedLibrary.h"
#endif

#if defined(WIN32)
#include <SDKDDKVer.h>
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define ROSIDL_TYPESUPPORT_CPP_NO_ENV 1
#endif
#endif
#endif

#if defined(ANDROID)
#define ROSIDL_TYPESUPPORT_CPP_NO_ENV 1
#endif

#include "rosidl_typesupport_cpp/type_support_map.h"

namespace rosidl_typesupport_cpp
{

std::string find_library_path(const std::string & library_name);

#if !defined(ROSIDL_TYPESUPPORT_CPP_NO_ENV)
std::string get_env_var(const char * env_var);
#endif

std::list<std::string> split(const std::string & value, const char delimiter);

bool is_file_exist(const char * filename);

extern const char * typesupport_identifier;

template<typename TypeSupport>
const TypeSupport *
get_typesupport_handle_function(
  const TypeSupport * handle, const char * identifier)
{
  if (strcmp(handle->typesupport_identifier, identifier) == 0) {
    return handle;
  }

#ifdef ROSIDL_TYPESUPPORT_CPP_USE_POCO
  if (handle->typesupport_identifier == rosidl_typesupport_cpp::typesupport_identifier) {
    const type_support_map_t * map = \
      static_cast<const type_support_map_t *>(handle->data);
    for (size_t i = 0; i < map->size; ++i) {
      if (strcmp(map->typesupport_identifier[i], identifier) != 0) {
        continue;
      }
      Poco::SharedLibrary * lib = nullptr;
      if (!map->data[i]) {
        char library_name[1024];
        snprintf(
          library_name, 1023, "%s__%s",
          map->package_name, identifier);
        std::string library_path = find_library_path(library_name);
        if (library_path.empty()) {
          fprintf(stderr, "Failed to find library '%s'\n", library_name);
          return nullptr;
        }
        lib = new Poco::SharedLibrary(library_path);
        map->data[i] = lib;
      }
      auto clib = static_cast<const Poco::SharedLibrary *>(map->data[i]);
      lib = const_cast<Poco::SharedLibrary *>(clib);
      if (!lib->hasSymbol(map->symbol_name[i])) {
        fprintf(stderr, "Failed to find symbol '%s' in library\n", map->symbol_name[i]);
        return nullptr;
      }
      void * sym = lib->getSymbol(map->symbol_name[i]);

      typedef const TypeSupport * (* funcSignature)(void);
      funcSignature func = reinterpret_cast<funcSignature>(sym);
      const TypeSupport * ts = func();
      return ts;
    }
  }
#endif

  return nullptr;
}

}  // namespace rosidl_typesupport_cpp

#endif  // TYPE_SUPPORT_DISPATCH_HPP_
