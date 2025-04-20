#pragma once

#include "node.hpp"

struct Level;

struct Grid : Renderable {
	using Base = Renderable;

	constexpr static int cell_size_x = 10;
	constexpr static int cell_size_y = 10;

	int width;
	int height;

	Grid(int width, int height);

	Rectangle get_cell_rect(int x, int y) const;
	Vector2 get_point(int x, int y) const;
	void clamp_point(int& x, int& y) const;
	void clamp_cell(int& x, int& y) const;
	Vector2 get_point_clamped(int x, int y) const;
	void render() override;
	Rectangle get_bounding_box() const override;
};

template<typename T>
struct GridNode {
	std::weak_ptr<Grid> grid;
	int grid_x = 0;
	int grid_y = 0;

	void set_grid_position(int x, int y)
	{
		static_cast<T*>(this)->position = grid.lock()->get_point(x, y);
	}
};

struct Player : CircleRenderer, GridNode<Player> {
	using Base = CircleRenderer;

	std::weak_ptr<Level> level;

	Player(std::weak_ptr<Level>);

	void init() override;
	void update() override;
};

struct Satellite : RectRenderer, GridNode<Satellite> {
	using Base = RectRenderer;
	int grid_x = 0;
	int grid_y = 0;

	void init() override;
};

struct Level : Node {
	using Base = Node;

	std::shared_ptr<Grid> grid;
	std::shared_ptr<Player> player;
	std::shared_ptr<Satellite> satellite;

	bool selected = false;

	void init() override;
	void render() override;
	void update() override;
};