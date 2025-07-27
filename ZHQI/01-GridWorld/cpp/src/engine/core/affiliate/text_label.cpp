#include "text_label.h"
#include "util/asset_store.h"
void TextLabel::render()
{
	ObjectAffiliate::render();
	auto pos = parent_->getScreenPos() + offset_;
	TTF_DrawRendererText(ttf_text_, pos.x, pos.y);

}

void TextLabel::clean()
{
	if (!ttf_text_) return;
	TTF_DestroyText(ttf_text_);
}

TextLabel* TextLabel::addTextLabelChild(ObjectWorld* parent, const std::string& text, const std::string& font_path, int font_size, Anchor anchor)
{
	auto text_label = new TextLabel();
	text_label->init();
	text_label->setFont(font_path, font_size);
	text_label->setText(text);
	text_label->setAnchor(anchor);
	text_label->updateSize();
	if (parent) {
		parent->addChild(text_label);
		text_label->setParent(parent);
	}
	return text_label;
}

void TextLabel::setFont(const std::string& font_path, int font_size)
{
	font_path_ = font_path;
	font_size_ = font_size;
	auto font = AssetStore::getFont(font_path_, font_size_);
	if (!ttf_text_) ttf_text_ = AssetStore::createTTF_Text("", font_path_, font_size_);
	TTF_SetTextFont(ttf_text_, font);
}

void TextLabel::setFontPath(const std::string& font_path)
{
	font_path_ = font_path;
	// 有可能是新的ttf_font，所以从AssetStore里载入，
	// 然后设置TTF_text里的ttf_font
	auto font = AssetStore::getFont(font_path_, font_size_);
	TTF_SetTextFont(ttf_text_, font);
}

void TextLabel::setFontSize(int font_size)
{
	font_size_ = font_size;
	auto font = AssetStore::getFont(font_path_, font_size_);
	TTF_SetTextFont(ttf_text_, font);
}

void TextLabel::updateSize()
{
	int w, h;
	TTF_GetTextSize(ttf_text_, &w, &h);
	setSize(glm::vec2(w, h));
}

