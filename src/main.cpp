#define LAY_FLOAT 1
#define LAY_IMPLEMENTATION
#include <layout.h>

#include <raylib.h>
#include <memory>

Rectangle lay_get_rectangle(lay_context* ctx, lay_id id){
	Rectangle rect;
	auto lay_rect = lay_get_rect(ctx, id);
	rect.x = lay_rect[0];
	rect.y = lay_rect[1];
	rect.width = lay_rect[2];
	rect.height = lay_rect[3];
	return rect;
}

struct GameState : std::enable_shared_from_this<GameState> {
	virtual void render() = 0;
	virtual std::shared_ptr<GameState> update() = 0;
	virtual ~GameState() = default;
};

struct Menu : GameState {
	lay_context ctx;
	lay_id root;
	lay_id buttons_container;
	lay_id buttons[2];

	Menu(){
		lay_init_context(&ctx);
		create_layout();
	}

	~Menu(){
		lay_destroy_context(&ctx);
	}

	void create_layout(){
		lay_reset_context(&ctx);

		root = lay_item(&ctx);
		lay_set_size_xy(&ctx, root, GetScreenWidth(), GetScreenHeight());
		lay_set_contain(&ctx, root, LAY_ROW);

		buttons_container = lay_item(&ctx);
		lay_insert(&ctx, root, buttons_container);
		lay_set_contain(&ctx, buttons_container, LAY_COLUMN);
		lay_set_size_xy(&ctx, buttons_container, 200, 0);
		lay_set_behave(&ctx, buttons_container, LAY_VFILL);

		for(int i = 0; i < 2; ++i){
			buttons[i] = lay_item(&ctx);
			lay_insert(&ctx, buttons_container, buttons[i]);
			lay_set_size_xy(&ctx, buttons[i], 0, 50);
			lay_set_behave(&ctx, buttons[i], LAY_HFILL);
			lay_set_margins(&ctx, buttons[i], (lay_vec4){0, 0, 0, i != 1 ? 10.0 : 0});
		}

		lay_run_context(&ctx);
	}

	void update_layout(){
		lay_set_size_xy(&ctx, root, GetScreenWidth(), GetScreenHeight());
		lay_run_context(&ctx);
	}

	void render() override {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		auto rect = lay_get_rectangle(&ctx, buttons_container);
		DrawRectangleRec(rect, YELLOW);
		for(int i = 0; i < 2; ++i){
			lay_id button = buttons[i];
			auto rect = lay_get_rectangle(&ctx, button);
			DrawRectangleRec(rect, i == 0 ? BLUE : GREEN);
		}

		EndDrawing();
	}

	std::shared_ptr<GameState> update() override {
		if(IsWindowResized()){
			update_layout();
		}

		return shared_from_this();
	}
};

int main(){
	InitWindow(800, 600, "Hello World");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	std::shared_ptr<GameState> gameState = std::make_shared<Menu>();

	while(!WindowShouldClose()){
		gameState->render();
		gameState = gameState->update();
	}

	CloseWindow();

	return 0;
}