#pragma once

#include <boost/config.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <algorithm>

#include <eosio/gate_plugin/RRSingletone.h>

using boost::multi_index_container;
using namespace boost::multi_index;
namespace bip=boost::interprocess;

typedef bip::basic_string<
  char,std::char_traits<char>,
  bip::allocator<char,bip::managed_mapped_file::segment_manager>
> shared_string;

struct action
{
  shared_string trx_id;  
  shared_string type;  
  shared_string from;  
  shared_string to;  
  shared_string quantity;  
  shared_string memo;  
  shared_string pending_trx_id; 
  int status;

  action(const shared_string::allocator_type& al):
    trx_id(al), type(al), from(al), to(al), quantity(al), memo(al), pending_trx_id(al), status(0)
  {}

  friend std::ostream& operator<<(std::ostream& os, const action& b)
  {
    os << b.trx_id<<":"<<b.from<<":"<<b.to<<":"<<b.quantity<<":"<<b.memo;
    return os;
  }

  friend bool operator < (const action& lhs, const action& rhs)
  { 
    return lhs.trx_id < rhs.trx_id;
  }
};

enum trx_status
{
  not_sended = 0,
  sended,
  accepted
};

struct action_ext
{
  std::string trx_id; 
  std::string type;   
  std::string from;  
  std::string to;  
  std::string quantity;  
  std::string memo;  
  std::string pending_trx_id; 
  int status;                                          
};

struct partial_string 
{
  partial_string(const shared_string& str):str(str){}
  shared_string str;
};

struct partial_str_less
{
  bool operator()(const shared_string& x,const shared_string& y)const
  {
    return x<y;
  }

  bool operator()(const shared_string& x,const partial_string& y)const
  {
    return x.substr(0,y.str.size())<y.str;
  }

  bool operator()(const partial_string& x,const shared_string& y)const
  {
    return x.str<y.substr(0,x.str.size());
  }
};

typedef multi_index_container<
  action,
  indexed_by<
    ordered_non_unique<
      BOOST_MULTI_INDEX_MEMBER(action,shared_string,trx_id)
    >
  >,
  bip::allocator<action,bip::managed_mapped_file::segment_manager>
> action_container;

#define STORAGE_NAME "actions.db"
#define STORAGE_SIZE 655350000

class RRStorage : public RRingleton<RRStorage>
{
private:
    friend class RRingleton<RRStorage>;

    bip::managed_mapped_file _mmf;
    bip::named_mutex _bip_mutex;

    action_container* _pending_container;
    std::vector<std::thread> _threads;



    RRStorage() : _mmf(bip::open_or_create,STORAGE_NAME,STORAGE_SIZE), _bip_mutex(bip::open_or_create,"7FD6D7E8-320B-11DC-82CF-F0B655D89593"){   
      _bip_mutex.unlock();

        account_action_seq = "0";
        trx_counter = 0;
        _pending_container=_mmf.find_or_construct<action_container>("_pending_container")(
        action_container::ctor_args_list(),
        action_container::allocator_type(_mmf.get_segment_manager()));    
    }

    ~RRStorage(){        
    }

public:
    std::string account_action_seq;
    int trx_counter;
    bool trx_exist(const std::string& trx_id)
    {
        typedef nth_index<action_container,0>::type index_by_action;
        index_by_action&          idx = get<0>(*_pending_container);
        index_by_action::iterator it;

        for(auto it = idx.begin(); it != idx.end(); it++){
          if((*it).trx_id.compare(trx_id.c_str()) == 0)
            return true; 
        }  
        return false;
    }

