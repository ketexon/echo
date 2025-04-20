#include <raylib.h>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include "node.hpp"
#include "main.hpp"

constexpr int DefaultWidth = 800;
constexpr int DefaultHeight = 600;

struct Node;
struct Player;
struct Level;
struct GameState;

struct GameState : std::enable_shared_from_this<GameState>
{
	virtual void render() = 0;
	virtual std::shared_ptr<GameState> update() = 0;
	virtual ~GameState() = default;
};

Grid::Grid(int width, int height)
	: width(width), height(height)
{
}

Rectangle Grid::get_cell_rect(int x, int y) const
{
	return {
		position.x + x * cell_size_x,
		position.y + y * cell_size_y,
		cell_size_x,
		cell_size_y
	};
}

Vector2 Grid::get_point(int x, int y) const {
	auto rect = get_cell_rect(x, y);
	return { rect.x, rect.y };
}

void Grid::clamp_point(int& x, int& y) const {
	x = std::clamp(x, 0, width);
	y = std::clamp(y, 0, height);
}

void Grid::clamp_cell(int& x, int& y) const {
	x = std::clamp(x, 0, width - 1);
	y = std::clamp(y, 0, height - 1);
}

Vector2 Grid::get_point_clamped(int x, int y) const {
	clamp_point(x, y);
	return get_point(x, y);
}

void Grid::render()
{
	Base::render();

	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			auto rect = get_global_rect(get_cell_rect(i, j));
			DrawRectangleLinesEx(rect, 1, BLACK);
		}
	}
}

Rectangle Grid::get_bounding_box() const
{
	return {
		position.x,
		position.y,
		static_cast<float>(width * cell_size_x),
		static_cast<float>(height * cell_size_y)
	};
}

Player::Player(std::weak_ptr<Level> level)
	: level(level)
{
}

void Player::init(){
	Base::init();

	grid = level.lock()->grid;
	radius = 5.0f;
	color = BLUE;

	set_grid_position(grid_x, grid_y);
}

void Player::update()
{
	Base::update();

	auto level_ptr = level.lock();
	if(!level_ptr || !level_ptr->selected) {
		return;
	}

	if(IsKeyPressed(KEY_W)) grid_y -= 1;
	if(IsKeyPressed(KEY_S)) grid_y += 1;
	if(IsKeyPressed(KEY_A)) grid_x -= 1;
	if(IsKeyPressed(KEY_D)) grid_x += 1;

	level_ptr->grid->clamp_point(grid_x, grid_y);
	set_grid_position(grid_x, grid_y);
}

void Satellite::init()
{
	Base::init();
	rect = { -5, -5, 10, 10 };
	color = RED;
}

void Level::init()
{
	grid = add_child<Grid>(7, 6);
	grid->position = { 0, 0 };

	player = add_child<Player>(std::static_pointer_cast<Level>(shared_from_this()));
	player->grid_x = 0;
	player->grid_y = 0;

	satellite = add_child<Satellite>();
	satellite->position = grid->get_point(5, 5);

	Base::init();
}


void Level::render()
{
	Base::render();

	if (selected) {
		auto rect = rect_extrude(
			grid->get_bounding_box(),
			10
		);

		rect = grid->get_global_rect(rect);

		DrawRectangleLinesEx(rect, 2, RED);
	}
}

void Level::update(){
	Base::update();


}

struct Root : Node {
	using Base = Node;

	std::shared_ptr<Level> level1;
	std::shared_ptr<Level> level2;

	std::shared_ptr<Level> selected_level;

	void init() override
	{
		Base::init();

		level1 = add_child<Level>();
		level1->position = { 0, 0 };

		level2 = add_child<Level>();
		level2->position = { 20 * Grid::cell_size_x, 0 };

		auto container = add_child<RectContainer>();
		container->position = { 0, 0 };
		container->padding_x = 1;
		container->padding_y = 1;
		container->color = RED;
		container->rounding = 0.3f;

		auto text_renderer = container->add_child<TextRenderer>();
		text_renderer->font_size = 5;
		text_renderer->text = "Hello World";
		text_renderer->color = YELLOW;

		auto text_renderer_bb = text_renderer->get_bounding_box();

		selected_level = level1;
		selected_level->selected = true;
	}
};

struct Game : GameState
{
	std::shared_ptr<Root> root;

	Camera2D camera;

	Game()
	{
		root = std::make_shared<Root>();
		root->init();

		camera.zoom = 2.0f;
		camera.target = root->level1->grid->get_center();
		camera.offset = {
			static_cast<float>(GetScreenWidth()) / 2,
			static_cast<float>(GetScreenHeight()) / 2
		};
		camera.rotation = 0.0f;
	}

	~Game()
	{
	}

	void render() override
	{
		auto selected_level_grid = root->selected_level->grid;
		camera.target = selected_level_grid->get_center();
		auto safe_area = rect_extrude(selected_level_grid->get_bounding_box(), 40);
		camera.zoom = std::min(
			static_cast<float>(GetScreenWidth()) / safe_area.width,
			static_cast<float>(GetScreenHeight()) / safe_area.height
		);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode2D(camera);

		root->render();

		EndMode2D();

		EndDrawing();
	}

	std::shared_ptr<GameState> update() override
	{
		if (IsWindowResized())
		{
			camera.offset = camera.offset = {
				static_cast<float>(GetScreenWidth()) / 2,
				static_cast<float>(GetScreenHeight()) / 2
			};
		}

		root->update();

		return shared_from_this();
	}
};

int main()
{
	InitWindow(DefaultWidth, DefaultHeight, "Hello World");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	std::shared_ptr<GameState> gameState = std::make_shared<Game>();

	while (!WindowShouldClose())
	{
		gameState->render();
		gameState = gameState->update();
	}

	CloseWindow();

	return 0;
}