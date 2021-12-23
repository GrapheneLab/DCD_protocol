#include <eosio/history_api_plugin/history_api_plugin.hpp>
#include <eosio/chain/exceptions.hpp>

#include <fc/io/json.hpp>

namespace eosio {

using namespace eosio;

static appbase::abstract_plugin& _history_api_plugin = app().register_plugin<history_api_plugin>();

history_api_plugin::history_api_plugin(){}
history_api_plugin::~history_api_plugin(){}

void history_api_plugin::set_program_options(options_description&, options_description&) {}
void history_api_plugin::plugin_initialize(const variables_map&) {}

#define CALL_HTTP(api_name, api_handle, api_namespace, call_name) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             fc::variant result( api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()) ); \
             cb(200, std::move(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CALL_WSS(api_name, api_handle, api_namespace, call_name) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             fc::variant result( api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()) ); \
             cb(200, std::move(result)); \
          } catch (...) { \
             ws_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CALL_GATE(api_name, api_handle, api_namespace, call_name) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             fc::variant result( api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()) ); \
             cb(200, std::move(result)); \
          } catch (...) { \
             gate_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CHAIN_RO_CALL_HTTP(call_name) CALL_HTTP(history, ro_api, history_apis::read_only, call_name)
#define CHAIN_RO_CALL_WSS(call_name) CALL_WSS(history, ro_api, history_apis::read_only, call_name)
#define CHAIN_RO_CALL_GATE(call_name) CALL_GATE(history, ro_api, history_apis::read_only, call_name)

//#define CHAIN_RW_CALL(call_name) CALL(history, rw_api, history_apis::read_write, call_name)

void history_api_plugin::plugin_startup() {
   ilog( "starting history_api_plugin" );
   auto ro_api = app().get_plugin<history_plugin>().get_read_only_api();
   //auto rw_api = app().get_plugin<history_plugin>().get_read_write_api();

   app().get_plugin<http_plugin>().add_api({
//      CHAIN_RO_CALL(get_transaction),
      CHAIN_RO_CALL_HTTP(get_actions),
      CHAIN_RO_CALL_HTTP(get_transaction),
      CHAIN_RO_CALL_HTTP(get_key_accounts),
      CHAIN_RO_CALL_HTTP(get_controlled_accounts)
   });

   app().get_plugin<ws_plugin>().add_api({
//      CHAIN_RO_CALL(get_transaction),
      CHAIN_RO_CALL_WSS(get_actions),
      CHAIN_RO_CALL_WSS(get_transaction),
      CHAIN_RO_CALL_WSS(get_key_accounts),
      CHAIN_RO_CALL_WSS(get_controlled_accounts)
   });

      app().get_plugin<gate_plugin>().add_api({
//      CHAIN_RO_CALL(get_transaction),
      CHAIN_RO_CALL_GATE(get_actions),
      CHAIN_RO_CALL_GATE(get_transaction),
      CHAIN_RO_CALL_GATE(get_key_accounts),
      CHAIN_RO_CALL_GATE(get_controlled_accounts)
   });
}

void history_api_plugin::plugin_shutdown() {}

}
