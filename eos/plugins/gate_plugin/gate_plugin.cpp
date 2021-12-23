/**\
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/gate_plugin/gate_plugin.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/thread_utils.hpp>
#include <eosio/gate_plugin/ec.hpp>

#include <fc/network/ip.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/crypto/openssl.hpp>
#include <fc/reflect/variant.hpp>

#include <fc/network/ip.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/crypto/openssl.hpp>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/iostreams/write.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>

#include <boost/uuid/uuid.hpp>           
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp> 

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/logger/stub.hpp>

#include <eosio/chain/config.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/transaction.hpp>

#include <thread>
#include <memory>
#include <regex>
#include <shared_mutex>
#include <chrono>

#include <eosio/chain_plugin/chain_plugin.hpp>

#include <eosio/gate_plugin/RRStorage.h>
#include <eosio/gate_plugin/RRWssConnection.h>

namespace bio = boost::iostreams;
using namespace boost::iostreams;

using namespace std;
using namespace eosio;
using namespace eosio::chain;

namespace eosio {

   struct token_transfer 
   {
      name from;
      name to;
      asset quantity;
      string memo;

      static account_name get_account() 
      {
         return N(sig.token);
      }

      static action_name get_name() 
      {
         return N(transfer);
      }

   };

   static appbase::abstract_plugin& _gate_plugin = app().register_plugin<gate_plugin>();

   namespace asio = boost::asio;

   using std::map;
   using std::vector;
   using std::set;
   using std::string;
   using std::regex;
   using boost::asio::ip::tcp;
   using boost::asio::ip::address_v4;
   using boost::asio::ip::address_v6;
   using std::shared_ptr;
   using websocketpp::connection_hdl;

   enum ws_ecdh_curve_t {
      SECP384R1,
      PRIME256V1
   };

   static gate_plugin_defaults current_gate_plugin_defaults;

   void gate_plugin::set_defaults(const gate_plugin_defaults config) {
      current_gate_plugin_defaults = config;
   }

   static bool verbose_http_errors = true;
         
   class gate_plugin_impl
   {
      public:  
         std::string child_node_wss_port;
         std::string child_node_wss_ip;
         std::string child_node_master_account_name;
         std::string child_node_master_account_key;

         std::string account_action_seq;

         info_service _info_service;
         external_actions_service _external_actions_service;

         int trx_counter;

         bool use_gate_plugin;

            void main_loop()
            {   
               if(!use_gate_plugin)
                  return;

               _wss_connection.connect(child_node_wss_ip, child_node_wss_port);   

               _wss_connection._session->_subscribers.push_back(&_info_service);
               _wss_connection._session->_info_service = &_info_service;
               _wss_connection._session->_subscribers.push_back(&_external_actions_service);
 
               fc::crypto::private_key priv_key = fc::crypto::private_key(child_node_master_account_key);  
               fc::crypto::private_key local_priv_key = fc::crypto::private_key(std::string("5KMszuSCPnKE4dTdKNJzEYyu15rYGR7X3iYSvbDLmBUQ6GvTBoJ")); 

               chain_plugin& cp = app().get_plugin<chain_plugin>();
               auto abi_serializer_max_time = cp.get_abi_serializer_max_time();

               std::this_thread::sleep_for(std::chrono::milliseconds(10000));
               while (true)
               {    
                  try
                  {    
                     //std::this_thread::sleep_for(std::chrono::seconds(60));  

                     std::cout << "DB trx_counter = " << RRStorage::GetInstance()->trx_counter  << std::endl;   

                                                     
                     _wss_connection.send_message( std::to_string(_info_service._hash), "/v1/chain/get_info", "");

                     std::this_thread::sleep_for(std::chrono::milliseconds(1000));  

                     while(!_wss_connection._session->_info_service->chain_id.size())
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                     std::cout << "{\"account_name\":\"siggateway\",\"pos\":\"" +_external_actions_service.account_action_seq+ "\",\"offset\":\"100\"}" << std::endl;
                     _wss_connection.send_message( std::to_string(_external_actions_service._hash), "/v1/history/get_actions", "{\"account_name\":\"siggateway\",\"pos\":\"" +_external_actions_service.account_action_seq+ "\",\"offset\":\"100\"}");
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
              
                     std::cout << "{\"account_name\":\"siggateway\",\"pos\":\"" +RRStorage::GetInstance()->account_action_seq+ "\",\"offset\":\"100\"}" << std::endl;
                     process_request( "/v1/history/get_actions", "{\"account_name\":\"siggateway\",\"pos\":\"" +RRStorage::GetInstance()->account_action_seq+ "\",\"offset\":\"100\"}", account_action_seq );
    
                     std::vector<action_ext> vData;
                     std::vector<action_ext> vCheck;
                     std::vector<action_ext> vLocal;
                     std::vector<action_ext> vLocalCheck;
                     RRStorage::GetInstance()->process_pending_trx(vData, vCheck, vLocal, vLocalCheck);

                     for( auto it = vData.begin(); it != vData.end(); it++ )
                     {
                        boost::uuids::uuid _uuid = boost::uuids::random_generator()();

                        std::string currency = (*it).quantity;
                        std::string hash = (*it).trx_id;

                        if(currency.find("SIG") == string::npos)
                           continue;

                        signed_transaction trx;
                        fc::variant pretty_trx = fc::mutable_variant_object()
                           ("actions", fc::variants({
                              fc::mutable_variant_object()
                                 ("account", "sig.token")
                                 ("name", "issue")
                                 ("authorization", fc::variants({
                                       fc::mutable_variant_object()
                                          ("actor", "eosio")
                                          ("permission", "active")
                                       }))
                                 ("data", fc::mutable_variant_object()
                                    ("to", (*it).from)
                                    ("quantity", currency)
                                    ("memo", hash)
                                 )
                           }))
                           ("fee", "0.0000 SIG");

                        // abi serializer resolver
                        auto abi_serializer_resolver = [&](const name& account) -> fc::optional<abi_serializer> {
                           eosio::chain_apis::read_only::get_abi_params abi_params;
                           abi_params.account_name = account;
                           auto abi_results = cp.get_read_only_api().get_abi(abi_params);

                           fc::optional<abi_serializer> abis;
                           abis.emplace( *abi_results.abi, abi_serializer_max_time );

                           return abis;
                        };
                           
                        abi_serializer::from_variant(pretty_trx, trx, abi_serializer_resolver, abi_serializer_max_time);

                        eosio::chain::chain_id_type _chain_id(_wss_connection._session->_info_service->chain_id);
                        eosio::chain::block_id_type _block_id(_wss_connection._session->_info_service->head_block_id);
                        fc::time_point _expiration = fc::time_point::from_iso_string(_wss_connection._session->_info_service->head_block_time);

                        trx.expiration = _expiration + fc::seconds(180);
                        trx.set_reference_block(_block_id);
                        trx.max_net_usage_words = 5000;                     
                        trx.sign(priv_key, _chain_id); 

                        auto packed_trx = eosio::chain::packed_transaction(trx, eosio::chain::packed_transaction::compression_type::zlib);
                        std::string block_str = json::to_pretty_string(packed_trx);
                     
                        std::cout << "!!!!!!!!!!!!!!!!!!! PROCESSING NEW ISSUE !!!!!!!!!!!!!!!!!!!" << std::endl;
                        _wss_connection.send_message( (*it).trx_id, "/v1/chain/push_transaction", block_str);  
                        RRStorage::GetInstance()->change_status((*it).trx_id, trx_status::sended, (*it).trx_id);  
                        trx_counter++;                  
                     }

                     for( auto it = vCheck.begin(); it != vCheck.end(); it++ )
                     {                   
                        std::stringstream ss;
                        ss << " {\"id\":\"";
                        ss << (*it).pending_trx_id;
                        ss << "\"}";
                        //_wss_connection.send_message( (*it).trx_id, "/v1/history/get_transaction", ss.str());     
                                       
                     }

                     for( auto it = vLocalCheck.begin(); it != vLocalCheck.end(); it++ )
                     {                   
                        std::stringstream ss;
                        ss << " {\"id\":\"";
                        ss << (*it).pending_trx_id;
                        ss << "\"}";
                        std::string tmp;
                        //process_request( "/v1/history/get_transaction", ss.str(), tmp);                    
                     }

                     std::cout << " remote trx to check count = " << vCheck.size() << std::endl;
                     std::cout << " local trx to check count = " << vLocalCheck.size() << std::endl;

                     for( auto it = vLocal.begin(); it != vLocal.end(); it++ )
                     {
                        std::cout << " LOCAL TRX PROCESS = " << (*it).type << " : " << (*it).from << " - " << (*it).to << " quantity = " << (*it).quantity << std::endl;

                        boost::uuids::uuid _uuid = boost::uuids::random_generator()();

                        std::string currency = (*it).quantity;
                        std::string hash = (*it).trx_id;

                        if(currency.find("SIG") == string::npos)
                           continue;

                        signed_transaction trx;
                        fc::variant pretty_trx = fc::mutable_variant_object()
                           ("actions", fc::variants({
                              fc::mutable_variant_object()
                                 ("account", "sig.token")
                                 ("name", "transfer")
                                 ("authorization", fc::variants({
                                       fc::mutable_variant_object()
                                          ("actor", GATE_ACCOUNT_NAME)
                                          ("permission", "active")
                                       }))
                                 ("data", fc::mutable_variant_object()
                                    ("from", GATE_ACCOUNT_NAME)
                                    ("to", (*it).from)
                                    ("quantity", currency)
                                    ("memo", hash)
                                 )
                           }))
                           ("fee", "0.0000 SIG");

                        // abi serializer resolver
                        auto abi_serializer_resolver = [&](const name& account) -> fc::optional<abi_serializer> {
                           eosio::chain_apis::read_only::get_abi_params abi_params;
                           abi_params.account_name = account;
                           auto abi_results = cp.get_read_only_api().get_abi(abi_params);

                           fc::optional<abi_serializer> abis;
                           abis.emplace( *abi_results.abi, abi_serializer_max_time );

                           return abis;
                        };

                        controller& cc = cp.chain();
                        abi_serializer::from_variant(pretty_trx, trx, abi_serializer_resolver, abi_serializer_max_time);

                        trx.expiration = cc.head_block_time() + fc::seconds(180);
                        trx.set_reference_block(cc.head_block_id());
                        trx.max_net_usage_words = 5000;
                        auto chainid = cp.get_chain_id();
                        trx.sign(local_priv_key, chainid);

                        auto packed_trx = eosio::chain::packed_transaction(trx, eosio::chain::packed_transaction::compression_type::zlib);
                        std::string block_str = json::to_pretty_string(packed_trx);

                        std::string data;
                        process_request( "/v1/chain/push_transaction", block_str, (*it).trx_id );     
                        std::cout << "!!!!!!!!!!!! PROCESSING NEW WITHDRAW = "  << std::endl;             
                        RRStorage::GetInstance()->change_status((*it).trx_id, trx_status::sended, (*it).trx_id);  

                        //trx_counter++;                                         
                     }                                                   
                  }
                  catch( const fc::exception& e)
                  {
                     std::cout << e.what() << std::endl;
                  }
               }
            } 

            std::string encode64(const std::string& s) 
            {
               std::string base64_padding[] = {"", "==","="};
               namespace bai = boost::archive::iterators;

               std::stringstream os;
               typedef bai::base64_from_binary
               <bai::transform_width<const char *, 6, 8> > base64_enc;

               std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
                           std::ostream_iterator<char>(os));

               os << base64_padding[s.size() % 3];
               return os.str();
            }

            std::string decode64(const std::string& s) 
            {
               std::string base64_padding[] = {"", "==","="};
               namespace bai = boost::archive::iterators;

               std::stringstream os;

               typedef bai::transform_width<bai::binary_from_base64<const char *>, 8, 6> base64_dec;

               unsigned int size = s.size();
               if (size && s[size - 1] == '=') {
                  --size;
                  if (size && s[size - 1] == '=') --size;
               }
               if (size == 0) return std::string();

               std::copy(base64_dec(s.data()), base64_dec(s.data() + size),
                           std::ostream_iterator<char>(os));

               return os.str();
            }

            std::string string_compress_encode(const std::string &data)
            {
               std::stringstream compressed;
               std::stringstream original;
               original << data;
               boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
               out.push(boost::iostreams::zlib_compressor());
               out.push(original);
               boost::iostreams::copy(out, compressed);
               return encode64( compressed.str( ) );
            }

            std::string string_decompress_decode(const std::string &data)
            {
               std::stringstream compressed_encoded;
               std::stringstream decompressed;
               compressed_encoded << data;
               std::stringstream compressed(decode64(compressed_encoded.str()));
               boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
               in.push(boost::iostreams::zlib_decompressor());
               in.push(compressed);
               boost::iostreams::copy(in, decompressed);
               return decompressed.str();
            }

            void decompress(const std::vector<char> &compressed,
                                  std::vector<char> &decompressed)
            {
               boost::iostreams::filtering_ostream os;

               os.push(boost::iostreams::zlib_decompressor());
               os.push(boost::iostreams::back_inserter(decompressed));

               boost::iostreams::write(os, &compressed[0], compressed.size());
            }

            std::string get_signature( const std::vector<uint8_t>& chain_id, const std::vector<uint8_t>& packed_trx, const std::vector<uint8_t>& private_key ) 
            {
               std::vector<uint8_t> data;
               std::vector<uint8_t> zeroes(32, 0);
               data.insert(data.end(), chain_id.begin(), chain_id.end());
               data.insert(data.end(), packed_trx.begin(), packed_trx.end());
               data.insert(data.end(), zeroes.begin(), zeroes.end());

               std::vector<uint8_t> hash = sha256_impl(data).get_hash();     

               secp256k1_impl secp256;
               std::vector<uint8_t> sign = secp256.sign_data_compact(private_key,hash);
               std::vector<uint8_t> check = sign;

               check.push_back(75);
               check.push_back(49); 

               std::vector<uint8_t> checksum = ::ripemd160(check);

               for( int i = 0; i < 4; i++ )
                  sign.push_back(checksum[i]);

               std::stringstream ss;
               ss << "SIG_K1_";
               ss << fc::to_base58(sign);
               return ss.str();
            }

            std::vector<uint8_t> HexToBytes(std::string hex) 
            {
               std::vector<uint8_t> bytes;
               for (unsigned int i = 0; i < hex.length(); i += 2) 
               {
                  std::string byteString = hex.substr(i, 2);
                  uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
                  bytes.push_back(byte);
               }
               return bytes;
            }

            std::vector<char> from_wif( const string& wif_key )
            {
               auto wif_bytes = chain::from_base58(wif_key);
               FC_ASSERT(wif_bytes.size() >= 5);
               auto key_bytes = vector<char>(wif_bytes.begin() + 1, wif_bytes.end() - 4);
               return key_bytes;
            }
          
     
         std::thread _main_loop_thread;

         map<string,url_handler>                     url_handlers;
         int const threads = 8;

         // The io_context is required for all I/O
         boost::asio::io_context ioc_ws{threads};
     
         // The SSL context is required, and holds certificates
         ssl::context ctx{ssl::context::sslv23};

         std::vector<std::thread> _v_threads;
         optional<eosio::chain::named_thread_pool>   thread_pool;

         RRWssConnection _wss_connection;

         gate_plugin_impl()
         {
            trx_counter = 0;
         };

         ~gate_plugin_impl()
         {
         };

         void setup(){
            ilog( "gate_plugin setup called!" );
            account_action_seq = "0";

            _main_loop_thread =  std::thread(&gate_plugin_impl::main_loop, this);
            _main_loop_thread.detach();
         }

         void stop()
         {

         }  

         void process_request( const std::string& resource, const std::string& body, std::string _session/*, std::shared_ptr<eosio::ws_plugin_impl::session> _session*/ )
         {
	      try{
               auto handler_itr = url_handlers.find( resource );
               if( handler_itr != url_handlers.end()) 
               {
                  app().post( appbase::priority::high,
                              [&ioc = thread_pool->get_executor(), handler_itr,
                               resource{std::move( resource )}, body{std::move( body )}, _session]() {
                     try {
                        handler_itr->second( resource, body,
                              [&ioc, _session]( int code, fc::variant response_body ) 
                              {
                                 boost::asio::post( ioc, [response_body{std::move( response_body )}, _session, code]() mutable {
                                 
                                 std::cout << " !!!!!!!!!!!!!!!!!!!!!!! INSIDE CODE = " << code << std::endl;

                                 if( code == 200)
                                 {
                                    std::stringstream ss;
                                    ss << fc::json::to_string( response_body );

                                    pt::ptree data;
                                    pt::read_json(ss, data); 
                              
                                    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, data.get_child("actions"))
                                    {
                                       assert(v.first.empty()); 
                                       boost::property_tree::ptree act = v.second.get_child("action_trace.act");
                                      // std::cout << act.get<std::string>("name")  << std::endl;
                                      // std::cout << act.get<std::string>("account")  << std::endl;
                                      // std::cout << "from = " << act.get<std::string>("data.from")  << std::endl;
                                      // std::cout << "to = " << act.get<std::string>("data.to")  << std::endl;


                                       // no one trx with to siggateway and from not eosio

                                       RRStorage::GetInstance()->account_action_seq = v.second.get<std::string>("account_action_seq");    

                                       if( act.get<std::string>("account") == CONTRACT_NAME && act.get<std::string>("name") == ACTION_NAME )
                                       {
                                             if( act.get<std::string>("data.to") ==  GATE_ACCOUNT_NAME && act.get<std::string>("data.from") != "eosio")
                                             {
                                                RRStorage::GetInstance()->insert(v.second.get<std::string>("action_trace.trx_id"),
                                                                                                "issue",
                                                                                                act.get<std::string>("data.from"),
                                                                                                act.get<std::string>("data.to"),
                                                                                                act.get<std::string>("data.quantity"),
                                                                                                act.get<std::string>("data.memo"));

                                                                                                                                         
                                             }
                                       }else
                                          std::cout << "wrong action gate" << std::endl;
                                    }
                                 }else if(code == 202)
                                 {
                                    std::stringstream ss;
                                    ss << fc::json::to_string( response_body );
                                    pt::ptree data; 
                                    pt::read_json(ss, data); 
                                    std::string processd_trx_id = data.get<std::string>("transaction_id");
                                    RRStorage::GetInstance()->change_status(_session, trx_status::sended, processd_trx_id);
                                    std::cout << "!!!!!!!!!!!!!!!!!!!!!! TRX WITH id = " << _session << " COMPLETED!!" << std::endl; 
                                 }else 
                                 {
                                    std::stringstream ss;
                                    ss << fc::json::to_string( response_body );
                                      std::cout << "ERROR INTERNAL PAYLOAD = " << ss.str() << std::endl;  
                                 }
                                 response_body.clear();
                              });

                           });          
                     } catch( ... ) 
                     {
                        std::cout << " wss plugin kakaya to huinya 1!" << std::endl;
			               //_session = ss.str();
			               std::cout << "try to process exception ok! " << std::endl;
                     }
		            });
               } 
               else 
               {
                  dlog( "404 - not found: ${ep}", ("ep", resource));
               }
            }catch( ... )
            {
                           std::cout << " !!!!!!!!!!!!!!!!!!!!!!!!!! wss plugin kakaya to huinya 2!" << std::endl;
               //_session = ss.str();
            }	   
         }

         static void handle_exception( std::string session, const int& error_code ) {
	         ilog( "handle excaption called!!!!" );
            string err = "Internal Service error, wss: ";
            std::stringstream ss;
	         ss << error_code; 
            ss << std::endl;
            try {
               try {
                  throw;
               } catch (const fc::exception& e) {
                  err += e.to_detail_string();
                  elog( "${e}", ("e", err));
                  error_results_gate results{websocketpp::http::status_code::internal_server_error,
                                       "Internal Service Error", error_results_gate::error_info_gate(e, verbose_http_errors )};
                  ss << fc::json::to_string( results );
                // session->post_answer(ss.str());
               } catch (const std::exception& e) {
                  err += e.what();
                  elog( "${e}", ("e", err));
                  error_results_gate results{websocketpp::http::status_code::internal_server_error,
                                       "Internal Service Error", error_results_gate::error_info_gate(fc::exception( FC_LOG_MESSAGE( error, e.what())), verbose_http_errors )};
                  ss <<  fc::json::to_string( results );
                 // session->post_answer(ss.str());
               } catch (...) {
                  err += "Unknown Exception";
                  error_results_gate results{websocketpp::http::status_code::internal_server_error,
                                          "Internal Service Error",
                                          error_results_gate::error_info_gate(fc::exception( FC_LOG_MESSAGE( error, "Unknown Exception" )), verbose_http_errors )};
                  ss << fc::json::to_string( results );
                //  session->post_answer(ss.str());
               }
            } catch (...) {
               ss << ( R"xxx({"message": "Internal Server Error"})xxx" );
             //  session->post_answer(ss.str());
               std::cerr << "Exception attempting to handle exception: " << err << std::endl;
            }
         }
      };

   gate_plugin::gate_plugin():my(new gate_plugin_impl()){
      ilog( "gate_plugin ctor called!" );
   }

   gate_plugin::~gate_plugin(){
      ilog( "gate_plugin dtor called!" );      
   }

   void gate_plugin::set_program_options(options_description&, options_description& cfg) {
      ilog( "gate_plugin set_program_options called!" );

      cfg.add_options()
         ( "child-node-wss-port", bpo::value<string>()->default_value( "11111" ), "The actual child-node-wss-port.");
      cfg.add_options()   
         ( "child-node-wss-ip", bpo::value<string>()->default_value( "127.0.0.1" ), "The actual child-node-wss-ip.");
      cfg.add_options()         
         ( "master-account-name", bpo::value<string>()->default_value( "account" ), "The actual child-node-master-account-name.");
      cfg.add_options()         
         ( "master-account-key", bpo::value<string>()->default_value( "111111111111111" ), "child-node-master-account-key.");
      cfg.add_options()         
         ( "use_gate_plugin", bpo::value<bool>()->default_value( false ), "use gate plugin.");
   }

   void gate_plugin::plugin_initialize(const variables_map& options) {
      ilog( "gate_plugin plugin_initialize called!" );
   
      try{
         my->child_node_wss_port = options.at( "child-node-wss-port" ).as<std::string>();
         my->child_node_wss_ip = options.at( "child-node-wss-ip" ).as<std::string>();
         my->child_node_master_account_name = options.at( "master-account-name" ).as<std::string>();
         my->child_node_master_account_key = options.at( "master-account-key" ).as<std::string>();
         my->use_gate_plugin = options.at( "use_gate_plugin" ).as<bool>();

         std::cout << " my->use_gate_plugin = " << my->use_gate_plugin << std::endl; 
         std::cout << " my->child_node_wss_port = " << my->child_node_wss_port << std::endl; 
         std::cout << " my->child_node_wss_ip = " << my->child_node_wss_ip << std::endl; 
         std::cout << " my->child_node_master_account_name = " << my->child_node_master_account_name << std::endl; 
         std::cout << " my->child_node_master_account_key = " << my->child_node_master_account_key << std::endl; 
         

      }catch( std::exception& e){
         ilog( "gate_plugin somthing wrong with configuration" );
      }

      my->setup();      
   }

   void gate_plugin::plugin_startup() {
       ilog( "gate_plugin plugin_startup called!" );

      my->thread_pool.emplace( "gate", my->threads * 4 );

      add_api({{
         std::string("/v1/node/get_supported_apis"),
         [&](string, string body, url_response_callback cb) mutable {
            try {
               if (body.empty()) body = "{}";
               auto result = (*this).get_supported_apis();
               cb(200, fc::variant(result));
            } catch (...) {
               handle_exception("node", "get_supported_apis", body, cb);
            }
         }
      }}); 
   }

   void gate_plugin::plugin_shutdown() {   

      my->stop();
      my.reset();

      ilog( "gate_plugin::plugin_shutdown()" );
   }

   void gate_plugin::add_handler(const string& url, const url_handler& handler) {       
      ilog( "add api url: ${c}", ("c",url) );
      my->url_handlers.insert(std::make_pair(url,handler));      
   };

   void gate_plugin::handle_exception( const char *api_name, const char *call_name, const string& body, url_response_callback cb ) {
      try {
         try {
            throw;
         } catch (chain::unknown_block_exception& e) {
            error_results_gate results{400, "Unknown Block", error_results_gate::error_info_gate(e, verbose_http_errors)};
            cb( 400, fc::variant( results ));
         } catch (chain::unsatisfied_authorization& e) {
            error_results_gate results{401, "UnAuthorized", error_results_gate::error_info_gate(e, verbose_http_errors)};
            cb( 401, fc::variant( results ));
         } catch (chain::tx_duplicate& e) {
            error_results_gate results{409, "Conflict", error_results_gate::error_info_gate(e, verbose_http_errors)};
            cb( 409, fc::variant( results ));
         } catch (fc::eof_exception& e) {
            error_results_gate results{422, "Unprocessable Entity", error_results_gate::error_info_gate(e, verbose_http_errors)};
            cb( 422, fc::variant( results ));
            ilog( "Unable to parse arguments to ${api}.${call}", ("api", api_name)( "call", call_name ));
            ilog("Bad arguments: ${args}", ("args", body));
         } catch (fc::exception& e) {
            error_results_gate results{500, "Internal Service Error", error_results_gate::error_info_gate(e, verbose_http_errors)};
            cb( 500, fc::variant( results ));
            if (e.code() != chain::greylist_net_usage_exceeded::code_value && e.code() != chain::greylist_cpu_usage_exceeded::code_value) {
               ilog( "FC Exception encountered while processing ${api}.${call}",
                     ("api", api_name)( "call", call_name ));
               ilog( "Exception Details: ${e}", ("e", e.to_detail_string()));
            }
         } catch (std::exception& e) {
            error_results_gate results{500, "Internal Service Error", error_results_gate::error_info_gate(fc::exception( FC_LOG_MESSAGE( error, e.what())), verbose_http_errors)};
            cb( 500, fc::variant( results ));
            ilog( "STD Exception encountered while processing ${api}.${call}",
                  ("api", api_name)( "call", call_name ));
            ilog( "Exception Details: ${e}", ("e", e.what()));
         } catch (...) {
            error_results_gate results{500, "Internal Service Error",
               error_results_gate::error_info_gate(fc::exception( FC_LOG_MESSAGE( error, "Unknown Exception" )), verbose_http_errors)};
            cb( 500, fc::variant( results ));
            ilog( "Unknown Exception encountered while processing ${api}.${call}",
                  ("api", api_name)( "call", call_name ));
         }
      } catch (...) {
         std::cerr << "Exception attempting to handle exception for " << api_name << "." << call_name << std::endl;
      }
   }

   bool gate_plugin::verbose_errors()const {
      return verbose_http_errors;
   }

   gate_plugin::get_supported_apis_result gate_plugin::get_supported_apis()const {
      get_supported_apis_result result;

      for (const auto& handler : my->url_handlers) {
         if (handler.first != "/v1/node/get_supported_apis")
            result.apis.emplace_back(handler.first);
      }

      return result;
   } 
}

FC_REFLECT( eosio::token_transfer                       , (from)(to)(quantity)(memo) )

