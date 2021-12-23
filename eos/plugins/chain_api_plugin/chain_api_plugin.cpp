#include <eosio/chain_api_plugin/chain_api_plugin.hpp>
#include <eosio/chain/exceptions.hpp>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _chain_api_plugin = app().register_plugin<chain_api_plugin>();

using namespace eosio;

class chain_api_plugin_impl {
public:
   chain_api_plugin_impl(controller& db)
      : db(db) {}

   controller& db;
};


chain_api_plugin::chain_api_plugin(){}
chain_api_plugin::~chain_api_plugin(){}

void chain_api_plugin::set_program_options(options_description&, options_description&) {}
void chain_api_plugin::plugin_initialize(const variables_map&) {}

struct async_result_visitor : public fc::visitor<fc::variant> {
   template<typename T>
   fc::variant operator()(const T& v) const {
      return fc::variant(v);
   }
};

#define CALL(api_name, api_handle, api_namespace, call_name, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          api_handle.validate(); \
          try { \
             if (body.empty()) body = "{}"; \
             fc::variant result( api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()) ); \
             cb(http_response_code, std::move(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CALL_ASYNC(api_name, api_handle, api_namespace, call_name, call_result, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
      if (body.empty()) body = "{}"; \
      api_handle.validate(); \
      api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>(),\
         [cb, body](const fc::static_variant<fc::exception_ptr, call_result>& result){\
            if (result.contains<fc::exception_ptr>()) {\
               try {\
                  result.get<fc::exception_ptr>()->dynamic_rethrow_exception();\
               } catch (...) {\
                  http_plugin::handle_exception(#api_name, #call_name, body, cb);\
               }\
            } else {\
               cb(http_response_code, result.visit(async_result_visitor()));\
            }\
         });\
   }\
}

#define CALL_WSS(api_name, api_handle, api_namespace, call_name, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          api_handle.validate(); \
          try { \
             if (body.empty()) body = "{}"; \
             fc::variant result( api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()) ); \
             cb(http_response_code, std::move(result)); \
          } catch (...) { \
             ws_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CALL_ASYNC_WSS(api_name, api_handle, api_namespace, call_name, call_result, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
      if (body.empty()) body = "{}"; \
      api_handle.validate(); \
      api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>(),\
         [cb, body](const fc::static_variant<fc::exception_ptr, call_result>& result){\
            if (result.contains<fc::exception_ptr>()) {\
               try {\
                  result.get<fc::exception_ptr>()->dynamic_rethrow_exception();\
               } catch (...) {\
                  ws_plugin::handle_exception(#api_name, #call_name, body, cb);\
               }\
            } else {\
               cb(http_response_code, result.visit(async_result_visitor()));\
            }\
         });\
   }\
}

#define CALL_GATE(api_name, api_handle, api_namespace, call_name, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          api_handle.validate(); \
          try { \
             if (body.empty()) body = "{}"; \
             fc::variant result( api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()) ); \
             cb(http_response_code, std::move(result)); \
          } catch (...) { \
             gate_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CALL_ASYNC_GATE(api_name, api_handle, api_namespace, call_name, call_result, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
      if (body.empty()) body = "{}"; \
      api_handle.validate(); \
      api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>(),\
         [cb, body](const fc::static_variant<fc::exception_ptr, call_result>& result){\
            if (result.contains<fc::exception_ptr>()) {\
               try {\
                  result.get<fc::exception_ptr>()->dynamic_rethrow_exception();\
               } catch (...) {\
                  gate_plugin::handle_exception(#api_name, #call_name, body, cb);\
               }\
            } else {\
               cb(http_response_code, result.visit(async_result_visitor()));\
            }\
         });\
   }\
}

#define CHAIN_RO_CALL(call_name, http_response_code) CALL(chain, ro_api, chain_apis::read_only, call_name, http_response_code)
#define CHAIN_RW_CALL(call_name, http_response_code) CALL(chain, rw_api, chain_apis::read_write, call_name, http_response_code)
#define CHAIN_RO_CALL_ASYNC(call_name, call_result, http_response_code) CALL_ASYNC(chain, ro_api, chain_apis::read_only, call_name, call_result, http_response_code)
#define CHAIN_RW_CALL_ASYNC(call_name, call_result, http_response_code) CALL_ASYNC(chain, rw_api, chain_apis::read_write, call_name, call_result, http_response_code)

