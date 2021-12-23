#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/chain/contract_types.hpp>

namespace eosio { namespace chain {

   class apply_context;

   /**
    * @defgroup native_action_handlers Native Action Handlers
    */
   ///@{
   void apply_sig_newaccount(apply_context&);
   void apply_sig_updateauth(apply_context&);
   void apply_sig_deleteauth(apply_context&);
   void apply_sig_linkauth(apply_context&);
   void apply_sig_unlinkauth(apply_context&);

   void apply_sig_setcode(apply_context&);
   void apply_sig_setabi(apply_context&);
   void apply_sig_setfee(apply_context&);

   void apply_sig_canceldelay(apply_context&);
   ///@}  end action handlers

} } /// namespace eosio::chain
