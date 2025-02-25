1. Se rendre compte de la durée de chargement d'une texture
  => C'est long, très long

2. Rendre le chargement de texture le plus indolore pour la main thread
  - Créer un TextureMgr pour réutiliser les sf::Texture déjà chargés
  - Si la texture n'est pas chargé en mémoire, il va falloir la charger sur un autre Thread, mais quid du créateur qui lui bosse sur la Main Thread
  - Le créateur (GameMgr) doit fonctionner en asynchrone, être rappelé par le TextureMgr, mais que ce passe-t-il si on a deux requêtes sur la même ressource en même temps (ou pendant le load du premier)

---------------
GameMgr
    - lors d'un event (touché pressé par exemple), créer X enemy qui vont utiliser une des textures "enemy_texture_N.png" ou N est compris entre 1 et 5
    - Au moment de la création, le GameMgr demande au TextureMgr si la texture est loadé, si oui on l'utilise directement, si non, on fait une requête

TextureMgr
  std::map<std::filesystem::path, sf::Texture> Textures

  struct RequestData
  {
    RequestData(const std::filesystem::path& texturePath, TextureMgr* pMgr);
  
    std::filesystem::path TexturePath;
    std::vector<TextureLoadingCallback> Callbacks;
    std::vector<void*> UserData;
    sf::Thread Thread
  }

et dans le cpp:

TextureMgr::RequestData::RequestData(const std::filesystem::path& texturePath, TextureMgr* pMgr) :
	Thread(std::bind(&TextureMgr::LoadTexture_Thread, pMgr, texturePath))
{}
  
  std::map<std::filesystem::path, RequestData> Requesting

  void RequestTextureLoading(const std::filesystem::path& texturePath, TextureLoadingCallback callback, void* userData);
  void LoadTexture_Thread(std::filesystem::path texturePath);
  sf::Texture* GetTexture(const std::filesystem::path& texturePath);

  void Update(float deltaTime);
    => Dépiler les requêtes dans Requesting et appeler les callbacks si finis
      - Moyen simple : on check chaque frame toutes les requetes => peu efficace
      - On ajoute un container en plus pour faire ca
        std::set<std::filesystem::path> RequestDone


Note sur les callbacks :
Le système de callbacks de la std est un enfer, voici quelques solutions pour ne pas avoir à débugger tout ca :
1. Définir le type de la callback (dans la partie public du textureMgr):
  	using TextureLoadingCallback = std::function<void(const sf::Texture*, void* pUserData)>;

2. La méthode  RequestTextureLoading du textureMgr attend un callback, on va donc lui donner
    partons du principer que le GameMgr à une méthode qui respecte la signature:
       void GameMgr::LoadCallback(const TextureData*, void* pUserData);

   L'appel à RequestTextureLoading ressemblera à:
     textureMgr.RequestLoadTexture("path", std::bind(&GameMgr::LoadCallback, this, std::placeholders::_1, std::placeholders::_2), userData);

3. Pour appeler la callback stockée, on utilise la même manière qu'un appelle de fonction classique. Au seing du RequestData:
  for (int i = 0; i < Callbacks.size(); ++i)
  {
    TextureLoadingCallback& callback = TextureLoadingCallback[i];
     void* userData = UserData[i];

     callback(texture, userData); // La texture provient du loading
  }


Pour ajouter dans la map Requesting, 
		auto itEmplace = Requesting.emplace(std::piecewise_construct,
			std::forward_as_tuple(texturePath),
			std::forward_as_tuple(texturePath, this));
