#ifndef GYFT_H
#define GYFT_H

#include <string>

#include "common.hpp"
#include "profile.hpp"
#include "gyftie.hpp"

using std::vector;
using std::string;

using namespace common;

class GyftClass {

    public:

        struct [[ eosio::table, eosio::contract("gyftietoken") ]] Gyft
        {
            uint64_t    gyft_id;
            name        gyfter;
            name        gyftee;
            asset       gyfter_issue;
            asset       gyftee_issue;
            string      relationship;
            string      notes;
            uint32_t    gyft_date;
            uint16_t    likes;
            uint64_t    primary_key() const { return gyft_id; }
            uint64_t    by_gyfter() const { return gyfter.value; }
            uint64_t    by_gyftee() const { return gyftee.value; }
            uint64_t    by_gyftdate() const { return gyft_date; }
        };

        typedef eosio::multi_index<"gyfts"_n, Gyft,
            indexed_by<"bygyfter"_n,
                const_mem_fun<Gyft, uint64_t, &Gyft::by_gyfter>>,
            indexed_by<"bygyftee"_n,
                const_mem_fun<Gyft, uint64_t, &Gyft::by_gyftee>>,
            indexed_by<"bygyftdate"_n, 
                const_mem_fun<Gyft, uint64_t, &Gyft::by_gyftdate>>
            >
        gyft_table;

        name            contract;
        gyft_table      gyft_t;
        GyftieClass     gyftieClass;

        GyftClass (const name& contract) 
            : gyft_t (contract, contract.value), 
            gyftieClass (contract),
            contract (contract) {}

        void create (
            const name& gyfter, 
            const name& gyftee, 
            const asset& gyfter_issue,
            const asset& gyftee_issue, 
            const string& relationship) {
                        
            gyft_t.emplace(contract, [&](auto &g) {
                g.gyft_id       = gyft_t.available_primary_key();
                g.gyfter        = gyfter;
                g.gyftee        = gyftee;
                g.gyfter_issue  = gyfter_issue;
                g.gyftee_issue  = gyftee_issue;
                g.relationship  = relationship;
                g.gyft_date     = current_block_time().to_time_point().sec_since_epoch();
            });
        }

        void throttle_check () {
            uint32_t throttle = gyftieClass.get_state().throttle;
            
            if (throttle < gyftieClass.get_state().account_count && throttle > 0) {
                auto gyftdate_index = gyft_t.get_index<"bygyftdate"_n>();
                auto gyftdate_itr = gyftdate_index.rbegin();

                for (int i=0; i< throttle; i++) {
                    gyftdate_itr++;
                }

                uint32_t throttled_gyfts_ago = gyftdate_itr->gyft_date;
                uint32_t throttled_time_period = 60 * 60 * 24; // 24 hours
                check (  throttled_gyfts_ago < 
                                    current_block_time().to_time_point().sec_since_epoch() - 
                                    throttled_time_period, 
                                "Gyfts are throttled. Please wait a few hours and try again.");
            }
        }
};

#endif