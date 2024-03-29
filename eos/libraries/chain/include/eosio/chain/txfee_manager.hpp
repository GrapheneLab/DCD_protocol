/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/chain/multi_index_includes.hpp>

namespace eosio { namespace chain {

   class controller;
   struct transaction;
   struct action;

   struct producers_rate_info {
      account_name    name;
      double          rate;
      time_point      rate_time;
   };

   struct fee_rate_info {
      asset           base_rate_asset;
      uint64_t        new_rate_period;
      uint64_t        out_of_date_time;
      double          prev_rate;
      double          cur_rate;
      time_point      cur_rate_time;

      std::vector<chain::producers_rate_info>    cur_rate_producers;
   };

   struct action_fee_info {
      asset core_fee;
      asset usd_fee;
      double cur_rate;
   };

   class txfee_manager {
      public:
         explicit txfee_manager();

         asset get_core_from_usd(const asset& usd) const;
         asset get_required_fee( const controller& ctl, const transaction& trx )const;
         asset get_required_fee( const controller& ctl, const action& act )const;
         asset get_required_fee( const controller& ctl, const account_name& account, const action_name& act )const;
         void  get_new_rate_params( const controller& ctl );

         action_fee_info get_action_fee(const controller& ctl, const account_name& account, const action_name& act )const;
         fee_rate_info get_fee_rate(const controller& ctl) const;
         asset get_base_rate_asset(const controller& ctl) const;

      private:
        inline void init_native_fee(const account_name &acc, const action_name &act, const asset &fee) {
           fee_map[std::make_pair(acc, act)] = fee;
        }

        inline asset get_native_fee(const account_name &acc, const action_name &act) const {
           const auto itr = fee_map.find(std::make_pair(acc, act));
           if( itr != fee_map.end() ){
               return get_core_from_usd(itr->second);
           }
           // no find
           return asset(0);
        }

        std::map<std::pair<account_name, action_name>, asset> fee_map;

        asset           base_rate_asset = asset::from_string("1.0000 USD");
        
        uint64_t        contract_new_rate_period = 10*60; // in seconds
        uint64_t        contract_out_of_date_time = 3*24*60*60; // in seconds
        double          contract_prev_rate;
        double          contract_cur_rate = 1.0;
        time_point      contract_cur_rate_time;
        std::vector<producers_rate_info>    contract_cur_rate_producers;
   };

   class fee_paramter {
      public:
            account_name name;
            asset        fee;
            fee_paramter(account_name name, asset fee) : name(name), fee(fee) {};
   };


   // action fee info in db, for action exec by user def code
   class action_fee_object : public chainbase::object<action_fee_object_type, action_fee_object> {
      OBJECT_CTOR(action_fee_object);

      id_type      id;
      account_name account;
      action_name  message_type;
      asset        fee;
   };

   struct by_action_name;
   struct by_contract_account;
   using action_fee_object_index = chainbase::shared_multi_index_container<
            action_fee_object,
            indexed_by<
                  ordered_unique<tag<by_id>,
                        BOOST_MULTI_INDEX_MEMBER(action_fee_object, action_fee_object::id_type, id)
                  >,
                  ordered_unique< tag<by_action_name>,
                        composite_key<action_fee_object,
                              BOOST_MULTI_INDEX_MEMBER(action_fee_object, account_name, account),
                              BOOST_MULTI_INDEX_MEMBER(action_fee_object, action_name, message_type)
                        >
                  >,
                  ordered_non_unique< tag<by_contract_account>,
                        BOOST_MULTI_INDEX_MEMBER(action_fee_object, account_name, account)
                  >
            >
      >;
} } /// namespace eosio::chain

FC_REFLECT(eosio::chain::fee_paramter, (name)(fee))
FC_REFLECT(eosio::chain::action_fee_object, (id)(account)(message_type)(fee))
FC_REFLECT(eosio::chain::producers_rate_info, (name)(rate)(rate_time))

CHAINBASE_SET_INDEX_TYPE(eosio::chain::action_fee_object, eosio::chain::action_fee_object_index)
