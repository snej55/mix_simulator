// Created by Jens Kromdijk 05-01-2025

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <array>

namespace CST
{
    inline constexpr int WINDOW_START_WIDTH{800};
    inline constexpr int WINDOW_START_HEIGHT{640};
    inline constexpr int FONT_TEX_SIZE{64};

    inline constexpr int FLOW_FIELD_DEPTH_LIMIT{4}; // max number of quadtree subdivisions
    inline constexpr float FLOW_FIELD_TILE_SIZE{20.f}; // initial quadtree node size

    inline constexpr std::size_t NUM_LEVELS{5};
    inline constexpr std::array<const char*, NUM_LEVELS> LEVEL_PATHS{
        "data/maps/0.json", "data/maps/1.json", "data/maps/2.json", "data/maps/3.json", "data/maps/4.json"};

    inline constexpr std::array<const char*, 13> MENU_MESSAGES{"Don't get mugged!",
                                                               "An eggcellent idea!",
                                                               "A game most fowl",
                                                               "Quack them up!",
                                                               "'The family friendly rage room'",
                                                               "Don't pond-er too long!",
                                                               "You don't pay the bill!",
                                                               "A cheeky game of beak-a-boo",
                                                               "Show some quackitude!",
                                                               "Leave them shellshocked!",
                                                               "Send them to the ducktor!",
                                                               "Just wing it!",
                                                               "Beak them up!"};
    inline constexpr std::array<const char*, 4> IBL_SKIES{"sunset", "bright", "clouds", "bright"};
} // namespace CST

#endif
