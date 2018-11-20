#include "eoscrazytown.hpp"

// @abi action
void eoscrazytown::init()
{
    require_auth(_self);

    auto g = _bagsglobal.get_or_create(_self, bagsglobal{
                                                  .pool = 0, .team = 0, .last = N(gy2dgmztgqge), .st = now() + 60 * 60 * 11, .ed = now() + 60 * 60 * 11 + 60 * 60 * 24});
    _bagsglobal.set(g, _self);
}
// @abi action
void eoscrazytown::clear()
{
    require_auth(_self);
    _bagsglobal.remove();
}

void eoscrazytown::newbag(asset &eos)
{

    require_auth(_self);
    for (int i = 0; i < 57; ++i)
    {
        bags.emplace(_self, [&](auto &p) {
            p.id = bags.available_primary_key();
            p.owner = N(eosbocai3333);
            p.price = eos.amount;
        });
    }
}

void eoscrazytown::setslogan(account_name &from, uint64_t id, string memo)
{
    auto itr = bags.find(id);
    eosio_assert(itr != bags.end(), "no character exist");

    eosio_assert(from == itr->owner, "not the owner...");
    bags.modify(itr, from, [&](auto &t) {
        t.slogan = memo;
    });
}

bool isbot(uint8_t id, uint64_t amount, uint64_t checksum)
{
    if (id * amount + 1 == checksum)
    {
        return false;
    }
    return true;
}

// input
void eoscrazytown::onTransfer(account_name &from, account_name &to, asset &eos, string &memo)
{
    if (to != _self)
        return;

    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer...");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only EOS token is allowed");

    if (memo.substr(0, 3) == "buy")
    {
        memo.erase(0, 4);
        std::size_t s = memo.find(' ');
        if (s == string::npos)
        {
            s = memo.size();
        }
        auto id = string_to_price(memo.substr(0, s));
        memo.erase(0, s + 1);
        auto itr = bags.find(id);
        eosio_assert(itr != bags.end(), "no character exist");
        eosio_assert(eos.amount == itr->next_price(), "pls check amount");
        auto chceksum = string_to_price(memo.c_str());
        print("checksum:::", chceksum);

        if (isbot(id, eos.amount, chceksum))
        {
            return;
        }

        auto g = _bagsglobal.get_or_default();
        g.team += eos.amount * 1 / 100;  //1 %
        g.pool += eos.amount * 10 / 100; //10%
        g.last = from;
        g.ed = now() + 60 * 10;
        _bagsglobal.set(g, _self);

        auto delta = eos;
        delta.amount = eos.amount * 89 / 100; //89%
        action(                               // winner winner chicken dinner
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, itr->owner, delta,
                       std::string("next hodl")))
            .send();

        bags.modify(itr, 0, [&](auto &t) {
            t.owner = from;
            t.price = eos.amount;
        });

        return;
    }
}