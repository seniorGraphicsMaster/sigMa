#pragma once
#ifndef __MAP_H__
#define __MAP_H__


struct map_t
{
	vec2 grid = vec2(0); //for gridmap
	int map[15][15]{};
};
inline std::vector<map_t> create_grid() {
	std::vector<map_t> arr;
	map_t m;
	m = { vec2(5,10),{{0, 2, 0, 0, 0, 0, 0, 0, 0, 2},
					  {0, 2, 0, 0, 0, 0, 0, 0, 0, 0},
					  {0, 2, 0, 1, 0, 2, 2, 2, 0, 0},
					  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					  {0, 0, 1, 0, 0, 0, 0, 0, 0, 0}}
	};
	arr.emplace_back(m);
	return arr;
}


#endif
