#ifndef REGION_DATA_H
#define REGION_DATA_H
struct RegionData {
    float cell_center_x{};
    float cell_center_y{};
    float army_position_x = -1e6;
    float army_position_y = -1e6;
    float identity{};
    float region_additional_info{};
};
#endif