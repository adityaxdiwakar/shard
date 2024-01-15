#include "engine.hpp"

template<Direction dir>
void Level<dir>::add_order(std::shared_ptr<Order<dir>> order) {
    orders.push(order);
    volume += order->size;
}

template<Direction dir>
void Book<dir>::add_order(std::shared_ptr<Order<dir>> order) {
    // if not in level map, insert into level map and levels book set
    if (level_map.find(order->price) == level_map.end()) {
        std::shared_ptr<Level<dir>> new_level = make_shared<Level>(order);
        level_map[order->price] = new_level;
        levels.insert(new_level);
    }

    level_map[order->price]->add_order(order);
}
