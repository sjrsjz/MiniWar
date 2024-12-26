#pragma once
struct RegionData {
    float cell_center_x{};
    float cell_center_y{};
	float army_position_x = -1e6;
	float army_position_y = -1e6;
    float identity{};
    float is_capital{};
};