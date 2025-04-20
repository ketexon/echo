#pragma once

#include <raylib.h>
#include <memory>
#include <vector>
#include <optional>
#include <string>

enum class Alignment {
	Start,
	Center,
	End
};

constexpr Rectangle rect_extrude(Rectangle rect, float extrude) {
	return {
		rect.x - extrude,
		rect.y - extrude,
		rect.width + 2 * extrude,
		rect.height + 2 * extrude
	};
}

constexpr Rectangle rect_extrude(Rectangle rect, float extrude_x, float extrude_y) {
	return {
		rect.x - extrude_x,
		rect.y - extrude_y,
		rect.width + 2 * extrude_x,
		rect.height + 2 * extrude_y
	};
}

struct Node : std::enable_shared_from_this<Node> {
	std::weak_ptr<Node> parent;
	std::vector<std::shared_ptr<Node>> children;
	Vector2 position;

	bool initialized = false;
	bool active = true;
	bool visible = true;

	virtual void update();
	virtual void render();
	virtual void init();
	virtual void activate();
	virtual void deactivate();
	virtual void set_active(bool active);
	virtual void set_visible(bool visible);

	std::shared_ptr<Node> add_child(std::shared_ptr<Node> child);
	template<typename T, typename... Args>
	std::shared_ptr<T> add_child(Args&&... args) {
		auto child = std::make_shared<T>(std::forward<Args>(args)...);
		add_child(std::static_pointer_cast<Node>(child));
		return child;
	}
	void remove_child(std::shared_ptr<Node> child);
	void clear_children();
	void remove_self();

	Vector2 get_global_position() const;
	Vector2 get_global_position(Vector2 local_pos) const;
	Rectangle get_global_rect(Rectangle local_rect) const;
};

struct Renderable : Node {
	using Base = Node;

	Color color = WHITE;

	virtual Rectangle get_bounding_box() const = 0;
	Rectangle get_children_bounding_box() const;
	Rectangle get_global_bounding_box() const;
	Vector2 get_center() const;
};

struct RectRenderer : Renderable {
	using Base = Renderable;

	float rounding = 0;
	Rectangle rect;

	void render() override;
	Rectangle get_bounding_box() const override;
};

struct CircleRenderer : Renderable {
	using Base = Renderable;

	float radius = 5.0f;

	void render() override;

	Rectangle get_bounding_box() const override;
};

struct TextRenderer : Renderable {
	using Base = Renderable;

	std::string text;
	int font_size = 20;
	std::optional<Font> font;

	std::optional<float> max_width;
	std::optional<float> max_height;
	bool word_wrap = true;
	float spacing = 1;

	void render() override;
	Rectangle get_bounding_box() const override;
};


struct RectContainer : RectRenderer {
	using Base = RectRenderer;

	float padding_x;
	float padding_y;
	Alignment alignment_x;
	Alignment alignment_y;

	void render() override;
	void update() override;
	Rectangle get_bounding_box() const override;
};