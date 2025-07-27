#pragma once

#include "core/object/object_affiliate.h"
#include <string>
#include <SDL3_ttf/SDL_ttf.h>

class TextLabel : public ObjectAffiliate
{
protected:
	TTF_Text* ttf_text_ = nullptr;
	std::string font_path_;
	int font_size_ = 16;

public:

	void render(int x, int y) {
		TTF_DrawRendererText(ttf_text_, x, y);
	}

	static TextLabel* addTextLabelChild(ObjectWorld* parent, const std::string& text, const std::string& font_path, int font_size, Anchor anchor = Anchor::CENTER);
	virtual void render() override;
	virtual void clean() override;
	// setters and getters
	void setFont(const std::string& font_path, int font_size);      // init() 之后需要立刻调用
	void setFontPath(const std::string& font_path);
	void setFontSize(int font_size);
	void setText(std::string ttf_text) { TTF_SetTextString(ttf_text_, ttf_text.c_str(), ttf_text.length()); }
	std::string getText() const { return ttf_text_->text; }
	int getTextWidth() {
		int w;
		TTF_GetTextSize(ttf_text_, &w, nullptr);
		return w;
	}
private:
	void updateSize();  // 根据文字内容决定大小
};
