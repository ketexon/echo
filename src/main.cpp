#include <raylib.h>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

constexpr int DefaultWidth = 800;
constexpr int DefaultHeight = 600;

struct Node;
struct Player;
struct Level;
struct GameState;

constexpr Rectangle rect_extrude(Rectangle rect, float extrude) {
	return {
		rect.x - extrude,
		rect.y - extrude,
		rect.width + 2 * extrude,
		rect.height + 2 * extrude
	};
}

struct Node : std::enable_shared_from_this<Node> {
	std::weak_ptr<Node> parent;
	std::vector<std::shared_ptr<Node>> children;
	Vector2 position;

	bool initialized = false;
	bool active = true;
	bool visible = true;

	virtual void update() {
		if(!active) return;
		for (auto& child : children) {
			child->update();
		}
	}
	virtual void render() {
		if(!visible) return;
		for (auto& child : children) {
			child->render();
		}
	}

	virtual void init(){
		if (initialized) return;
		initialized = true;

		for (auto& child : children) {
			child->init();
		}
	}

	virtual void activate() {
		active = true;
		for (auto& child : children) {
			child->activate();
		}
	}

	virtual void deactivate() {
		active = false;
		for (auto& child : children) {
			child->deactivate();
		}
	}

	virtual void set_active(bool active) {
		if (active) {
			activate();
		} else {
			deactivate();
		}
	}

	virtual void set_visible(bool visible) {
		this->visible = visible;
		for (auto& child : children) {
			child->set_visible(visible);
		}
	}

	std::shared_ptr<Node> add_child(std::shared_ptr<Node> child) {
		child->parent = shared_from_this();
		children.push_back(child);
		if(initialized) {
			child->init();
		}
		if(active) {
			child->activate();
		}
		if(visible) {
			child->set_visible(visible);
		}
		return child;
	}

	template<typename T, typename... Args>
	std::shared_ptr<T> add_child(Args&&... args) {
		auto child = std::make_shared<T>(std::forward<Args>(args)...);
		add_child(std::static_pointer_cast<Node>(child));
		return child;
	}

	void remove_child(std::shared_ptr<Node> child) {
		auto it = std::remove(children.begin(), children.end(), child);
		if (it != children.end()) {
			children.erase(it);
		}
	}

	void clear_children() {
		for(auto& child : children) {
			child->parent.reset();
		}
		children.clear();
	}

	void remove_self(){
		if (auto parent_ptr = parent.lock()) {
			parent_ptr->remove_child(shared_from_this());
		}
		parent.reset();
	}

	Vector2 get_global_position() const {
		if (auto parent_ptr = parent.lock()) {
			auto parent_pos = parent_ptr->get_global_position();
			return { position.x + parent_pos.x, position.y + parent_pos.y };
		}
		return position;
	}

	Vector2 get_global_position(Vector2 local_pos) const {
		auto global_pos = get_global_position();
		return { local_pos.x + global_pos.x, local_pos.y + global_pos.y };
	}

	Rectangle get_global_rect(Rectangle local_rect) const {
		auto global_pos = get_global_position();
		return {
			local_rect.x + global_pos.x,
			local_rect.y + global_pos.y,
			local_rect.width,
			local_rect.height
		};
	}
};

struct Renderable : Node {
	using Base = Node;

	Color color = WHITE;

	virtual Rectangle get_bounding_box() const = 0;

	Rectangle get_global_bounding_box() const
	{
		auto rect = get_bounding_box();
		auto global_pos = get_global_position();
		return {
			global_pos.x + rect.x,
			global_pos.y + rect.y,
			rect.width,
			rect.height
		};
	}

	Vector2 get_center() const {
		auto rect = get_bounding_box();
		return {
			rect.x + rect.width / 2,
			rect.y + rect.height / 2
		};
	}
};

struct GameState : std::enable_shared_from_this<GameState>
{
	virtual void render() = 0;
	virtual std::shared_ptr<GameState> update() = 0;
	virtual ~GameState() = default;
};

struct Grid : Renderable {
	using Base = Renderable;

	constexpr static int cell_size_x = 10;
	constexpr static int cell_size_y = 10;

	int width;
	int height;

	Grid(int width, int height)
		: width(width), height(height)
	{
	}

	Rectangle get_cell_rect(int x, int y) const
	{
		return {
			position.x + x * cell_size_x,
			position.y + y * cell_size_y,
			cell_size_x,
			cell_size_y
		};
	}

	Vector2 get_point(int x, int y) const {
		auto rect = get_cell_rect(x, y);
		return { rect.x, rect.y };
	}

	void clamp_point(int& x, int& y) const {
		x = std::clamp(x, 0, width);
		y = std::clamp(y, 0, height);
	}

	void clamp_cell(int& x, int& y) const {
		x = std::clamp(x, 0, width - 1);
		y = std::clamp(y, 0, height - 1);
	}

	Vector2 get_point_clamped(int x, int y) const {
		clamp_point(x, y);
		return get_point(x, y);
	}

	void render() override
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

	Rectangle get_bounding_box() const override
	{
		return {
			position.x,
			position.y,
			static_cast<float>(width * cell_size_x),
			static_cast<float>(height * cell_size_y)
		};
	}
};

struct RectRenderer : Renderable {
	using Base = Renderable;

	Rectangle rect;

	void render() override
	{
		Base::render();

		DrawRectangleRec(get_global_rect(rect), BLUE);
	}

	Rectangle get_bounding_box() const override
	{
		return rect;
	}
};

struct CircleRenderer : Renderable {
	using Base = Renderable;

	float radius = 5.0f;

	void render() override
	{
		Base::render();

		DrawCircleV(get_global_position(), radius, BLUE);
	}

	Rectangle get_bounding_box() const override
	{
		return {
			position.x - radius,
			position.y - radius,
			radius * 2,
			radius * 2
		};
	}
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

	void init() override
	{
		Base::init();
		grid = level.lock()->grid;
		radius = 5.0f;
		color = BLUE;

		set_grid_position(grid_x, grid_y);
	}

	void update() override;
};

struct Satellite : RectRenderer {
	using Base = RectRenderer;
	int grid_x = 0;
	int grid_y = 0;

	void init() override
	{
		Base::init();
		rect = { -5, -5, 10, 10 };
		color = RED;
	}
};

struct Level : Node {
	using Base = Node;

	std::shared_ptr<Grid> grid;
	std::shared_ptr<Player> player;
	std::shared_ptr<Satellite> satellite;

	bool selected = false;

	void init() override
	{
		Base::init();

		grid = add_child<Grid>(10, 10);
		grid->position = { 0, 0 };

		player = add_child<Player>();
		player->level = std::static_pointer_cast<Level>(shared_from_this());
		player->position = grid->get_point(0, 0);
		player->grid_x = 0;
		player->grid_y = 0;

		satellite = add_child<Satellite>();
		satellite->position = grid->get_point(5, 5);
	}

	void render() override
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
};

void Player::update()
{
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

		position = level_ptr->grid->get_point(grid_x, grid_y);
	}
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
		camera.target = root->level1->grid->get_center();

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