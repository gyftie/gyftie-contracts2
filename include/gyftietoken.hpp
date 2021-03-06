#ifndef GYFTIETOKEN_H
#define GYFTIETOKEN_H

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>

#include <algorithm> // std::find
#include <math.h>

#include "gyftie.hpp"
#include "gyft.hpp"
#include "common.hpp"
#include "lock.hpp"
#include "permit.hpp"
#include "proposal.hpp"
#include "badge.hpp"
#include "migration.hpp"
#include "lockchain.hpp"
#include "comment.hpp"
#include "profile.hpp"
#include "permissions.hpp"

using std::string;
using std::vector;
using std::tie;
using namespace eosio;

CONTRACT gyftietoken : public contract
{
    using contract::contract;

  public:

    //  TEMPORARY actions needed for deployments
    ACTION xferzj ();
    ACTION addekey (const name& account, const string& key); 
    // ACTION backupprofs (const name& profile);
    // ACTION restoreprofs (const name& profile);
    
    //   Admin Actions
    ACTION addsig (const name new_signatory);
    ACTION remsig (const name existing_signatory);
    ACTION pause();
    ACTION unpause();
    ACTION chgthrottle(const uint32_t throttle);
    ACTION setconfig(const name gftorderbook, const name gyftie_foundation, const name gyftieoracle);
    ACTION setusercnt (const uint32_t count);
    ACTION claim (const name account, const string claim_key);
    ACTION isstoskoracl (const name to, const asset quantity, const string memo);
    ACTION upgrade (const name& account);
    ACTION reset ();
    ACTION smsverify (const name& account); 

    //  Token Actions
    ACTION create();
    ACTION issue(const name to, const asset quantity, const string memo);
    ACTION issuetostake (const name to, const asset quantity, const string memo);
    ACTION transfer(const name from, const name to, const asset quantity, const string memo);
    ACTION xfertostake(const name from, const name to, const asset quantity, const string memo);
    ACTION requnstake (const name user, const asset quantity);
    ACTION unstaked2 (const name user, const asset quantity); 
    ACTION stake (const name account, const asset quantity);

    //  Badge Actions
    ACTION createbadge (const name& badge_id, 
                        const string& badge_name, 
                        const string& description, 
                        const asset& reward, 
                        const string& profile_image, 
                        const string& badge_image, 
                        const string& mat_icon_name, 
                        const name& issuer, 
                        uint8_t badge_type);

    ACTION issuebadge (const name& badge_recipient, const name& badge_id, const string& notes);
    ACTION unissuebadge (const name& badge_recipient, const name& badge_id);
    ACTION issueidemp(const name& badge_recipient, const name& badge_id, const string& notes);

    //   Profile and Reputation Actions
    ACTION addhash (const name idchecker, const name idholder, const string idhash, const string id_expiration);
    ACTION createprof (const name& account);
    ACTION setrank (const name account, const uint64_t rank);
    ACTION voteforuser (const name voter, const name profile);
    ACTION unvoteuser (const name voter, const name profile);
    ACTION sellnotify (const name& seller, const asset& amount);
    ACTION buynotify (const name& buyer, const asset& amount);
    ACTION accelunstake (const name& account);
    ACTION remprofprep (const name& account);
    ACTION removeprof (const name& account);
    ACTION verifyuser (const name& verifier, const name& account_to_verify);
    ACTION referuser (const name& referrer, const name& account_to_refer);

    //   Profile Challenge Actions
    ACTION nchallenge (const name challenger_account, const name challenged_account, const string notes);
    ACTION validate (const name validator, const name account, const string idhash, const string id_expiration);
    ACTION dchallenge (const name challenged_account);
    ACTION addcnote (const name scribe, const name challenged_account, const string note);

    //   Profile Locking Actions
    ACTION addlock (const name account_to_lock, const string note);
    ACTION addlockchain (const name account_to_lock, const string note );
    ACTION addlocknote (const name account_to_lock, const string note);
    ACTION unlockchain (const name account_to_unlock, const string note );
    ACTION unlock (const name account_to_unlock, const string note);

    //   Proposal Actions
    ACTION propose(const name& proposer, const string& proposal_name, const string& notes);
    ACTION proposetrx (ignore<name> proposer,
                        ignore<string> proposal_name,
                        ignore<string> notes,
                        ignore<transaction> trx);
    ACTION execproposal (const uint64_t& proposal_id, const name& executer);
    ACTION votefor(const name voter, const uint64_t proposal_id);
    ACTION voteagainst(const name voter, const uint64_t proposal_id);
    ACTION unvoteprop(const name voter, const uint64_t proposal_id);
    ACTION removeprop(const uint64_t proposal_id);

    // Comment Actions
    ACTION addcomment (const name& commenter, 
                        const name& comment_type, 
                        const uint64_t& proposal_id,    // -1 for no proposal
                        const name& profile,            // "noprofile"_n for no profile
                        const uint64_t& parent_id, 
                        const string& comment_text);

    ACTION remcomment (const uint64_t& comment_id);
    ACTION editcomment (const uint64_t& comment_id, const string& comment_text);

  private:
   
    ProfileClass profileClass = ProfileClass (get_self());
    GyftClass gyftClass = GyftClass (get_self());
    GyftieClass gyftieClass = GyftieClass (get_self());
    LockClass lockClass = LockClass (get_self());
    ProposalClass proposalClass = ProposalClass (get_self());
    BadgeClass badgeClass = BadgeClass (get_self());
    Migration migration = Migration (get_self());
    CommentClass commentClass = CommentClass (get_self());

    TABLE account
    {
        asset balance;
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };
    typedef eosio::multi_index<"accounts"_n, account> accounts;

    TABLE voter
    {
        name        votee;
        uint64_t    vote_power;
        uint64_t    primary_key() const { return votee.value; }
    };

    typedef eosio::multi_index<"voters"_n, voter> voter_table;

    TABLE votee 
    {
        name        voter;
        uint64_t    vote_power;
        uint64_t    primary_key() const { return voter.value; }
    };

    typedef eosio::multi_index<"votees"_n, votee> votee_table;

    TABLE challenge 
    {
        name            challenged_account;
        name            challenger_account;
        vector<string>  challenge_notes;
        asset           challenge_stake;
        uint32_t        challenged_time;
        uint64_t        primary_key () const { return challenged_account.value; }
        uint64_t        by_challenger() const { return challenger_account.value; }
    };
    typedef eosio::multi_index<"challenges"_n, challenge,
        indexed_by<"bychallenger"_n,
            const_mem_fun<challenge, uint64_t, &challenge::by_challenger>>
    > challenge_table;
  
    TABLE currency_stats
    {
        asset supply;
        symbol symbol;
        name issuer;
        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"stat"_n, currency_stats> stats;

    TABLE balance
    {
        asset funds;
        name token_contract;
        uint64_t primary_key() const { return funds.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"balances"_n, balance> balance_table;

    TABLE buyorder
    {
        uint64_t order_id;
        name buyer;
        asset price_per_gft;
        asset gft_amount;
        asset order_value;
        uint32_t created_date;
        uint64_t primary_key() const { return order_id; }
        uint64_t by_buyer() const { return buyer.value; }
        uint64_t by_price() const { return price_per_gft.amount; }
    };

    typedef eosio::multi_index<"buyorders"_n, buyorder,
                               indexed_by<"bybuyer"_n,
                                          const_mem_fun<buyorder, uint64_t, &buyorder::by_buyer>>,
                               indexed_by<"byprice"_n,
                                          const_mem_fun<buyorder, uint64_t, &buyorder::by_price>>>
        buyorder_table;

    uint64_t get_next_sender_id()
    {
        return gyftieClass.get_next_sender_id();
    }
    
    void paytoken(const name token_contract,
                  const name from,
                  const name to,
                  const asset token_amount,
                  const string memo)
    {
        print("---------- Payment -----------\n");
        print("Token Contract   : ", name{token_contract}, "\n");
        print("From             : ", name{from}, "\n");
        print("To               : ", name{to}, "\n");
        print("Amount           : ", token_amount, "\n");
        print("Memo             : ", memo, "\n");

        action(
            permission_level{from, "owner"_n},
            token_contract, "transfer"_n,
            std::make_tuple(from, to, token_amount, memo))
            .send();

        print("---------- End Payment -------\n");
    }


    void retire( const asset& quantity, const string& memo )
    {
        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        stats statstable( get_self(), sym.code().raw() );
        auto existing = statstable.find( sym.code().raw() );
        check( existing != statstable.end(), "token with symbol does not exist" );
        const auto& st = *existing;

        require_auth( st.issuer );
        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must retire positive quantity" );

        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

        statstable.modify( st, same_payer, [&]( auto& s ) {
            s.supply -= quantity;
        });

        sub_balance( st.issuer, quantity );
    }

    void unstake (name account, asset quantity) 
    {
        profileClass.unstake (account, quantity);
    }

    asset adjust_asset(asset original_asset, float adjustment)
    {
        return asset{static_cast<int64_t>(original_asset.amount * adjustment), original_asset.symbol};
    }

    bool is_tokenholder(name account)
    {
        accounts a_t(get_self(), account.value);
        auto a_itr = a_t.find(S_GFT.code().raw());
        if (a_itr == a_t.end())
        {
            return false;
        }

        asset zero_balance = asset{0, S_GFT};
        if (a_itr->balance <= zero_balance)
        {
            return false;
        }
        return true;
    }

    bool is_paused()
    {
        return gyftieClass.get_state().paused == PAUSED;
    }

    void increment_account_count()
    {
        gyftieClass.increment_account_count();
    }

    asset get_gft_amount(asset price_per_gft, asset eos_amount)
    {
        float gft_quantity = pow(10, S_GFT.precision()) * eos_amount.amount / price_per_gft.amount;
        return asset{static_cast<int64_t>(gft_quantity), S_GFT};
    }

    void defer_unstake (const name user, const asset quantity, const uint32_t delay) 
    {
        eosio::transaction out{};
        out.actions.emplace_back(permission_level{get_self(), "owner"_n}, 
            get_self(), "unstaked2"_n, 
            std::make_tuple(user, quantity));
        out.delay_sec = delay;
        out.send(current_block_time().to_time_point().sec_since_epoch() + user.value + delay, get_self());    
    }

    void sub_balance(const name owner, const asset value)
    {
        accounts from_acnts(_self, owner.value);

        const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
        eosio::check(from.balance.amount >= value.amount, "overdrawn balance");

        auto sym_name = value.symbol.code().raw();
        stats statstable(_self, sym_name);
        auto existing = statstable.find(sym_name);
        eosio::check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
        const auto &st = *existing;

        from_acnts.modify(from, get_self(), [&](auto &a) {
            a.balance -= value;
        });

        // update profile balances       
        check (profileClass.existsInV2(owner), "Cannot subtract from balance. Account " + owner.to_string() + " must upgrade profile to version 2.");
        auto p_itr = profileClass.profile2_t.find (owner.value);
        eosio::check (p_itr != profileClass.profile2_t.end(), "Cannot subtract from balance. Account " + owner.to_string() + " profile not found.");

        eosio::check (p_itr->gft_balance >= value, "overdrawn balance - GFT is staked");

        profileClass.profile2_t.modify (p_itr, get_self(), [&](auto &p) {
            p.gft_balance -= value;
        });
    }

    void add_balance(const name owner, const asset value, const name ram_payer)
    {
        accounts to_acnts(_self, owner.value);
        auto to = to_acnts.find(value.symbol.code().raw());
        if (to == to_acnts.end())
        {
            auto sym_name = value.symbol.code().raw();
            stats statstable(_self, sym_name);
            auto existing = statstable.find(sym_name);
            eosio::check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
            const auto &st = *existing;

            if (owner != st.issuer) {  // do not increment account count for issuer
                increment_account_count ();
            }
            to_acnts.emplace(ram_payer, [&](auto &a) {
                a.balance = value;
            });
        }
        else
        {
            to_acnts.modify(to, eosio::same_payer, [&](auto &a) {
                a.balance += value;
            });
        }

        check (profileClass.existsInV2(owner), "Cannot add to balance. Account " + owner.to_string() + " must upgrade profile to version 2.");

        auto p_itr = profileClass.profile2_t.find (owner.value);
        eosio::check (p_itr != profileClass.profile2_t.end(), "Cannot add to balance. Account " + owner.to_string() + " profile not found.");

        profileClass.profile2_t.modify (p_itr, get_self(), [&](auto &p) {
            p.gft_balance += value;
        });
    }
};


#endif