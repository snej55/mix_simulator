// Created by Jens Kromdijk 05-01-2025

#pragma once

namespace CST
{
    inline constexpr int WINDOW_START_WIDTH{800};
    inline constexpr int WINDOW_START_HEIGHT{640};
    inline constexpr int FONT_TEX_SIZE{64};

    inline constexpr int FLOW_FIELD_DEPTH_LIMIT{4}; // max number of quadtree subdivisions
    inline constexpr float FLOW_FIELD_TILE_SIZE{30.f}; // initial quadtree node size
} // namespace CST
