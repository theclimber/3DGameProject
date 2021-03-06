/*
 * AppDemarrage.cpp
 *
 *  Created on: 8 mai 2014
 *      Author: sym
 */

#include "AppDemarrage.h"

AppDemarrage::AppDemarrage(): mRoot(0), mWindow(0), mSceneMgr(0), mCamera(0), mInputListener(0), mNodeMainPlayer(0), mPlayer(0) {

}

AppDemarrage::~AppDemarrage() {
	delete mRoot;
//	delete mWindow;
//	delete mSceneMgr;
	delete mCamera;
    delete mPlayer;
//	delete mInputListener;
}

bool AppDemarrage::start() {
	//création du root

	mRoot = new Ogre::Root("plugins.cfg", "ogre.cfg", "Ogre.log");

	//chargement des ressources
	Ogre::ConfigFile configFile;
	configFile.load("resources.cfg");

	//Ajout des path des ressources dans le ResourceGroupManager
	Ogre::ConfigFile::SectionIterator seci = configFile.getSectionIterator();
	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
	    secName = seci.peekNextKey();
	    Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
	    Ogre::ConfigFile::SettingsMultiMap::iterator i;
	    for (i = settings->begin(); i != settings->end(); ++i)
	    {
	        typeName = i->first;
	        archName = i->second;
	        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
	            archName, typeName, secName);
	    }
	}

	//-----------------------------------
	//Chargement du moteur en lui-même :
	//-----------------------------------
	if(!(mRoot->restoreConfig() || mRoot->showConfigDialog())) //showConfigDialog = fenêtre choix de configuration qui renvoie true si utilisateur clique sur OK
	{
	    return false;
	}

	//Configuration manuelle du RenderSystem :

	/*Ogre::RenderSystem *rs = mRoot->getRenderSystemByName("Direct3D9 Rendering Subsystem"); // Pour OpenGL : "OpenGL Rendering Subsystem"
	mRoot->setRenderSystem(rs);
	rs->setConfigOption("Full Screen", "No");
	rs->setConfigOption("Video Mode", "1280 x 800 @ 32-bit colour");
	rs->setConfigOption("VSync", "Yes");*/

	// Création de la fenêtre
	mWindow = mRoot->initialise(true, "3DGameProject"); // 1er param : ogre crée la fenêtre lui-même ou pas, 2e param: titre de la fenêtre

	// Initialisation des resources et du nombre de MipMap par défaut:
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Création du scene manager :
	mSceneMgr = mRoot->createSceneManager("DefaultSceneManager", "SceneManager");
    
	// création de la scene avec le viewport, la camera et le player
    setupScene();

	//remplissage de la scene
	remplirScene();

	// instancie le FrameListener
	createFrameListener();

	// Boucle infinie "toute faite" par Ogre
	// mRoot->startRendering();

	//Boucle infinie manuelle :
	while(true)
	{
		// Methode qui prend en charge le raffraichissement de la fenêtre :
		Ogre::WindowEventUtilities::messagePump();

		// Pour stopper le processus quand on ferme la fenêtre:
		if(mWindow->isClosed())
		    return false;

		// Pour faire le rendu
		if(!mRoot->renderOneFrame()) // Fait le rendu d'une image
			return false;
	}

	return true;
}

void AppDemarrage::setupScene()
{
    // Type d'ombres qu'on va utiliser
    mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
    mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
    
    // Création du noeud parent du player et de la camera
    mNodeMainPlayer = mSceneMgr->getRootSceneNode()->createChildSceneNode("nodeMainPlayer");
    
    // Création du player
    mPlayer = new Player(mSceneMgr, "sinbad", "Sinbad.mesh", mNodeMainPlayer, Ogre::Vector3(0, 50, 0));
    
    // Création de la camera (associée au player)
    mCamera = new NewCamera("camera1", mSceneMgr, mNodeMainPlayer, mPlayer);
    
	// Viewport = "une vue 3D"
	Ogre::Viewport *vue = mWindow->addViewport(mCamera->getOgreCamera());
	vue->setBackgroundColour(Ogre::ColourValue(0.5, 0.8, 0.8));
	mCamera->getOgreCamera()->setAspectRatio(Ogre::Real(vue->getActualWidth()) / Ogre::Real(vue->getActualHeight()));
    
    // Réglage de la précision des textures pour les matériaux (ici anisotrope).
    Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(8);
}

