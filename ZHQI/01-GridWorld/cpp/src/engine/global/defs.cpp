#include "defs.h"
#include "util/asset_store.h"

Texture::Texture(const std::string& file_path)
{
	texture = AssetStore::getTexture(file_path);
	SDL_GetTextureSize(texture, &src_rect.w, &src_rect.h);
}
