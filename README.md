1. Se rendre compte de la durée de chargement d'une texture
  => C'est long, très long

2. Rendre le chargement de texture le plus indolore pour la main thread
  - Créer un TextureMgr pour réutiliser les sf::Texture déjà chargés
  - Si la texture n'est pas chargé en mémoire, il va falloir la charger sur un autre Thread, mais quid du créateur qui lui bosse sur la Main Thread
  - Le créateur (GameMgr) doit fonctionner en asynchrone, être rappelé par le TextureMgr, mais que ce passe-t-il si on a deux requêtes sur la même ressource en même temps (ou pendant le load du premier)

---------------
GameMgr
    - lors d'un event (touché pressé par exemple), crée X enemy qui vont utiliser une des textures "enemy_texture_N.png" ou N est compris entre 1 et 5
    - Au moment de la création, le GameMgr demande au TextureMgr si la texture est loadé, si oui on l'utilise directement, si non, on fait une requête

TextureMgr
  std::map<std::filesystem::path, sf::Texture> Textures

  struct RequestData
  {
    std::filesystem::path TexturePath;
    std::vector<TextureLoadingCallback> Callbacks;
    std::vector<void*> UserData;
    sf::Thread Thread
  }
  
  std::map<std::filesystem::path, RequestData> Requesting

  void RequestTextureLoading(const std::filesystem::path& texturePath, TextureLoadingCallback callback, void* userData);
  sf::Texture* GetTexture(const std::filesystem::path& texturePath);

  void Update(float deltaTime);
    => Dépiler les requêtes dans Requesting et appeler les callbacks si finis
      - Moyen simple : on check chaque frame toutes les requetes => peu efficace
      - On ajoute un container en plus pour faire ca
        std::set<std::filesystem::path> RequestDone
