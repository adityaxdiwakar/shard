#include "engine.hpp"

void Level::add_order(std::shared_ptr<Order> order) {
    orders.push(order);
    volume += order->size;
}

void Book::add_order(std::shared_ptr<Order> order) {
    // if not in level map, insert into level map and levels book set
    if (level_map.find(order->price) == level_map.end()) {
        std::shared_ptr<Level> new_level = make_shared<Level>(order);
        level_map[order->price] = new_level;
        levels.insert(new_level);
    }

    level_map[order->price]->add_order(order);
}
