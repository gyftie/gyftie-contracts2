#ifndef BADGE_H
#define BADGE_H

#include <iterator>
#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/asset.hpp>

#include "common.hpp"

using std::string;
using std::vector;
using std::iterator;
using namespace eosio;

class BadgeClass
{

  private:
    name contract;

  public:
    struct [[ eosio::table, eosio::contract("gyftietoken") ]] Badge
    {
        uint64_t    badge_id        =   0;
        string      badge_name      ;
        string      description     =   0;
        asset       reward          = asset { 0, common::S_GFT };
        string      profile_image   ;
        string      badge_image     ;
        string      mat_icon_name   ;
      //  name        issuer          ;

        uint64_t    primary_key()   const { return badge_id; }
        uint64_t    by_reward()     const { return reward.amount; }
    };

    struct [[ eosio::table, eosio::contract("gyftietoken") ]] BadgeAccount
    {
        uint64_t    badgeacct_id        = 0;
        uint64_t    badge_id            = 0;
        name        badge_holder        ;
        string      notes               ;
        asset       reward              = asset { 0, common::S_GFT };
        uint32_t    badge_receipt_date  = current_block_time().to_time_point().sec_since_epoch();

        uint64_t    primary_key()       const { return badgeacct_id; }
        uint64_t    by_holder()         const { return badge_holder.value; }
        uint64_t    by_receipt()        const { return badge_receipt_date; }
    };

    typedef eosio::multi_index<"badges"_n, Badge,
      indexed_by<"byreward"_n,
        const_mem_fun<Badge, uint64_t, &Badge::by_reward>>>
      badge_table;

    typedef eosio::multi_index<"badgeaccts"_n, BadgeAccount,
        indexed_by<"byholder"_n,
            const_mem_fun<BadgeAccount, uint64_t, &BadgeAccount::by_holder>>,
        indexed_by<"byreceipt"_n, 
            const_mem_fun<BadgeAccount, uint64_t, &BadgeAccount::by_receipt>>
    > badgeaccount_table;

    badge_table         badge_t;
    badgeaccount_table  badgeaccount_t;

    BadgeClass (const name& contract) : 
        badge_t         (contract, contract.value),
        badgeaccount_t  (contract, contract.value),
        contract        (contract) {}
    
    void add_badge (const string& badge_name, 
                    const string& description, 
                    const asset& reward, 
                    const string& profile_image, 
                    const string& badge_image,
                    const string& mat_icon_name,
                    const name& issuer) {

        badge_t.emplace (contract, [&](auto &b) {
            b.badge_id      = badge_t.available_primary_key();
            b.badge_name    = badge_name;
            b.description   = description;
            b.reward        = reward;
            b.badge_image   = badge_image;
            b.mat_icon_name = mat_icon_name;
        });
    }

    void reward_badge (const name&          badge_recipient,
                        const uint64_t&     badge_id,
                        const string&       notes) {

        auto b_itr = badge_t.find (badge_id);
        check (b_itr != badge_t.end(), "Badge ID does not exist.");

       // DEPLOY
       // require_auth (b_itr->issuer);

        auto byholder = badgeaccount_t.get_index<"byholder"_n>();
        auto ba_itr = byholder.lower_bound (badge_recipient.value);
        while (ba_itr->badge_holder == badge_recipient && ba_itr != byholder.end()) {
            check (ba_itr->badge_id != badge_id, "Recipient has already received this badge. Recipient: " + 
                badge_recipient.to_string() + "; Badge ID: " + std::to_string(badge_id));
            ba_itr++;
        }

        badgeaccount_t.emplace (contract, [&](auto &ba) {
            ba.badgeacct_id     = badgeaccount_t.available_primary_key();
            ba.badge_id         = badge_id;
            ba.badge_holder     = badge_recipient;
            ba.notes            = notes;
            ba.reward           = b_itr->reward;
        });

        if (b_itr->reward.amount > 0) {
            action (
                permission_level{contract, "owner"_n},
                contract, "issuetostake"_n,
                std::make_tuple(badge_recipient, b_itr->reward, notes))
            .send();
        }
    }

    // for testing
    void unreward_badge (const name& badge_recipient,
                            const uint64_t& badge_id) {
        auto b_itr = badge_t.find (badge_id);
        check (b_itr != badge_t.end(), "Badge ID does not exist.");

        // DEPLOY
        // check (has_auth (contract) || has_auth(b_itr->issuer), 
        //     "Permission denied. Must have approval from contract or issuer: " + b_itr->issuer.to_string());

        auto byholder = badgeaccount_t.get_index<"byholder"_n>();
        auto ba_itr = byholder.lower_bound (badge_recipient.value);
        while (ba_itr->badge_holder == badge_recipient && ba_itr != byholder.end()) {
            
            if (ba_itr->badge_id == badge_id) {
                byholder.erase (ba_itr);
                return;
            }
            ba_itr++;
        }
    }

    void reset ()  {
        auto b_itr = badge_t.begin();
        while (b_itr != badge_t.end()) {
            b_itr = badge_t.erase (b_itr);
        }

        auto ba_itr = badgeaccount_t.begin();
        while (ba_itr != badgeaccount_t.end()) {
            ba_itr = badgeaccount_t.erase (ba_itr);
        }
    }
};

#endif