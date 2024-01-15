#include <unordered_map>
#include <stdint.h>
#include <string>
#include <memory>
#include <queue> 
#include <set>
    
enum class Direction { BUY, SELL };

template<Direction dir>
struct Order {

    std::string participant; // TODO: use user order id, not strig
    std::string symbol; // TODO: use symbol id
    uint32_t order_id;
    uint32_t price; // in number of pennies (or per underlying)
    uint32_t size;
    uint32_t ts;

    bool valid = true;

    Order(std::string participant, std::string symbol, uint32_t price, uint32_t size)
        : participant(participant)
        , symbol(symbol)
        , price(price)
        , size(size)
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

template<Direction dir>
struct Level {
    std::queue<std::shared_ptr<Order<dir>>> orders;
    uint32_t volume;
    uint32_t price; // in number of pennies

    Level(std::shared_ptr<Order<dir>> order)
        : volume(0)
        , price(order->price)
    {}

    void add_order(std::shared_ptr<Order<dir>> order);

    struct Cmp {
        bool cmp_ref(const Level& a, const Level& b) const {
            if constexpr (dir == Direction::BUY) return a.price < b.price;
            else return a.price > b.price;
        }

        bool operator()(const std::shared_ptr<Level<dir>>& a, const std::shared_ptr<Level<dir>>& b) const {
            return cmp_ref(*a, *b);
        }
    };
};


template<Direction dir>
class Book { 
    void add_order(std::shared_ptr<Order<dir>> order);

    private:
        using LevelBook = std::set<std::shared_ptr<Level<dir>>, typename Level<dir>::Cmp>;
        // maps price levels -> level objects
        using LevelMap = std::unordered_map<uint32_t, std::shared_ptr<Level<dir>>>;
        // maps order ids -> order objects
        using OrderMap = std::unordered_map<uint32_t, std::shared_ptr<Order<dir>>>;

        OrderMap order_map;
        LevelMap level_map;
        LevelBook levels;
};

class MatchingEngine {
    Book<Direction::BUY> bidBook;
    Book<Direction::SELL> askBook;
};
