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
