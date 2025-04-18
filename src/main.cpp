#include <clay.h>

#include <raylib.h>
#include <memory>

struct GameState : std::enable_shared_from_this<GameState> {
	virtual void render() = 0;
	virtual std::shared_ptr<GameState> update() = 0;
	virtual ~GameState() = default;
};

struct Menu : GameState {
	Menu(){
	}

	~Menu(){
	}

	void create_layout(){
	}

	void render() override {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		EndDrawing();
	}

	std::shared_ptr<GameState> update() override {
		if(IsWindowResized()){
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