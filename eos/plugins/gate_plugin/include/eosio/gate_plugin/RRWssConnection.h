#pragma once

#include <boost/asio/ssl.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
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
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/foreach.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <sstream>

#include <eosio/gate_plugin/RRStorage.h>
#include <eosio/gate_plugin/common.h>

#define CONTRACT_NAME "sig.token"
#define ACTION_NAME "introduce"
#define GATE_ACCOUNT_NAME "siggateway"

namespace ssl = boost::asio::ssl; 

namespace pt = boost::property_tree;


inline void load_root_certificates(ssl::context& ctx, boost::system::error_code& ec)
{
    std::string const cert =
                "-----BEGIN CERTIFICATE-----\n"
                "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
                "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
                "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
                "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
                "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
                "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
                "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
                "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
                "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
                "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
                "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
                "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
                "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
                "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
                "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
                "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
                "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
                "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
                "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
                "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
                "+OkuE6N36B9K\n"
                "-----END CERTIFICATE-----\n"
            "-----BEGIN CERTIFICATE-----\n"
            "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
            "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
            "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
            "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
            "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
            "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
            "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
            "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
            "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
            "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
            "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
            "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
            "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
            "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
            "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
            "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
            "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
            "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
            "UEVbkhd5qstF6qWK\n"
            "-----END CERTIFICATE-----\n";
                
            ctx.add_certificate_authority(
                boost::asio::buffer(cert.data(), cert.size()), ec);
            if(ec)
                return;
}

inline void load_root_certificates(ssl::context& ctx)
{
    boost::system::error_code ec;
    load_root_certificates(ctx, ec);
    if(ec)
        throw boost::system::system_error{ec};
}

using tcp = boost::asio::ip::tcp;               
namespace ssl = boost::asio::ssl;               
namespace websocket = boost::beast::websocket;  

inline std::string encode64(const std::string& s) 
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

inline std::string decode64(const std::string& s) 
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

inline std::string string_compress_encode(const std::string &data)
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

inline std::string string_decompress_decode(const std::string &data)
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

inline void fail(boost::system::error_code ec, char const* what)
{
    throw std::runtime_error(ec.message());
}

template <typename T>
class service : public IObserver<T>{
public:    
    std::string _service_id;
    size_t _hash;

    service(){        
    };

    virtual ~service(){        
    };

    virtual void HandleEvent( const T& inMessage ){
    };
};

class info_service : public service<std::string>{
public:
    std::string chain_id;
    std::string head_block_id;
    std::string head_block_time;
    std::string head_block_num;
    std::string last_irreversible_block_id;

    info_service(){
        _service_id = typeid(info_service).name();
        _hash = cx_hash(_service_id);
    }; 

    virtual ~info_service(){
    };

    virtual void HandleEvent( const std::string& inMessage ){
        std::stringstream ss;
        ss << inMessage;

        pt::ptree data; 
        pt::read_json(ss, data); 

        chain_id = data.get<std::string>("chain_id");
        head_block_num = data.get<std::string>("head_block_num");
        head_block_id = data.get<std::string>("head_block_id");
        head_block_time = data.get<std::string>("head_block_time");
        last_irreversible_block_id = data.get<std::string>("last_irreversible_block_id");     
    };
};

class external_actions_service : public service<std::string>{
public:
    std::string account_action_seq;
    external_actions_service(){       
        _service_id = typeid(external_actions_service).name();  
        _hash = cx_hash(_service_id);    
        account_action_seq = "0";        
    }; 

    virtual ~external_actions_service(){
    };

    virtual void HandleEvent( const std::string& inMessage ){
        std::stringstream ss;
        ss << inMessage;

        pt::ptree data;      
        pt::read_json(ss, data); 

        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, data.get_child("actions"))
        {
            assert(v.first.empty()); 
            boost::property_tree::ptree act = v.second.get_child("action_trace.act");

            if( act.get<std::string>("account") == "sig.token" && act.get<std::string>("name") == "withdraw" )
            {                
                RRStorage::GetInstance()->insert(v.second.get<std::string>("action_trace.trx_id"),
                                                                            act.get<std::string>("name"),
                                                                            act.get<std::string>("data.from"),
                                                                            "empty",
                                                                            act.get<std::string>("data.quantity"),
                                                                            "empty");
                
                                                                                                                                                                             
            }

            account_action_seq = v.second.get<std::string>("account_action_seq");  
        }
    };
};

class RRWssConnection;
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    websocket::stream<ssl::stream<tcp::socket>> ws_;
    boost::beast::multi_buffer buffer_;
    std::string host_;

