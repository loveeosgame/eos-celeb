#include "eoscrazytown.hpp"

// @abi action
void eoscrazytown::init()
{
    require_auth(_self);

    auto g = _bagsglobal.get_or_create(_self);
    g.pool = 0;
    g.team = 0;
    g.last = N(gy2dgmztgqge);
    g.st = now();
    g.ed = now() + 60;
    _bagsglobal.set(g, _self);
}
// @abi action
void eoscrazytown::clear()
{
    require_auth(_self);
    for (int i = 0; i < 57; ++i)
    {
        bags.emplace(_self, [&](auto &p) {
            p.id = bags.available_primary_key();
            p.owner = N(gy2dgmztgqge);
            p.price = 1000;
            p.nextprice = p.price * 1.35;
        });
    }
}

void eoscrazytown::newbag(asset &eos, uint8_t &st)
{
    require_auth(_self);
    for (int i = 0; i < st + 57; ++i)
    {
        auto b = bags.find(i);
        bags.modify(b, 0, [&](auto &p) {
            p.owner = N(gy2dgmztgqge);
            p.price = eos.amount;
            p.nextprice = p.price * 1.35;
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
    if (id * amount + 2333 == checksum)
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
        eosio_assert(eos.amount == itr->nextprice, "pls check amount");
        eosio_assert(from != itr->owner, "pls check owner");
        auto chceksum = string_to_price(memo.c_str());

        auto checkout = eos.amount - itr->price; //拿到0.35的钱

        auto g = _bagsglobal.get_or_default();
        eosio_assert(now() > g.st, "game not start");
        eosio_assert(now() < g.ed, "game over");
        if (isbot(id, eos.amount, chceksum))
        {
            return;
        }
        g.team += checkout * 1 / 100;  //1 %
        g.pool += checkout * 10 / 100; //10%
        g.last = from;
        g.ed = now() + 60 * 1;
        _bagsglobal.set(g, _self);

        auto delta = eos;
        delta.amount = checkout * 89 / 100 + itr->price; //89%+上一次的钱
        action(                                          // winner winner chicken dinner
            permission_level{_self, N(active)},
            EOS_CONTRACT, N(transfer),
            make_tuple(_self, itr->owner, delta,
                       std::string("next hodl")))
            .send();

        bags.modify(itr, 0, [&](auto &t) {
            t.owner = from;
            t.price = eos.amount;
            t.nextprice = t.price * 1.35;
        });

        return;
    }
}