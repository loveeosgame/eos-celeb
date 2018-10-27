/**
 *  @dev minakokojima, yukiexe
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>
// #include <cmath>
// #include <string>

#include "config.hpp"
#include "utils.hpp"
// #include "eosio.token.hpp"

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

typedef double real_type;
typedef uint8_t card;

using eosio::action;
using eosio::asset;
using eosio::permission_level;
using eosio::symbol_name;
using eosio::symbol_type;
using std::string;

class eoscrazytown : public eosio::contract
{
  public:
    eoscrazytown(account_name self) : contract(self),
                                      _bagsglobal(_self, _self),
                                      bags(_self, _self) {}

    // @abi action
    void init();
    // @abi action
    void clear();

    // @abi action
    void transfer(account_name from,
                  account_name to,
                  asset quantity,
                  string memo);

    void onTransfer(account_name &from,
                    account_name &to,
                    asset &quantity,
                    string &memo);

    // @abi table bagsglobal
    struct bagsglobal
    {
        uint64_t team;
        uint64_t pool;
        account_name last;
        time st, ed;
    };
    typedef singleton<N(bagsglobal), bagsglobal> singleton_bagsglobal;
    singleton_bagsglobal _bagsglobal;

    // @abi action
    void newbag(account_name &from, asset &eos);

    // @abi action
    void setslogan(account_name &from, uint64_t id, string memo);

    // @abi table bag i64
    struct bag
    {
        uint64_t id;
        account_name owner;
        uint64_t price;
        string slogan;

        uint64_t primary_key() const { return id; }
        uint64_t next_price() const
        {
            return price * 1.35;
        }
    };
    typedef eosio::multi_index<N(bag), bag> bag_index;
    bag_index bags;

    void apply(account_name code, action_name action);

  private:
    const vector<int64_t> getBets(const string &s, const char &c);
    auto getBeton(const vector<int64_t> &v);
    const int64_t getTotalBets(const vector<int64_t> &v);

    auto checkBets(const asset &eos, const string &memo,
                   vector<int64_t> &vbets, int64_t &totalBets);

    auto getResult(const card &a, const card &b);
};

void eoscrazytown::apply(account_name code, action_name action)
{
    auto &thiscontract = *this;

    if (action == N(transfer))
    {
        if (code == N(eosio.token))
        {
            execute_action(&thiscontract, &eoscrazytown::onTransfer);
        }
        return;
    }

    if (code != _self)
        return;
    switch (action)
    {
        EOSIO_API(eoscrazytown, (init)(clear)(newbag)(setslogan));
    };
}

extern "C"
{
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        eoscrazytown p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}