void AppDemarrage::remplirScene()
{
	// Lumière ambiante :
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.6, 0.6, 0.6));
    
    //ajout d'un brouillard
    mSceneMgr->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(0.5f, 0.8f, 0.8f), 0.5, 100, 200);
    
    // Ajout d'un "soleil"
    mLight = mSceneMgr->createLight("light1");
    mLight->setType(Ogre::Light::LT_DIRECTIONAL);
    mLight->setDirection(10.0, -20.0, -5);
    mLight->setDiffuseColour(0.4, 0.4, 0.4);
    mLight->setCastShadows(true);

	// Ajout des modèles 3D
    Ogre::Entity *bonhomme = mSceneMgr->createEntity("2eBonhomme", "Sinbad.mesh");
    Ogre::SceneNode *nodeBon = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    nodeBon->attachObject(bonhomme);
    nodeBon->setPosition(Ogre::Vector3(100, 5, 100));
    

    
	// Lumière diffuse :
	/*Ogre::Light* light = mSceneMgr->createLight("lightPoint");
	light->setPosition(15, 30, 15);
	light->setDiffuseColour(1.0, 1.0, 1.0);
    light->setCastShadows(true);*/
    
    //Ajout d'un plan pour le sol
    Ogre::Plane plan(Ogre::Vector3::UNIT_Y, 0);
    Ogre::MeshManager::getSingleton().createPlane("sol", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plan, 500, 500, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
    Ogre::Entity *entSol= mSceneMgr->createEntity("EntiteSol", "sol");
    mNodeSol = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mNodeSol->attachObject(entSol);
    entSol->setMaterialName("grassFloor");
    entSol->setCastShadows(false);
    
    createTerrain();
}

void AppDemarrage::createFrameListener()
{
    mInputListener = new InputListener(mWindow, mCamera, mPlayer);
    mRoot->addFrameListener(mInputListener);
}

void AppDemarrage::createTerrain()
{
    // options globales
    mTerrainOptions = OGRE_NEW Ogre::TerrainGlobalOptions();
    mTerrainOptions->setMaxPixelError(8);
    mTerrainOptions->setLightMapDirection(mLight->getDerivedDirection());
    mTerrainOptions->setCompositeMapDistance(3000);
    mTerrainOptions->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
    mTerrainOptions->setCompositeMapDiffuse(mLight->getDiffuseColour());

    mTerrain = OGRE_NEW Ogre::Terrain(mSceneMgr);

    Ogre::Image img;
    img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    // informations géométriques
    Ogre::Terrain::ImportData imp;
    imp.inputImage = &img;
    imp.terrainSize = img.getWidth();
    imp.worldSize = 8000;
    imp.inputScale = 600;
    imp.minBatchSize = 33;
    imp.maxBatchSize = 65;

    // textures
    imp.layerList.resize(3);
    imp.layerList[0].worldSize = 100;
    imp.layerList[0].textureNames.push_back("grass_green-01_diffusespecular.dds");
    imp.layerList[0].textureNames.push_back("grass_green-01_normalheight.dds");
    imp.layerList[1].worldSize = 30;
    imp.layerList[1].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
    imp.layerList[1].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
    imp.layerList[2].worldSize = 200;
    imp.layerList[2].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
    imp.layerList[2].textureNames.push_back("dirt_grayrocky_normalheight.dds");
    mTerrain->prepare(imp);
    mTerrain->load();

    // plaquage de texture
    Ogre::TerrainLayerBlendMap* blendMap1 = mTerrain->getLayerBlendMap(1);
    float* pBlend1 = blendMap1->getBlendPointer();

    for (Ogre::uint16 y = 0; y < mTerrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < mTerrain->getLayerBlendMapSize(); ++x)
        {
            *pBlend1++ = 150;
        }
    }

    blendMap1->dirty();
    blendMap1->update();

    mTerrain->freeTemporaryResources();
}
