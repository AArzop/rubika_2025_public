#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <filesystem>
#include <string>

class AnimationData;

class Sprite
{
public:

	// Premi�re fonction appel� au moment de la cr�ation du sprite
	// Set les diff�rents param�tres � leut valeur par d�faut
	void Init();

	// Update l'animation du sprite (si possible)
	// En fonction des valeurs comprise dans l'AnimationData
	void Update(float deltaTime);

	// Reset l'�tat du Sprite � ses valeurs par d�faut
	void Reset();

	// Fait un Get aupr�s du TextureMgr pour acc�der � ses donn�es
	// et Set la texture du sf::Sprite avec le r�sultat trouv�
	void SetTexture(const std::filesystem::path& textureName);

	// Fait un Get aupr�s du TextureMgr pour acc�der � ses donn�es
	// se sert du nom de la texture pr�c�dement set pour acc�der � la bonne animation
	void SetAnimation(const std::string& animationName);

	// (Un)Pause l'animation
	void EnableAnimation(bool enable);

	// Retourne le Sprite pour le dessin
	sf::Sprite& Get() { return sfSprite; }

protected:

	// Le Sprite SFML qui va �tre utilis� pour dessiner
	sf::Sprite sfSprite;

	// Un pointeur vers l'animationData contenu dans le TextureMgr
	// Permet d'acc�der directement aux donn�es sans avoir � faire de get � chaque fois
	const AnimationData* CurrentAnimationData;

	// Le nom de la texture actuellement set
	std::filesystem::path CurrentTexture;

	// Le nom de l'animation actuellement set
	std::string CurrentAnimation;

	// Index courant de l'animation jou� (� mettre en relation avec SpriteNum et SpritesOnLine)
	unsigned CurrentAnimationNb = 0;

	// Temps courant de l'animation jou� (� mettre en relation avec TimeBetweenAnimation
	float CurrentAnimationTime = 0.f;

	// Est ce qye l'animaion est Pause ou pas
	bool PlayAnimation = true;
};