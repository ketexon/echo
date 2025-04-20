#include "node.hpp"

#include <algorithm>
#include <limits>

static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);
static float MeasureBoxedTextHeight(Font font, const char *text, float maxWidth, float fontSize, float spacing, bool wordWrap);

void Node::update() {
	if(!active) return;
	for (auto& child : children) {
		child->update();
	}
}
void Node::render() {
	if(!visible) return;
	for (auto& child : children) {
		child->render();
	}
}

void Node::init(){
	if (initialized) return;
	initialized = true;

	for (auto& child : children) {
		child->init();
	}
}

void Node::activate() {
	active = true;
	for (auto& child : children) {
		child->activate();
	}
}

void Node::deactivate() {
	active = false;
	for (auto& child : children) {
		child->deactivate();
	}
}

void Node::set_active(bool active) {
	if (active) {
		activate();
	} else {
		deactivate();
	}
}

void Node::set_visible(bool visible) {
	this->visible = visible;
	for (auto& child : children) {
		child->set_visible(visible);
	}
}

std::shared_ptr<Node> Node::add_child(std::shared_ptr<Node> child) {
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

void Node::remove_child(std::shared_ptr<Node> child) {
	auto it = std::remove(children.begin(), children.end(), child);
	if (it != children.end()) {
		children.erase(it);
	}
}

void Node::clear_children() {
	for(auto& child : children) {
		child->parent.reset();
	}
	children.clear();
}

void Node::remove_self(){
	if (auto parent_ptr = parent.lock()) {
		parent_ptr->remove_child(shared_from_this());
	}
	parent.reset();
}

Vector2 Node::get_global_position() const {
	if (auto parent_ptr = parent.lock()) {
		auto parent_pos = parent_ptr->get_global_position();
		return { position.x + parent_pos.x, position.y + parent_pos.y };
	}
	return position;
}

Vector2 Node::get_global_position(Vector2 local_pos) const {
	auto global_pos = get_global_position();
	return { local_pos.x + global_pos.x, local_pos.y + global_pos.y };
}

Rectangle Node::get_global_rect(Rectangle local_rect) const {
	auto global_pos = get_global_position();
	return {
		local_rect.x + global_pos.x,
		local_rect.y + global_pos.y,
		local_rect.width,
		local_rect.height
	};
}



Rectangle Renderable::get_global_bounding_box() const
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

Vector2 Renderable::get_center() const {
	auto rect = get_bounding_box();
	return {
		rect.x + rect.width / 2,
		rect.y + rect.height / 2
	};
}

Rectangle Renderable::get_children_bounding_box() const {
	float l = 0,t = 0,r = 0,b = 0;
	for (const auto& child : children) {
		auto child_renderable = std::dynamic_pointer_cast<Renderable>(child);
		if(!child_renderable) continue;
		auto bb = child_renderable->get_bounding_box();
		l = std::min(l, bb.x);
		t = std::min(t, bb.y);
		r = std::max(r, bb.x + bb.width);
		b = std::max(b, bb.y + bb.height);
	}
	return {
		l,
		t,
		r - l,
		b - t
	};
}

void RectRenderer::render()
{
	if(rounding > 0){
		DrawRectangleRounded(get_global_rect(rect), rounding, 16, color);
	}
	else {
		DrawRectangleRec(get_global_rect(rect), color);
	}
	Base::render();
}

Rectangle RectRenderer::get_bounding_box() const
{
	return rect;
}

void CircleRenderer::render()
{
	DrawCircleV(get_global_position(), radius, BLUE);
	Base::render();
}

Rectangle CircleRenderer::get_bounding_box() const
{
	return {
		position.x - radius,
		position.y - radius,
		radius * 2,
		radius * 2
	};
}



void TextRenderer::render()
{
	if(max_width.has_value()){
		auto rect = Rectangle{
			position.x,
			position.y,
			*max_width,
			max_height.value_or(std::numeric_limits<float>::max())
		};
		DrawTextBoxedSelectable(
			font.value_or(GetFontDefault()),
			text.c_str(),
			get_global_rect(rect),
			font_size,
			spacing,
			word_wrap,
			color,
			0, 0,
			WHITE, WHITE
		);
	}
	else {
		DrawTextEx(
			font.value_or(GetFontDefault()),
			text.c_str(),
			get_global_position(),
			font_size,
			spacing,
			color
		);
	}
	Base::render();
}

Rectangle TextRenderer::get_bounding_box() const
{
	if(max_width.has_value()){
		float height = MeasureBoxedTextHeight(
			font.value_or(GetFontDefault()),
			text.c_str(),
			*max_width,
			font_size,
			spacing,
			word_wrap
		);
		auto rect = Rectangle{
			position.x,
			position.y,
			*max_width,
			height
		};
		rect.height = std::min(rect.height, max_height.value_or(rect.height));
		return rect;
	}
	else {
		auto text_size = MeasureTextEx(
			font.value_or(GetFontDefault()),
			text.c_str(),
			font_size,
			spacing
		);
		return {
			position.x,
			position.y,
			text_size.x,
			text_size.y
		};
	}
}

void RectContainer::render() {
	auto bb = rect_extrude(
		get_bounding_box(),
		padding_x,
		padding_y
	);
	bb.x += padding_x;
	bb.y += padding_y;

	rect = bb;
	Base::render();
}

void RectContainer::update(){
	for(auto& child : children) {
		child->position = {
			padding_x,
			padding_y
		};
	}
	Base::update();
}

Rectangle RectContainer::get_bounding_box() const
{
	auto rect = get_children_bounding_box();
	return rect;
}

// Draw text using font inside rectangle limits with support for text selection
static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/(float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth) > rec.width))
                {
                    textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + font.baseSize*scaleFactor) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                    isGlyphSelected = true;
                }

                // Draw current character glyph
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + textOffsetX, rec.y + textOffsetY }, fontSize, isGlyphSelected? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}

static float MeasureBoxedTextHeight(Font font, const char *text, float maxWidth, float fontSize, float spacing, bool wordWrap)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/(float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

	float maxOffsetY = 0;

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > maxWidth)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth) > maxWidth))
                {
                    textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }

                if ((codepoint != ' ') && (codepoint != '\t'))
                {
					maxOffsetY = textOffsetY + (font.baseSize*scaleFactor);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
	return maxOffsetY;
}