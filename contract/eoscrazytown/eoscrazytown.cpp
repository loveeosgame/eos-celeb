#include "eoscrazytown.hpp"

// @abi action
void eoscrazytown::init(const checksum256 &hash)
{
    require_auth(_self);

    auto g = _bagsglobal.get_or_create(_self, bagsglobal{
                                                  .pool = 0, .team = 0, .last = N(crazytown.bp), .st = now() + 60 * 60 * 11, .ed = now() + 60 * 60 * 11 + 60 * 60 * 24});
    _bagsglobal.set(g, _self);
}
// @abi action
void eoscrazytown::clear()
{
    require_auth(_self);
    _bagsglobal.remove();
}

void eoscrazytown::newbag(account_name &from, asset &eos)
{

    require_auth(_self);
    for (int i = 0; i < 10; ++i)
    {
        bags.emplace(from, [&](auto &p) {
            p.id = bags.available_primary_key();
            p.owner = from;
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

// input
void eoscrazytown::onTransfer(account_name &from, account_name &to, asset &eos, string &memo)
{
    if (to != _self)
        return;

    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer...");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only EOS token is allowed");
    eosio_assert(eos.amount > 0, "must buy a positive amount");

    if (memo.substr(0, 3) == "buy")
    {

        memo.erase(0, 4);
        std::size_t s = memo.find(' ');
        if (s == string::npos)
        {
            s = memo.size();
        }

        auto id = string_to_price(memo.substr(0, s));
        //  auto id = 0;
        memo.erase(0, s + 1);
        auto itr = bags.find(id);
        eosio_assert(itr != bags.end(), "no character exist");
        eosio_assert(eos.amount >= itr->next_price(), "no enough eos");
        asset d(eos.amount - itr->next_price(), EOS_SYMBOL);

        if (d.amount > 0)
        {
            action( // winner winner chicken dinner
                permission_level{_self, N(active)},
                TOKEN_CONTRACT, N(transfer),
                make_tuple(_self, from, d,
                           std::string("refund")))
                .send();
        }

        d.amount = itr->next_price() - itr->price;
        ;

        auto g = _bagsglobal.get_or_create(_self, bagsglobal{

                                                      .pool = 0, .team = 0});

        g.team += d.amount * 1 / 1000; //0.1 %
        g.pool += d.amount * 10 / 100; //10%
        g.last = from;
        g.ed = now() + 60 * 60 * 24;

        _bagsglobal.set(g, _self);

        auto delta = d;
        delta.amount = d.amount * 899 / 1000; //89.9

        if (delta.amount > 0)
        {
            action( // winner winner chicken dinner
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, itr->owner, delta,
                           std::string("next hodl")))
                .send();
        }

        bags.modify(itr, 0, [&](auto &t) {
            t.owner = from;
            t.price = eos.amount;
        });

        return;
    }
}