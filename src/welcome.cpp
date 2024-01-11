#include "welcome.h"

unordered_map<string, Texture> TextureManager::textures;

Texture& TextureManager::getTexture(string textureName)
{
    auto result = textures.find(textureName);
    if (result == textures.end()) {
        // Texture does not already exist in the map, go get it!
        Texture newTexture;
        newTexture.loadFromFile("files/images/" + textureName + ".png");
        textures[textureName] = newTexture;
        return textures[textureName];
    }
    else {
        // Texture already exists, return it!
        return result->second;
    }
}