#define CHAIN_RO_CALL_WSS(call_name, http_response_code) CALL_WSS(chain, ro_api, chain_apis::read_only, call_name, http_response_code)
#define CHAIN_RW_CALL_WSS(call_name, http_response_code) CALL_WSS(chain, rw_api, chain_apis::read_write, call_name, http_response_code)
#define CHAIN_RO_CALL_ASYNC_WSS(call_name, call_result, http_response_code) CALL_ASYNC_WSS(chain, ro_api, chain_apis::read_only, call_name, call_result, http_response_code)
#define CHAIN_RW_CALL_ASYNC_WSS(call_name, call_result, http_response_code) CALL_ASYNC_WSS(chain, rw_api, chain_apis::read_write, call_name, call_result, http_response_code)

#define CHAIN_RO_CALL_GATE(call_name, http_response_code) CALL_GATE(chain, ro_api, chain_apis::read_only, call_name, http_response_code)
#define CHAIN_RW_CALL_GATE(call_name, http_response_code) CALL_GATE(chain, rw_api, chain_apis::read_write, call_name, http_response_code)
#define CHAIN_RO_CALL_ASYNC_GATE(call_name, call_result, http_response_code) CALL_ASYNC_GATE(chain, ro_api, chain_apis::read_only, call_name, call_result, http_response_code)
#define CHAIN_RW_CALL_ASYNC_GATE(call_name, call_result, http_response_code) CALL_ASYNC_GATE(chain, rw_api, chain_apis::read_write, call_name, call_result, http_response_code)

