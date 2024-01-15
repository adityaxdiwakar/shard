#include <stdint.h>
#include <string>
#include <memory>
#include <queue>
#include <unordered_map>
#include <set>

struct Order {
    enum Direction { BUY, SELL };

    std::string participant; // TODO: use user order id, not strig
    std::string symbol; // TODO: use symbol id
    uint32_t order_id;
    uint32_t price; // in number of pennies (or per underlying)
    uint32_t size;
    Direction dir;
    uint32_t ts;

    bool valid = true;

    Order(std::string participant, Direction dir, std::string symbol, uint32_t price, uint32_t size)
        : participant(participant)
        , symbol(symbol)
        , price(price)
        , size(size)
        , dir(dir)
    {}

    // cancel will invalidate the order, so that when the matching engine
    // attempts a match, it can determine if the first in position order
    // has been cancelled
    //
    // performance effect is limited to when executions against an empty
    // top level may happen (top level will be removed from book when
    // volume is at zero and an execution is attempted)
    void cancel() { valid = false; }
};

struct Level {
    std::queue<std::shared_ptr<Order>> orders;
    uint32_t volume;
    uint32_t price; // in number of pennies
    Order::Direction dir;

    Level(Order::Direction dir) : dir(dir) {}
    Level(std::shared_ptr<Order> order)
        : volume(0)
        , price(order->price)
        , dir(order->dir)
    {}

    void add_order(std::shared_ptr<Order> order);

    struct Cmp {
        bool cmp_ref(const Level& a, const Level& b) const {
            if (a.dir == Order::BUY) return a.price < b.price;
            else return a.price > b.price;
        }

        bool operator()(const std::shared_ptr<Level>& a, const std::shared_ptr<Level>& b) const {
            return cmp_ref(*a, *b);
        }
    };
};

// maps price levels -> level objects
using LevelMap = std::unordered_map<uint32_t, std::shared_ptr<Level>>;

// maps order ids -> order objects
using OrderMap = std::unordered_map<uint32_t, std::shared_ptr<Order>>;

// levels book
using LevelBook = std::set<std::shared_ptr<Level>, Level::Cmp>;

class Book { 
    Order::Direction dir; // 2 books will exist, bid and ask book

    void add_order(std::shared_ptr<Order> order);

    private:
        OrderMap order_map;
        LevelMap level_map;
        LevelBook levels;
};
