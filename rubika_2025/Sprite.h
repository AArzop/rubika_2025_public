#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <filesystem>
#include <string>

class AnimationData;

class Sprite
{
public:

	// Première fonction appelé au moment de la création du sprite
	// Set les différents paramètres à leut valeur par défaut
	void Init();

	// Update l'animation du sprite (si possible)
	// En fonction des valeurs comprise dans l'AnimationData
	void Update(float deltaTime);

	// Reset l'état du Sprite à ses valeurs par défaut
	void Reset();

	// Fait un Get auprès du TextureMgr pour accèder à ses données
	// et Set la texture du sf::Sprite avec le résultat trouvé
	void SetTexture(const std::filesystem::path& textureName);

	// Fait un Get auprès du TextureMgr pour accèder à ses données
	// se sert du nom de la texture précédement set pour accèder à la bonne animation
	void SetAnimation(const std::string& animationName);

	// (Un)Pause l'animation
	void EnableAnimation(bool enable);

	// Retourne le Sprite pour le dessin
	sf::Sprite& Get() { return sfSprite; }

protected:

	// Le Sprite SFML qui va être utilisé pour dessiner
	sf::Sprite sfSprite;

	// Un pointeur vers l'animationData contenu dans le TextureMgr
	// Permet d'accèder directement aux données sans avoir à faire de get à chaque fois
	const AnimationData* CurrentAnimationData;

	// Le nom de la texture actuellement set
	std::filesystem::path CurrentTexture;

	// Le nom de l'animation actuellement set
	std::string CurrentAnimation;

	// Index courant de l'animation joué (à mettre en relation avec SpriteNum et SpritesOnLine)
	unsigned CurrentAnimationNb = 0;

	// Temps courant de l'animation joué (à mettre en relation avec TimeBetweenAnimation
	float CurrentAnimationTime = 0.f;

	// Est ce qye l'animaion est Pause ou pas
	bool PlayAnimation = true;
};