void chain_api_plugin::plugin_startup() {
   ilog( "starting chain_api_plugin" );
   my.reset(new chain_api_plugin_impl(app().get_plugin<chain_plugin>().chain()));
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();

   auto& _http_plugin = app().get_plugin<http_plugin>();
   ro_api.set_shorten_abi_errors( !_http_plugin.verbose_errors() );

   _http_plugin.add_api({
      CHAIN_RO_CALL(get_info, 200l),
      CHAIN_RO_CALL(get_activated_protocol_features, 200),
      CHAIN_RO_CALL(get_block, 200),
      CHAIN_RO_CALL(get_block_header_state, 200),
      CHAIN_RO_CALL(get_account, 200),
      CHAIN_RO_CALL(get_code, 200),
      CHAIN_RO_CALL(get_code_hash, 200),
      CHAIN_RO_CALL(get_abi, 200),
      CHAIN_RO_CALL(get_raw_code_and_abi, 200),
      CHAIN_RO_CALL(get_raw_abi, 200),
      CHAIN_RO_CALL(get_table_rows, 200),
      CHAIN_RO_CALL(get_table_by_scope, 200),
      CHAIN_RO_CALL(get_currency_balance, 200),
      CHAIN_RO_CALL(get_currency_stats, 200),
      CHAIN_RO_CALL(get_producers, 200),
      CHAIN_RO_CALL(get_producer_schedule, 200),
      CHAIN_RO_CALL(get_rate_schedule, 200),
      CHAIN_RO_CALL(get_scheduled_transactions, 200),
      CHAIN_RO_CALL(abi_json_to_bin, 200),
      CHAIN_RO_CALL(get_action_fee, 200),
      CHAIN_RO_CALL(get_required_fee, 200),
      CHAIN_RO_CALL(get_fee_rate, 200),
      CHAIN_RO_CALL(abi_bin_to_json, 200),
      CHAIN_RO_CALL(get_required_keys, 200),
      CHAIN_RO_CALL(get_transaction_id, 200),
      CHAIN_RW_CALL_ASYNC(push_block, chain_apis::read_write::push_block_results, 202),
      CHAIN_RW_CALL_ASYNC(push_transaction, chain_apis::read_write::push_transaction_results, 202),
      CHAIN_RW_CALL_ASYNC(push_transactions, chain_apis::read_write::push_transactions_results, 202),
      CHAIN_RW_CALL_ASYNC(send_transaction, chain_apis::read_write::send_transaction_results, 202)
   });

   auto& _ws_plugin = app().get_plugin<ws_plugin>();
   ro_api.set_shorten_abi_errors( !_ws_plugin.verbose_errors() );

   _ws_plugin.add_api({
      CHAIN_RO_CALL_WSS(get_info, 200l),
      CHAIN_RO_CALL_WSS(get_activated_protocol_features, 200),
      CHAIN_RO_CALL_WSS(get_block, 200),
      CHAIN_RO_CALL_WSS(get_block_header_state, 200),
      CHAIN_RO_CALL_WSS(get_account, 200),
      CHAIN_RO_CALL_WSS(get_code, 200),
      CHAIN_RO_CALL_WSS(get_code_hash, 200),
      CHAIN_RO_CALL_WSS(get_abi, 200),
      CHAIN_RO_CALL_WSS(get_raw_code_and_abi, 200),
      CHAIN_RO_CALL_WSS(get_raw_abi, 200),
      CHAIN_RO_CALL_WSS(get_table_rows, 200),
      CHAIN_RO_CALL_WSS(get_table_by_scope, 200),
      CHAIN_RO_CALL_WSS(get_currency_balance, 200),
      CHAIN_RO_CALL_WSS(get_currency_stats, 200),
      CHAIN_RO_CALL_WSS(get_producers, 200),
      CHAIN_RO_CALL_WSS(get_producer_schedule, 200),
      CHAIN_RO_CALL_WSS(get_rate_schedule, 200),
      CHAIN_RO_CALL_WSS(get_scheduled_transactions, 200),
      CHAIN_RO_CALL_WSS(abi_json_to_bin, 200),
      CHAIN_RO_CALL(get_action_fee, 200),
      CHAIN_RO_CALL(get_required_fee, 200),
      CHAIN_RO_CALL(get_fee_rate, 200),      
      CHAIN_RO_CALL_WSS(abi_bin_to_json, 200),
      CHAIN_RO_CALL_WSS(get_required_keys, 200),
      CHAIN_RO_CALL_WSS(get_transaction_id, 200),
      CHAIN_RW_CALL_ASYNC_WSS(push_block, chain_apis::read_write::push_block_results, 202),
      CHAIN_RW_CALL_ASYNC_WSS(push_transaction, chain_apis::read_write::push_transaction_results, 202),
      CHAIN_RW_CALL_ASYNC_WSS(push_transactions, chain_apis::read_write::push_transactions_results, 202),
      CHAIN_RW_CALL_ASYNC_WSS(send_transaction, chain_apis::read_write::send_transaction_results, 202)
   });

   auto& _gate_plugin = app().get_plugin<gate_plugin>();
   ro_api.set_shorten_abi_errors( !_gate_plugin.verbose_errors() );

   _gate_plugin.add_api({
      CHAIN_RO_CALL_GATE(get_info, 200l),
      CHAIN_RO_CALL_GATE(get_activated_protocol_features, 200),
      CHAIN_RO_CALL_GATE(get_block, 200),
      CHAIN_RO_CALL_GATE(get_block_header_state, 200),
      CHAIN_RO_CALL_GATE(get_account, 200),
      CHAIN_RO_CALL_GATE(get_code, 200),
      CHAIN_RO_CALL_GATE(get_code_hash, 200),
      CHAIN_RO_CALL_GATE(get_abi, 200),
      CHAIN_RO_CALL_GATE(get_raw_code_and_abi, 200),
      CHAIN_RO_CALL_GATE(get_raw_abi, 200),
      CHAIN_RO_CALL_GATE(get_table_rows, 200),
      CHAIN_RO_CALL_GATE(get_table_by_scope, 200),
      CHAIN_RO_CALL_GATE(get_currency_balance, 200),
      CHAIN_RO_CALL_GATE(get_currency_stats, 200),
      CHAIN_RO_CALL_GATE(get_producers, 200),
      CHAIN_RO_CALL_GATE(get_producer_schedule, 200),
      CHAIN_RO_CALL_GATE(get_rate_schedule, 200),
      CHAIN_RO_CALL_GATE(get_scheduled_transactions, 200),
      CHAIN_RO_CALL_GATE(abi_json_to_bin, 200),
      CHAIN_RO_CALL_GATE(abi_bin_to_json, 200),
      CHAIN_RO_CALL_GATE(get_required_keys, 200),
      CHAIN_RO_CALL_GATE(get_transaction_id, 200),
      CHAIN_RW_CALL_ASYNC_GATE(push_block, chain_apis::read_write::push_block_results, 202),
      CHAIN_RW_CALL_ASYNC_GATE(push_transaction, chain_apis::read_write::push_transaction_results, 202),
      CHAIN_RW_CALL_ASYNC_GATE(push_transactions, chain_apis::read_write::push_transactions_results, 202),
      CHAIN_RW_CALL_ASYNC_GATE(send_transaction, chain_apis::read_write::send_transaction_results, 202)
   });
}

void chain_api_plugin::plugin_shutdown() {}

}