    void insert( const std::string& trx_id,
                  const std::string& type,
                  const std::string& from,
                  const std::string& to,
                  const std::string& quantity,
                  const std::string& memo)
    {
      bip::scoped_lock<bip::named_mutex> lock(_bip_mutex);
      if( !trx_exist(trx_id.c_str()) )
      {    

        trx_counter++;
        std::cout << "!!!!!!!!!!!!!!!! adding new trx - " << trx_id << std::endl;
        action b(shared_string::allocator_type(_mmf.get_segment_manager()));
        b.trx_id = trx_id.c_str();
        b.type = type.c_str();
        b.from = from.c_str();
        b.to = to.c_str();
        b.quantity = quantity.c_str();
        b.memo = memo.c_str();
        b.status = trx_status::not_sended;
        b.pending_trx_id = "";

        _pending_container->insert(b);
      }      
    }

    void process_pending_trx(std::vector<action_ext>& vData, std::vector<action_ext>& vDataCheck, std::vector<action_ext>& vLocalTransactions, std::vector<action_ext>& vLocalCheck)
    {
        bip::scoped_lock<bip::named_mutex> lock(_bip_mutex);

        typedef nth_index<action_container,0>::type index_by_action;
        index_by_action&          idx = get<0>(*_pending_container);
        index_by_action::iterator it;
        action_ext a; 

        for(auto it = idx.begin(); it != idx.end(); it++)
        {
          if((*it).type == "issue")
          {
            switch((*it).status)
            {
              case trx_status::not_sended:
              
                a = {(*it).trx_id.c_str(),
                                            (*it).type.c_str(),
                                            (*it).from.c_str(),
                                            (*it).to.c_str(),
                                            (*it).quantity.c_str(),
                                            (*it).memo.c_str(),
                                            (*it).pending_trx_id.c_str(),
                                            (*it).status};
                vData.push_back(a);                            
              break;

              case trx_status::sended:
              
                a = {(*it).trx_id.c_str(),
                                            (*it).type.c_str(),
                                            (*it).from.c_str(),
                                            (*it).to.c_str(),
                                            (*it).quantity.c_str(),
                                            (*it).memo.c_str(),
                                            (*it).pending_trx_id.c_str(),
                                            (*it).status};
                      vDataCheck.push_back(a);                      
              break;

              case trx_status::accepted:
              std::cout <<" INTERNAL TRX = " << (*it).type.c_str() << " WAS COMPLETE" << std::endl;
              break;
            }
          }else if((*it).type == "withdraw")
          {
            switch((*it).status)
            {
              case trx_status::not_sended:
              
                a = {(*it).trx_id.c_str(),
                                            (*it).type.c_str(),
                                            (*it).from.c_str(),
                                            (*it).to.c_str(),
                                            (*it).quantity.c_str(),
                                            (*it).memo.c_str(),
                                            (*it).pending_trx_id.c_str(),
                                            (*it).status};
                vLocalTransactions.push_back(a);                                         
              break;

              case trx_status::sended:
              
                a = {(*it).trx_id.c_str(),
                                            (*it).type.c_str(),
                                            (*it).from.c_str(),
                                            (*it).to.c_str(),
                                            (*it).quantity.c_str(),
                                            (*it).memo.c_str(),
                                            (*it).pending_trx_id.c_str(),
                                            (*it).status};
                      vLocalCheck.push_back(a);                                                               
              break;

              case trx_status::accepted:
              std::cout << " EXTERNAL TRX = " << (*it).type.c_str() << " WAS COMPLETE" << std::endl;
              break;
            }           
          }else
          {
            std::cout << " WRONG ACTIONS " << (*it).type << std::endl;
          }        
        }     
    }

    void change_status(const std::string& trx_id, const trx_status& status, const std::string& pending_trx_id = NULL)
    {
        bip::scoped_lock<bip::named_mutex> lock(_bip_mutex);

        typedef nth_index<action_container,0>::type index_by_action;
        index_by_action&          idx = get<0>(*_pending_container);
        index_by_action::iterator it;

        for(auto it = idx.begin(); it != idx.end(); it++)
        {
          if(trx_id.compare((*it).trx_id.c_str()) == 0)
          {
            if( (*it).status != status )
              idx.modify(it, [&](auto& p){p.status = status;});
              idx.modify(it, [&](auto& p){p.pending_trx_id = pending_trx_id.c_str();});
          }
        }
    }
};