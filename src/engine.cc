#include <iostream>
#include "engine.hpp"

namespace {
    // get type of opposite direction
    template<Direction dir>
    struct OpposingDir;

    template<>
    struct OpposingDir<Direction::BUY> {
        static constexpr Direction value = Direction::SELL;
    };

    template<>
    struct OpposingDir<Direction::SELL> {
        static constexpr Direction value = Direction::BUY;
    };
}

template<Direction dir>
void Level<dir>::add_order(std::shared_ptr<Order<dir>> order) {
    orders.push(order);
    volume += order->size;
}

template<Direction dir>
void Book<dir>::add_order(std::shared_ptr<Order<dir>> order) {
    // if not in level map, insert into level map and levels book set
    if (level_map.find(order->price) == level_map.end()) {
        std::shared_ptr<Level<dir>> new_level = make_shared<Level<dir>>(order);
        level_map[order->price] = new_level;
        levels.insert(new_level);
    }

    level_map[order->price]->add_order(order);
}

template<Direction dir>
bool fillable_against(std::shared_ptr<Order<dir>> order, uint32_t price) {
    if constexpr (dir == Direction::BUY) return order->price >= price;
    else return order->price <= price;
}

template <Direction dir>
void MatchingEngine::process(std::shared_ptr<Order<dir>> order) {
    // check opposite book to see if order is more aggressive than top
    auto top = get_side<OpposingDir<dir>::value>().top();
    std::cout << "ACK " << order->id << std::endl;

    // less aggressive than top of other book, add to dir book
    if (!top 
        || top->orders.empty() 
        || !fillable_against(order, top->orders.front()->price)) {
        get_side<dir>().add_order(order);

        // TODO: advertise order added, no fill (tell server)

        return;
    }

    // aggressive order, fill against other book
    while (true) { // TODO loop condition
        auto top_order = top->orders.front();
        if (top_order->size > order->size) {
            top_order->size -= order->size;
            // TODO: announce fills for both parties (top.participant and order.participant)
            // announce OUT
            std::cout << "OUT " << order->id << std::endl;
            std::cout << "FILL " << order-> id << std::endl;
            std::cout << "FILL " << top_order->id << std::endl;
            break;
        } 

        // order clears the top order
        order->size -= top_order->size;
        std::cout << "OUT " << top_order->id << std::endl;
        std::cout << "FILL " << top_order->id << std::endl;
        std::cout << "FILL " << order->id << std::endl;
        top->orders.pop(); // consumed top order
        // announce top order participant is OUT

        if (top->orders.empty()) {
            // top is empty, remove this level
            get_side<OpposingDir<dir>::value>().rm_level(top);

            if (order->size == 0) {
                std::cout << "OUT " << order->id << std::endl;
            } else {
                process(order);
            }
            return;
        }

    }
}

template void MatchingEngine::process(std::shared_ptr<Order<Direction::BUY>> order);
template void MatchingEngine::process(std::shared_ptr<Order<Direction::SELL>> order);
