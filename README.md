# Tooling

L'objectif de cet exercice est de se familiariser avec un outil d'interface (ImGUi) afin de créer une interface de debug permettant de visualiser certains informations du jeu mais aussi des outils de debug permettant d'alterer le flow/règle/... du jeu

Pour se faire, implémenter les différentes fonctions `DrawDebug` des managers afin de réaliser certains taches:

Commençons par le `GameMgr`, voici une liste de différentes features à implémenter :
- Visualisation des variables du jeu comme le round actuel, le temps de break, le nombre d'enemis par vagues...
- Ajouter des boutons permettant de lancer des actions spécifiques:
  * Activer/Désactiver le God Mode
  * Lancer un round spécifique (ex: jouer le round 250)
  * Relancer le jeu un fois perdu
  * ...
- Permettre de modifier via une interface les variables du jeu pour que n'importe qui puisse ajuster le jeu.
  * Dans un premier temps, ces modifications ne doivent avoir lieu que lors de l'instance active du programme (on perd les modifications au reboot).
  * Faire en sorte que ces variables persistent d'une session à l'autre.

Ensuite, passez au `TextureMgr` et au `RandomMgr`. Voici quelques idées:
- Changer la seed
- Visualiser les textures chargées, en chargement...
- Pouvoir unload un texture à la volée

## Conseils

