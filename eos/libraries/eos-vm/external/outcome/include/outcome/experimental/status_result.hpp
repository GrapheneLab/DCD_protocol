/* A very simple result type
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (59 commits)
File Created: Apr 2018


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
(See accompanying file Licence.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef OUTCOME_EXPERIMENTAL_STATUS_RESULT_HPP
#define OUTCOME_EXPERIMENTAL_STATUS_RESULT_HPP

#include "../basic_result.hpp"
#include "../policy/fail_to_compile_observers.hpp"

#include "status-code/include/system_error2.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  // Customise _set_error_is_errno
  template <class State> constexpr inline void _set_error_is_errno(State &state, const SYSTEM_ERROR2_NAMESPACE::generic_code & /*unused*/) { state._status |= status_error_is_errno; }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const SYSTEM_ERROR2_NAMESPACE::posix_code & /*unused*/) { state._status |= status_error_is_errno; }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const SYSTEM_ERROR2_NAMESPACE::errc & /*unused*/) { state._status |= status_error_is_errno; }

}  // namespace detail

namespace experimental
{
  using namespace SYSTEM_ERROR2_NAMESPACE;
  using OUTCOME_V2_NAMESPACE::success;
  using OUTCOME_V2_NAMESPACE::failure;

  namespace policy
  {
    using namespace OUTCOME_V2_NAMESPACE::policy;
    template <class T, class EC, class E> struct status_code_throw
    {
      static_assert(!std::is_same<T, T>::value, "policy::status_code_throw not specialised for these types, did you use status_result<T, status_code<DomainType>, E>?");
    };
    template <class T, class DomainType> struct status_code_throw<T, status_code<DomainType>, void> : base
    {
      using _base = base;
      template <class Impl> static constexpr void wide_value_check(Impl &&self)
      {
        if(!base::_has_value(static_cast<Impl &&>(self)))
        {
          if(base::_has_error(static_cast<Impl &&>(self)))
          {
#ifdef __cpp_exceptions
            base::_error(static_cast<Impl &&>(self)).throw_exception();
#else
            OUTCOME_THROW_EXCEPTION("wide value check failed");
#endif
          }
        }
      }
      template <class Impl> static constexpr void wide_error_check(Impl &&self) { _base::narrow_error_check(static_cast<Impl &&>(self)); }
    };
    template <class T, class DomainType> struct status_code_throw<T, errored_status_code<DomainType>, void> : status_code_throw<T, status_code<DomainType>, void>
    {
      status_code_throw() = default;
      using status_code_throw<T, status_code<DomainType>, void>::status_code_throw;
    };

    template <class T, class EC>
    using default_status_result_policy = std::conditional_t<                            //
    std::is_void<EC>::value,                                                            //
    OUTCOME_V2_NAMESPACE::policy::terminate,                                            //
    std::conditional_t<is_status_code<EC>::value || is_errored_status_code<EC>::value,  //
                       status_code_throw<T, EC, void>,                                  //
                       OUTCOME_V2_NAMESPACE::policy::fail_to_compile_observers          //
                       >>;
  }  // namespace policy

  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  template <class R, class S = system_code, class NoValuePolicy = policy::default_status_result_policy<R, S>>  //
  using status_result = basic_result<R, S, NoValuePolicy>;

}  // namespace experimental

OUTCOME_V2_NAMESPACE_END

#endif