public:
    RRWssConnection* _connection;
    std::vector<service<std::string>*> _subscribers;  
    info_service* _info_service;

    explicit session(boost::asio::io_context& ioc, ssl::context& ctx) : resolver_(ioc), ws_(ioc, ctx)
    {
    }

    bool is_opened()
    {
        return ws_.is_open();
    }

    void run( char const* host, char const* port, RRWssConnection* conn)
    {
        host_ = host;
        _connection = conn;

        resolver_.async_resolve(
            host,
            port,
            std::bind(
                &session::on_resolve,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void on_resolve(
        boost::system::error_code ec,
        tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        boost::asio::async_connect(
            ws_.next_layer().next_layer(),
            results.begin(),
            results.end(),
            std::bind(
                &session::on_connect,
                shared_from_this(),
                std::placeholders::_1));
    }

    void on_connect(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "connect");

        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            std::bind(
                &session::on_ssl_handshake,
                shared_from_this(),
                std::placeholders::_1));

        std::cout << "on_connect" << std::endl;                
    }

    void on_ssl_handshake(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "ssl_handshake");

        auto timeout = 30;
        boost::beast::websocket::stream_base::timeout opt{
            std::chrono::seconds(timeout), 
            std::chrono::seconds(timeout), 
            true};                        
        ws_.set_option(opt);

        ws_.async_handshake(host_, "/",
            std::bind(
                &session::on_handshake,
                shared_from_this(),
                std::placeholders::_1));

        std::cout << "on_ssl_handshake" << std::endl;  
    }

    void on_handshake(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");    

        std::cout << "on_handshake" << std::endl;    

        ws_.async_write(
            boost::asio::buffer("DDG"),
            std::bind(
                &session::on_write,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));              
    }

    void write(const std::string& data)
    {
        ws_.async_write(
            boost::asio::buffer(data),
            std::bind(
                &session::on_write,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        
        buffer_.clear();
        ws_.async_read(
            buffer_,
            std::bind(
                &session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

public:
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        try{
            std::stringstream tmp;           
            tmp << string_decompress_decode(boost::beast::buffers_to_string(buffer_.data()));
            buffer_.clear();

            std::string hash;
            std::getline(tmp, hash);

            std::string code;
            std::getline(tmp, code);

            std::string tmp_payload;
            std::string payload;
            while(std::getline(tmp, tmp_payload))
                payload.append(tmp_payload);

       
            if( code == "202")
            {
                std::stringstream ss;
                ss << payload;
                pt::ptree data; 
                pt::read_json(ss, data); 
                std::string processd_trx_id = data.get<std::string>("transaction_id");
                RRStorage::GetInstance()->change_status(hash, trx_status::sended, processd_trx_id);
                std::cout << "!!!!!!!!!!!!!!!!!!!!!! TRX WITH id = " << hash << " COMPLETED!!" << std::endl; 
            }else if( code == "200")  
            {
                std::stringstream ss;
                ss << hash;
                size_t h;
                ss >> h;
                for( auto it = _subscribers.begin(); it != _subscribers.end(); it++ )
                {
                    if( (*it)->_hash == h)
                    {
                        (*it)->HandleEvent(payload);
                    }
                }
            }else
            {
                std::cout << " SOMTHING WRONG!!! Code = " << code << " Payload = " << payload << std::endl;
            }
        }
        catch( std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }       
    }

    void process_data(const std::string& data);     

    void process_json(pt::ptree& data);    

    void close()
    {
        ws_.async_close(websocket::close_code::normal,
            std::bind(
                &session::on_close,
                shared_from_this(),
                std::placeholders::_1));
    }

    void on_close(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "close");
    }
};

namespace bio = boost::iostreams;
using namespace boost::iostreams;

class chain_one {};
class chain_two {};

class RRWssConnection
{
public:
    RRWssConnection(){
    };

    void connect( const std::string& ipaddr, const std::string& port){
        load_root_certificates(_ctx);
        try{
        _session = std::make_shared<session>(_ioc, _ctx);
        _session->run(ipaddr.c_str(), port.c_str(), this);

        _threads.emplace_back(
        [&]{
            _ioc.run();
        });

        while(!_session->is_opened())
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        catch( std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    void send_message( const std::string& hash, const std::string& url, const std::string& msg ){
        std::stringstream tmp;
        tmp << hash << std::endl << url << std::endl << msg;// << std::endl;
        _session->write(string_compress_encode(tmp.str()));
    }

    void on_incomming_message( const std::string& data ){
        std::stringstream tmp;
        tmp << string_decompress_decode(data);

        std::string hash;
        std::getline(tmp, hash);

        std::string tmp_payload;
        std::string payload;

        while(std::getline(tmp, tmp_payload))
            payload.append(tmp_payload);

        std::cout << " hash = " << hash << std::endl;
        std::cout << " paload = " << payload << std::endl;
    }

    ~RRWssConnection(){
        _session->close();
        _ioc.stop();
        _threads.clear();
    };  

private:
    boost::asio::io_context _ioc;
    ssl::context _ctx{ssl::context::sslv23_client};
    std::vector<std::thread> _threads;

public:
    std::shared_ptr<session> _session;  
};
