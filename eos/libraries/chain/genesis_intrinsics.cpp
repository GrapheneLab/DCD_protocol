#include <eosio/chain/genesis_intrinsics.hpp>

namespace eosio { namespace chain {

const std::vector<const char*> genesis_intrinsics = {
   "__ashrti3",
   "__lshlti3",
   "__lshrti3",
   "__ashlti3",
   "__divti3",
   "__udivti3",
   "__modti3",
   "__umodti3",
   "__multi3",
   "__addtf3",
   "__subtf3",
   "__multf3",
   "__divtf3",
   "__eqtf2",
   "__netf2",
   "__getf2",
   "__gttf2",
   "__lttf2",
   "__letf2",
   "__cmptf2",
   "__unordtf2",
   "__negtf2",
   "__floatsitf",
   "__floatunsitf",
   "__floatditf",
   "__floatunditf",
   "__floattidf",
   "__floatuntidf",
   "__floatsidf",
   "__extendsftf2",
   "__extenddftf2",
   "__fixtfti",
   "__fixtfdi",
   "__fixtfsi",
   "__fixunstfti",
   "__fixunstfdi",
   "__fixunstfsi",
   "__fixsfti",
   "__fixdfti",
   "__fixunssfti",
   "__fixunsdfti",
   "__trunctfdf2",
   "__trunctfsf2",
   "is_feature_active",
   "activate_feature",
   "get_resource_limits",
   "set_resource_limits",
   "set_proposed_producers",
   "set_proposed_rate",
   "get_blockchain_parameters_packed",
   "set_blockchain_parameters_packed",
   "is_privileged",
   "set_privileged",
   "get_active_producers",
   "db_idx64_store",
   "db_idx64_remove",
   "db_idx64_update",
   "db_idx64_find_primary",
   "db_idx64_find_secondary",
   "db_idx64_lowerbound",
   "db_idx64_upperbound",
   "db_idx64_end",
   "db_idx64_next",
   "db_idx64_previous",
   "db_idx128_store",
   "db_idx128_remove",
   "db_idx128_update",
   "db_idx128_find_primary",
   "db_idx128_find_secondary",
   "db_idx128_lowerbound",
   "db_idx128_upperbound",
   "db_idx128_end",
   "db_idx128_next",
   "db_idx128_previous",
   "db_idx256_store",
   "db_idx256_remove",
   "db_idx256_update",
   "db_idx256_find_primary",
   "db_idx256_find_secondary",
   "db_idx256_lowerbound",
   "db_idx256_upperbound",
   "db_idx256_end",
   "db_idx256_next",
   "db_idx256_previous",
   "db_idx_double_store",
   "db_idx_double_remove",
   "db_idx_double_update",
   "db_idx_double_find_primary",
   "db_idx_double_find_secondary",
   "db_idx_double_lowerbound",
   "db_idx_double_upperbound",
   "db_idx_double_end",
   "db_idx_double_next",
   "db_idx_double_previous",
   "db_idx_long_double_store",
   "db_idx_long_double_remove",
   "db_idx_long_double_update",
   "db_idx_long_double_find_primary",
   "db_idx_long_double_find_secondary",
   "db_idx_long_double_lowerbound",
   "db_idx_long_double_upperbound",
   "db_idx_long_double_end",
   "db_idx_long_double_next",
   "db_idx_long_double_previous",
   "db_store_i64",
   "db_update_i64",
   "db_remove_i64",
   "db_get_i64",
   "db_next_i64",
   "db_previous_i64",
   "db_find_i64",
   "db_lowerbound_i64",
   "db_upperbound_i64",
   "db_end_i64",
   "assert_recover_key",
   "recover_key",
   "assert_sha256",
   "assert_sha1",
   "assert_sha512",
   "assert_ripemd160",
   "sha1",
   "sha256",
   "sha512",
   "ripemd160",
   "check_transaction_authorization",
   "check_permission_authorization",
   "get_permission_last_used",
   "get_account_creation_time",
   "current_time",
   "publication_time",
   "abort",
   "eosio_assert",
   "eosio_assert_message",
   "eosio_assert_code",
   "eosio_exit",
   "read_action_data",
   "action_data_size",
   "current_receiver",
   "require_recipient",
   "require_auth",
   "require_auth2",
   "has_auth",
   "is_account",
   "prints",
   "prints_l",
   "printi",
   "printui",
   "printi128",
   "printui128",
   "printsf",
   "printdf",
   "printqf",
   "printn",
   "printhex",
   "read_transaction",
   "transaction_size",
   "expiration",
   "tapos_block_prefix",
   "tapos_block_num",
   "get_action",
   "send_inline",
   "send_context_free_inline",
   "send_deferred",
   "cancel_deferred",
   "get_context_free_data",
   "memcpy",
   "memmove",
   "memcmp",
   "memset"
};

} } // namespace eosio::chain