- Voici une petite [aide ImGui](https://pthom.github.io/imgui_manual_online/)

# Debug

L'objectif de cet exercice est de se familliariser avec le debugger. Pour ce faire, vous aurez la base de code d'un jeu assez simple (voir les règles ci-dessous).
Cependant, cette base de code comporte de nombreuses erreurs disséminées à différents endroits du code. Certaines (la plupart) sont très graves (crash) d'autres sont plus discrètes.
Votre objectif dans un premier temps est de réparer le projet pour qu'il puisse fonctionner comme il devrait. Je vous encourage très fortement ;) ;) ;) à garder une trace de vos différents trouvailles et de la façon dont vous les avez résolues.

Ce projet est le votre, vous pouvez donc en réécrire certains parties si vous désirez. Gardez cependant à l'esprit que le projet à d'abord été complété avant d'ajouter/de retirer/de modifier certains parties pour ajouter des bugs. Une réécriture complète n'est donc très probablement pas la bonne solution.

## Règles du jeu
- Une succession de round
- Le joueur peut se déplacer dans les 8 directions à sa guise
- Durant un round, un certain nombre d'ennemis vont apparaitre et balayer l'écran dans une des 4 directions
- Si le joueur entre en contact avec l'un des ennemis, la partie s'arrète
- Il s'écoule un temps non constant entre l'apparition de deux ennemis
- Le nombre d'ennemis croit avec les round tandis que le temps d'apparition diminue
- Un round se finit lorsqu'il n'y a plus d'ennemi à faire apparaitre.
- Entre deux rounds s'écoule un petit temps permettant au joueur de faire une pause

## Conseils

Si vous êtes perdus et vous ne savez pas quel chemin suivre, voici quelques conseils:
- Lancer **toujours** votre programme avec debbuger attaché. Autrement, vous n'aurez que le mimimum d'informations. De même, utiliser la configuration **debug** lors de vos recherches.
- La première chose à faire est de se focaliser sur le bug les plus problématique (les crashs)
- Certains sont reproduisibles à l'infini, d'autres sont plus situationnel. Traquer s'en un qui est facilement reproduisible.
- Focalisez vous sur un seul problème à la fois (sauf bien entendu si vous voyez une erreur évidente en lisant le code)
- Vous ne devriez avoir besoin que d'ajouter un membre (et donc de modifier ce qu'il faut pour l'intégrer). Si vous ajoutez plus, c'est peut être un signe que vous n'allez pas dans la bonne direction.
- N'hésitez pas à poser des questions!!!

------------------------------------------------------------------------------

# Système d'animation

1. Les classes qui devraient être impactées par cette partie ont été changés durant le dernier push
   La classe TextureMgr et Sprite ont des commentaires expliquant brievement ce qu'elle devrait faire

2. Voici l'ordre que je conseille :
   Commenncer par la parsing dans le TextureMgr
   	Au moment du loadTexture_Thread, vérifier que les dépendances existent (le fichier de metadata a le même nom que la texture avec l'extension xml)
   	Avant ou après le loadFromFile de la texture, il faut load les metadata
   	Petit point sur rapidxml en dessous

   Implémenter la classe Sprite pour fonctionner avec les données de l'animation
   	Le but est de load la texture avec un IntRect particulier qui correspond qu'à la partie à rendre
   	La taille de l'animation ne change pas au sein d'une animation (ce qui peut aider), il faut donc bouger ce Rect au cours de l'animation
   	Ne pas oublier le deltaTime

   Modifier le GameMgr pour utiliser la classe Sprite au lieu du sf::Sprite


## Note rapidxml : 

Besoin d'inclure #include <rapidxml/rapidxml_utils.hpp> pour utiliser le fichier (plus simple que d'utiliser fopen...)
Voici les types auquels vous devrez avoir besoin :
	- rapidxml::file<>
 	- rapidxml::xml_document
  	- rapidxml::xml_node<>
  	- rapidxml::xml_attribute<>

Voici les fonctions auquelles vous devriez avoir besoin:
	- parse<0>
 	- first_node
 	- first_attribute
  	- value / value_size
   	- next_sibling
   
# Multi Threading

## Contexte
1. Se rendre compte de la durée de chargement d'une texture
  => C'est long, très long

2. Rendre le chargement de texture le plus indolore pour la main thread
  - Créer un TextureMgr pour réutiliser les sf::Texture déjà chargés
  - Si la texture n'est pas chargé en mémoire, il va falloir la charger sur un autre Thread, mais quid du créateur qui lui bosse sur la Main Thread
  - Le créateur (GameMgr) doit fonctionner en asynchrone, être rappelé par le TextureMgr, mais que ce passe-t-il si on a deux requêtes sur la même ressource en même temps (ou pendant le load du premier)

## Implémentation

### GameMgr
Lors d'un event (touché pressé par exemple), créer X enemy qui vont utiliser une des textures "enemy_texture_N.png" ou N est compris entre 1 et 5
Au moment de la création, le GameMgr demande au TextureMgr si la texture est loadé, si oui on l'utilise directement, si non, on fait une requête

### TextureMgr
```
  struct RequestData
  {
    RequestData(const std::filesystem::path& texturePath, TextureMgr* pMgr);
  
    std::filesystem::path TexturePath;
    std::vector<TextureLoadingCallback> Callbacks;
    std::vector<void*> UserData;
    sf::Thread Thread
  }

  std::map<std::filesystem::path, sf::Texture> Textures
  std::map<std::filesystem::path, RequestData> Requesting

  void RequestTextureLoading(const std::filesystem::path& texturePath, TextureLoadingCallback callback, void* userData);
  void LoadTexture_Thread(std::filesystem::path texturePath);
  sf::Texture* GetTexture(const std::filesystem::path& texturePath);
```

et dans le cpp:

```
TextureMgr::RequestData::RequestData(const std::filesystem::path& texturePath, TextureMgr* pMgr) :
	Thread(std::bind(&TextureMgr::LoadTexture_Thread, pMgr, texturePath))
{}
```

void Update(float deltaTime);
Dépiler les requêtes dans Requesting et appeler les callbacks si finis
      - Moyen simple : on check chaque frame toutes les requetes => peu efficace
      - On ajoute un container en plus pour faire ca
        `std::set<std::filesystem::path> RequestDone`


## Note sur les callbacks :
Le système de callbacks de la std est un enfer, voici quelques solutions pour ne pas avoir à débugger tout ca :
1. Définir le type de la callback (dans la partie public du textureMgr):
  	`using TextureLoadingCallback = std::function<void(const sf::Texture*, void* pUserData)>;`

2. La méthode  RequestTextureLoading du textureMgr attend un callback, on va donc lui donner.
Partons du principe que le GameMgr à une méthode qui respecte la signature:
       `void GameMgr::LoadCallback(const TextureData*, void* pUserData);`

   L'appel à RequestTextureLoading ressemblera à:
     `textureMgr.RequestLoadTexture("path", std::bind(&GameMgr::LoadCallback, this, std::placeholders::_1, std::placeholders::_2), userData);`

4. Pour appeler la callback stockée, on utilise la même manière qu'un appelle de fonction classique. Au sein du RequestData:
```
  for (int i = 0; i < Callbacks.size(); ++i)
  {
    TextureLoadingCallback& callback = TextureLoadingCallback[i];
     void* userData = UserData[i];

     callback(texture, userData); // La texture provient du loading
  }
```

Pour ajouter dans la map Requesting,
```
auto itEmplace = Requesting.emplace(std::piecewise_construct, std::forward_as_tuple(texturePath), std::forward_as_tuple(texturePath, this));
```
