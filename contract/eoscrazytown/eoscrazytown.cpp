#include "eoscrazytown.hpp"

// @abi action
void eoscrazytown::init()
{
    require_auth(_self);

    auto g = _bagsglobal.get_or_create(_self, bagsglobal{
                                                  .pool = 0, .team = 0, .last = N(gy2dgmztgqge), .st = 1542715200, .ed = 1542715200 + 60 * 10});
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
        eosio_assert(eos.amount == itr->nextprice, "pls check amount");
        eosio_assert(from != itr->owner, "pls check owner");
        auto chceksum = string_to_price(memo.c_str());
        if (isbot(id, eos.amount, chceksum))
        {
            return;
        }

        auto checkout = eos.amount - itr->price; //拿到0.35的钱

        auto g = _bagsglobal.get_or_default();
        g.team += checkout * 5 / 1000; //0.5 %
        g.pool += checkout * 10 / 100; //10%
        g.last = from;
        g.ed = now() + 60 * 10;
        _bagsglobal.set(g, _self);

        auto delta = eos;
        delta.amount = checkout * 89 / 100 + itr->price; //89%+上一次的钱
        action(                                          // winner winner chicken dinner
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, itr->owner, delta,
                       std::string("next hodl")))
            .send();

        action( // 给bocai代币持有者分红
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, N(eosbocaidivi), asset(checkout * 5 / 1000, EOS_SYMBOL),
                       std::string("make_profit")))
            .send();

        bags.modify(itr, 0, [&](auto &t) {
            t.owner = from;
            t.price = eos.amount;
            t.nextprice = t.price * 1.35;
        });

        return;
    }
}