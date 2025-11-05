
#include "EngineGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "OgreLogManager.h"

#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreItem.h"
#include "OgreLight.h"

// Ogre-Next HLMS includes
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

#include "OgreHlmsSamplerblock.h"

// For file loading
#include "OgreArchiveManager.h"
#include "OgreFileSystemLayer.h"
#include <fstream>
#include "OgreResourceGroupManager.h"
#include "OgreStreamSerialiser.h"

// RapidJSON Includes (Assuming Ogre includes provide access, otherwise add manually)
// If you get compiler errors, you might need to add:
// #include "rapidjson/document.h"
// #include "rapidjson/error/en.h"
// And potentially adjust include paths in CMakeLists.txt if not already accessible.
// Ogre uses RapidJSON internally, check OgreCommon/System/Desktop/UnitTesting.cpp for include examples.
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

using namespace Demo;

namespace Demo
{
    // --- Helper Functions for JSON Parsing ---

    Ogre::Vector3 parseVector3(const rapidjson::Value& arr) {
        if (!arr.IsArray() || arr.Size() != 3) {
            // Log error or return default
            Ogre::LogManager::getSingleton().logMessage(
                "WARNING: Failed to parse Vector3 from JSON array.", Ogre::LML_CRITICAL);
            return Ogre::Vector3::ZERO;
        }
        return Ogre::Vector3(
            static_cast<Ogre::Real>(arr[0].GetDouble()),
            static_cast<Ogre::Real>(arr[1].GetDouble()),
            static_cast<Ogre::Real>(arr[2].GetDouble())
        );
    }

    Ogre::Quaternion parseQuaternion(const rapidjson::Value& arr) {
        if (!arr.IsArray() || arr.Size() != 4) {
            // Log error or return default
            Ogre::LogManager::getSingleton().logMessage(
                "WARNING: Failed to parse Quaternion from JSON array.", Ogre::LML_CRITICAL);
            return Ogre::Quaternion::IDENTITY;
        }
        // Assuming order is w, x, y, z in JSON
        return Ogre::Quaternion(
            static_cast<Ogre::Real>(arr[0].GetDouble()),
            static_cast<Ogre::Real>(arr[1].GetDouble()),
            static_cast<Ogre::Real>(arr[2].GetDouble()),
            static_cast<Ogre::Real>(arr[3].GetDouble())
        );
    }

    Ogre::ColourValue parseColourValue(const rapidjson::Value& arr, bool includesAlpha = true) {
        if (!arr.IsArray() || (includesAlpha && arr.Size() != 4) || (!includesAlpha && arr.Size() != 3)) {
            // Log error or return default
            Ogre::LogManager::getSingleton().logMessage(
                "WARNING: Failed to parse ColourValue from JSON array.", Ogre::LML_CRITICAL);
            return Ogre::ColourValue::White;
        }
        return Ogre::ColourValue(
            static_cast<Ogre::Real>(arr[0].GetDouble()),
            static_cast<Ogre::Real>(arr[1].GetDouble()),
            static_cast<Ogre::Real>(arr[2].GetDouble()),
            includesAlpha ? static_cast<Ogre::Real>(arr[3].GetDouble()) : 1.0f
        );
    }
    // --- End Helper Functions ---

    EngineGameState::EngineGameState(const Ogre::String& helpDescription,
        int argc, const char* argv[]) :
        TutorialGameState( helpDescription ),
        mSceneToLoad( "scene.json" ),  // Default scene if no argument is provided
		mArgc( argc ),
        mArgv( argv )
    {
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::loadSceneFromJson(const Ogre::String& filename)
    {
        Ogre::LogManager::getSingleton().logMessage("Loading scene from JSON: " + filename, Ogre::LML_TRIVIAL);
        Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::Root* root = mGraphicsSystem->getRoot();

        // --- Load JSON File using Ogre Resource System ---
        // 1. Attempt to load as an absolute/relative path from the filesystem
        std::string jsonData;
        std::ifstream inFile(filename.c_str(), std::ios::binary | std::ios::in);

        if (inFile.is_open())
        {
            Ogre::LogManager::getSingleton().logMessage("Loading scene from direct file path.");
            inFile.seekg(0, std::ios::end);
            jsonData.resize(static_cast<size_t>(inFile.tellg()));
            inFile.seekg(0, std::ios::beg);
            inFile.read(&jsonData[0], static_cast<std::streamsize>(jsonData.size()));
            inFile.close();
        }
        else
        {
            // 2. If direct open failed, try Ogre's resource system
            Ogre::LogManager::getSingleton().logMessage("Direct file path failed. Trying Ogre resource system for: " + filename);
            Ogre::DataStreamPtr stream;
            try {
                stream = Ogre::ResourceGroupManager::getSingleton().openResource(
                    filename, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, true);

                if (!stream)
                {
                    throw Ogre::FileNotFoundException(0, "Could not open scene file via ResourceGroupManager: " + filename, __FUNCTION__, __FILE__, __LINE__);
                }

                jsonData = stream->getAsString();
                stream->close();
            }
            catch (Ogre::Exception& e) {
                Ogre::LogManager::getSingleton().logMessage("Could not open/find scene file '" + filename + "' using direct path or resource system. Error: " + e.getFullDescription(), Ogre::LML_CRITICAL);
                throw; // Rethrow the exception
            }
        }

        // --- Parse JSON ---
        rapidjson::Document document;
        document.Parse(jsonData.c_str());

        if (document.HasParseError())
        {
            Ogre::LogManager::getSingleton().logMessage(
                "JSON parse error in " + filename + " at offset " +
                Ogre::StringConverter::toString(document.GetErrorOffset()) + ": " +
                rapidjson::GetParseError_En(document.GetParseError()), Ogre::LML_CRITICAL);
            // Optionally throw or handle error appropriately
            throw Ogre::Exception(Ogre::Exception::ERR_INVALIDPARAMS, "JSON parse error in " + filename, __FUNCTION__);
            // return;
        }
        // ____________________________________

        // --- Process Scene Settings ---
        if (document.HasMember("scene_settings") && document["scene_settings"].IsObject())
        {
            const auto& settings = document["scene_settings"];

            // Ambient Light
            if (settings.HasMember("ambient_light") && settings["ambient_light"].IsObject())
            {
                const auto& ambient = settings["ambient_light"];
                Ogre::ColourValue upperHemi = Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f;
                Ogre::ColourValue lowerHemi = Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f;
                Ogre::Vector3 hemisphereDir = Ogre::Vector3::UNIT_Y;

                if (ambient.HasMember("upper_hemisphere")) upperHemi = parseColourValue(ambient["upper_hemisphere"]);
                if (ambient.HasMember("lower_hemisphere")) lowerHemi = parseColourValue(ambient["lower_hemisphere"]);
                if (ambient.HasMember("hemisphere_direction")) hemisphereDir = parseVector3(ambient["hemisphere_direction"]);

                sceneManager->setAmbientLight(upperHemi, lowerHemi, hemisphereDir);
                Ogre::LogManager::getSingleton().logMessage("Applied ambient light settings from JSON.", Ogre::LML_TRIVIAL);
            }

            // Camera (Initial Setup)
            if (settings.HasMember("camera") && settings["camera"].IsObject())
            {
                const auto& camSettings = settings["camera"];
                Ogre::Vector3 camPos = Ogre::Vector3(0, 6, 10);
                Ogre::Vector3 camLookAt = Ogre::Vector3::ZERO;

                if (camSettings.HasMember("position")) camPos = parseVector3(camSettings["position"]);
                if (camSettings.HasMember("look_at")) camLookAt = parseVector3(camSettings["look_at"]);

                mGraphicsSystem->getCamera()->setPosition(camPos);
                mGraphicsSystem->getCamera()->lookAt(camLookAt);
                Ogre::LogManager::getSingleton().logMessage("Applied camera settings from JSON.", Ogre::LML_TRIVIAL);
            }
        }
        else {
            Ogre::LogManager::getSingleton().logMessage("No 'scene_settings' found in JSON. Using defaults.", Ogre::LML_NORMAL);
            // Apply default ambient light if not in JSON
            sceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f,
                Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f,
                Ogre::Vector3::UNIT_Y);
        }

        // --- Process Lights ---
        if (document.HasMember("lights") && document["lights"].IsArray())
        {
            const auto& lights = document["lights"];
            Ogre::LogManager::getSingleton().logMessage("Processing " + Ogre::StringConverter::toString(lights.Size()) + " lights from JSON.", Ogre::LML_TRIVIAL);
            for (rapidjson::SizeType i = 0; i < lights.Size(); ++i)
            {
                const auto& lightData = lights[i];
                if (!lightData.IsObject()) continue;

                Ogre::Light* light = sceneManager->createLight();
                Ogre::SceneNode* lightNode = sceneManager->getRootSceneNode()->createChildSceneNode(); // Could attach later based on component
                lightNode->attachObject(light);

                if (lightData.HasMember("type") && lightData["type"].IsString())
                {
                    std::string type = lightData["type"].GetString();
                    if (type == "directional")
                    {
                        light->setType(Ogre::Light::LT_DIRECTIONAL);
                        if (lightData.HasMember("direction"))
                        {
                            light->setDirection(parseVector3(lightData["direction"]).normalisedCopy());
                        }
                        else {
                            light->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z); // Default direction
                        }
                    }
                    else if (type == "point")
                    {
                        light->setType(Ogre::Light::LT_POINT);
                        if (lightData.HasMember("position"))
                        {
                            lightNode->setPosition(parseVector3(lightData["position"]));
                        }
                        // Add attenuation properties if needed
                    }
                    else if (type == "spot")
                    {
                        light->setType(Ogre::Light::LT_SPOTLIGHT);
                        if (lightData.HasMember("position"))
                        {
                            lightNode->setPosition(parseVector3(lightData["position"]));
                        }
                        if (lightData.HasMember("direction"))
                        {
                            light->setDirection(parseVector3(lightData["direction"]).normalisedCopy());
                        }
                        else {
                            light->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z); // Default direction
                        }
                        // Add inner/outer angle, falloff etc. if needed
                    }
                }

                if (lightData.HasMember("power_scale") && lightData["power_scale"].IsNumber())
                {
                    light->setPowerScale(static_cast<Ogre::Real>(lightData["power_scale"].GetDouble()));
                }
                else {
                    light->setPowerScale(1.0f); // Default power
                }

                if (lightData.HasMember("diffuse_colour"))
                {
                    light->setDiffuseColour(parseColourValue(lightData["diffuse_colour"], false));
                }
                if (lightData.HasMember("specular_colour"))
                {
                    light->setSpecularColour(parseColourValue(lightData["specular_colour"], false));
                }
            }
        }
        else {
            Ogre::LogManager::getSingleton().logMessage("No 'lights' array found in JSON. Creating default directional light.", Ogre::LML_NORMAL);
            // Create default light if none specified
            Ogre::Light* light = sceneManager->createLight();
            Ogre::SceneNode* lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
            lightNode->attachObject(light);
            light->setPowerScale(1.0f);
            light->setType(Ogre::Light::LT_DIRECTIONAL);
            light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
        }

        // --- Process Entities ---
        if (document.HasMember("entities") && document["entities"].IsArray())
        {
            const auto& entities = document["entities"];
            Ogre::LogManager::getSingleton().logMessage("Processing " + Ogre::StringConverter::toString(entities.Size()) + " entities from JSON.", Ogre::LML_TRIVIAL);

            for (rapidjson::SizeType i = 0; i < entities.Size(); ++i)
            {
                const auto& entityData = entities[i];
                if (!entityData.IsObject() || !entityData.HasMember("components") || !entityData["components"].IsObject())
                {
                    Ogre::LogManager::getSingleton().logMessage("Skipping invalid entity definition #" + Ogre::StringConverter::toString(i), Ogre::LML_NORMAL);
                    continue;
                }

                entt::entity entity = mRegistry.create();
                const auto& components = entityData["components"];
                Ogre::String entityName = entityData.HasMember("name") ? entityData["name"].GetString() : ("Entity_" + Ogre::StringConverter::toString(static_cast<uint32_t>(entity)));

                // -- Transform Component --
                Ogre::Vector3 pos = Ogre::Vector3::ZERO;
                Ogre::Quaternion ori = Ogre::Quaternion::IDENTITY;
                Ogre::Vector3 scale = Ogre::Vector3::UNIT_SCALE;
                if (components.HasMember("TransformComponent") && components["TransformComponent"].IsObject())
                {
                    const auto& transformData = components["TransformComponent"];
                    if (transformData.HasMember("position")) pos = parseVector3(transformData["position"]);
                    if (transformData.HasMember("orientation")) ori = parseQuaternion(transformData["orientation"]);
                    if (transformData.HasMember("scale")) scale = parseVector3(transformData["scale"]);
                }
                else {
                    Ogre::LogManager::getSingleton().logMessage("Entity " + entityName + " missing TransformComponent. Using default.");
                }
                mRegistry.emplace<TransformComponent>(entity, pos, ori, scale);

                // -- OgreRenderable Component --
                Ogre::Item* item = nullptr;
                Ogre::SceneNode* sceneNode = nullptr;
                if (components.HasMember("OgreRenderableComponent") && components["OgreRenderableComponent"].IsObject())
                {
                    const auto& renderableData = components["OgreRenderableComponent"];
                    std::string meshName = "Cube_d.mesh"; // Default mesh
                    std::string materialName = "BaseWhite"; // Default material
                    Ogre::String resourceGroup = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME; // Default resource group

                    if (renderableData.HasMember("mesh") && renderableData["mesh"].IsString())
                    {
                        meshName = renderableData["mesh"].GetString();
                    }
                    else {
                        Ogre::LogManager::getSingleton().logMessage("Entity " + entityName + " OgreRenderableComponent missing 'mesh'. Using default.", Ogre::LML_NORMAL);
                    }
                    if (renderableData.HasMember("material") && renderableData["material"].IsString())
                    {
                        materialName = renderableData["material"].GetString();
                    }
                    else {
                        Ogre::LogManager::getSingleton().logMessage("Entity " + entityName + " OgreRenderableComponent missing 'material'. Using default.", Ogre::LML_NORMAL);
                    }
                    if (renderableData.HasMember("resource_group") && renderableData["resource_group"].IsString())
                    {
                        resourceGroup = renderableData["resource_group"].GetString();
                    }


                    try {
                        item = sceneManager->createItem(meshName, resourceGroup, Ogre::SCENE_DYNAMIC);
                        item->setDatablockOrMaterialName(materialName); // Use setDatablockOrMaterialName for flexibility

                        sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)
                            ->createChildSceneNode(Ogre::SCENE_DYNAMIC);
                        sceneNode->attachObject(item);

                        // Initial placement using TransformComponent data
                        sceneNode->setPosition(pos);
                        sceneNode->setOrientation(ori);
                        sceneNode->setScale(scale);

                        mRegistry.emplace<OgreRenderableComponent>(entity, item, sceneNode);
                        Ogre::LogManager::getSingleton().logMessage("Created Ogre renderable for " + entityName + " (Mesh: " + meshName + ", Material: " + materialName + ")", Ogre::LML_TRIVIAL);
                    }
                    catch (Ogre::Exception& e) {
                        Ogre::LogManager::getSingleton().logMessage("Failed to create Ogre renderable for entity " + entityName + ": " + e.getFullDescription(), Ogre::LML_CRITICAL);
                        // Clean up potentially partially created Ogre objects if needed
                        if (item && sceneNode) sceneNode->detachObject(item);
                        if (item) sceneManager->destroyItem(item);
                        if (sceneNode) sceneManager->destroySceneNode(sceneNode);
                        mRegistry.destroy(entity); // Destroy the EnTT entity if Ogre setup failed
                        continue; // Skip other components for this entity
                    }
                }
                else {
                    Ogre::LogManager::getSingleton().logMessage("Entity " + entityName + " missing OgreRenderableComponent. Entity will not be visible.", Ogre::LML_CRITICAL);
                }

                // -- Spin Component --
                if (components.HasMember("SpinComponent") && components["SpinComponent"].IsObject())
                {
                    const auto& spinData = components["SpinComponent"];
                    float speed = 0.0f;
                    if (spinData.HasMember("speed") && spinData["speed"].IsNumber())
                    {
                        speed = static_cast<float>(spinData["speed"].GetDouble());
                    }
                    mRegistry.emplace<SpinComponent>(entity, speed);
                }

                // --- Add processing for other components here ---

                Ogre::LogManager::getSingleton().logMessage("Successfully processed entity: " + entityName, Ogre::LML_TRIVIAL);
            }
        }
        else {
            Ogre::LogManager::getSingleton().logMessage("No 'entities' array found in JSON.", Ogre::LML_NORMAL);
        }
        Ogre::LogManager::getSingleton().logMessage("Finished loading scene from JSON.", Ogre::LML_TRIVIAL);
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::parseCommandLineArgs(int argc, const char* argv[])
    {
        for (int i = 1; i < argc; ++i)
        {
            // Check for the --scene argument
            if (Ogre::String(argv[i]) == "--scene" && (i + 1) < argc)
            {
                mSceneToLoad = argv[i + 1];
                Ogre::LogManager::getSingleton().logMessage(
                    "Command line: Loading scene " + mSceneToLoad);
                ++i;  // Increment 'i' again to skip the path argument in the next loop
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::createScene01()
    {
        // Parse command line arguments to get scene file
        parseCommandLineArgs(mArgc, mArgv);

        // --- 1. Create HLMS Materials (Keep this or move to a separate loading step) ---
        // It's often better to load/create materials *before* loading the scene that uses them.
        Ogre::Root* root = mGraphicsSystem->getRoot();
        Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
        Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

        // Create a "RedPlastic" material if it doesn't exist
        Ogre::String datablockName = "RedPlastic";
        Ogre::HlmsDatablock* datablock = hlmsPbs->getDatablock(datablockName);
        if (!datablock)
        {
            Ogre::HlmsPbsDatablock* pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
                hlmsPbs->createDatablock(datablockName, datablockName, Ogre::HlmsMacroblock(),
                    Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));
            pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);
            pbsDatablock->setDiffuse(Ogre::Vector3(1.0f, 0.0f, 0.0f));
            pbsDatablock->setRoughness(0.2f);
            pbsDatablock->setMetalness(0.0f);
            Ogre::LogManager::getSingleton().logMessage("Created 'RedPlastic' datablock.");
        }

        // Create a "PolishedMetal" material if it doesn't exist
        datablockName = "PolishedMetal";
        datablock = hlmsPbs->getDatablock(datablockName);
        if (!datablock)
        {
            Ogre::HlmsPbsDatablock* pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
                hlmsPbs->createDatablock(datablockName, datablockName, Ogre::HlmsMacroblock(),
                    Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));
            pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);
            pbsDatablock->setDiffuse(Ogre::Vector3(0.95f, 0.95f, 0.95f));
            pbsDatablock->setRoughness(0.1f);
            pbsDatablock->setMetalness(1.0f);
            Ogre::LogManager::getSingleton().logMessage("Created 'PolishedMetal' datablock.");
        }
        // Add creation for "BaseWhite" or ensure it's loaded from files
        datablockName = "BaseWhite";
        datablock = hlmsPbs->getDatablock(datablockName);
        if (!datablock)
        {
            // Consider loading this from a default material file instead
            Ogre::HlmsPbsDatablock* pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
                hlmsPbs->createDatablock(datablockName, datablockName, Ogre::HlmsMacroblock(),
                    Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));
            pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);
            pbsDatablock->setDiffuse(Ogre::Vector3(0.9f, 0.9f, 0.9f));
            pbsDatablock->setRoughness(0.5f);
            pbsDatablock->setMetalness(0.0f);
            Ogre::LogManager::getSingleton().logMessage("Created 'BaseWhite' datablock.");
        }


        // --- 2. Load Scene from JSON ---
        try {
            loadSceneFromJson(mSceneToLoad); // Assuming the file is passed as a command line argument, or is in a location Ogre can find (e.g., bin/Data if added as resource path)
        }
        catch (Ogre::Exception& e) {
            Ogre::LogManager::getSingleton().logMessage("Failed to load scene from JSON: " + e.getFullDescription(), Ogre::LML_CRITICAL);
            // Handle the error, maybe load a fallback scene or show an error message
            // For now, we'll just log and continue with an potentially empty scene
        }


        // --- 3. Set up Camera Controller (after potential camera setup from JSON) ---
        // Camera position might be overridden by JSON, so create controller after loading
        mCameraController = new CameraController(mGraphicsSystem, false);

        // Call base class setup AFTER loading our scene
        TutorialGameState::createScene01();

  //      Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();

  //      // --- 1. Set up Lighting (from PbsMaterials sample) ---

  //      // Set ambient light
  //      sceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f,
  //          Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f,
  //          Ogre::Vector3::UNIT_Y);

  //      // Create a directional light
  //      Ogre::Light* light = sceneManager->createLight();
  //      Ogre::SceneNode* lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
  //      lightNode->attachObject(light);
  //      light->setPowerScale(1.0f);
  //      light->setType(Ogre::Light::LT_DIRECTIONAL);
  //      light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());

  //      // --- 2. Create HLMS Materials (from PbsMaterials sample) ---

  //      Ogre::Root* root = mGraphicsSystem->getRoot();
  //      Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
  //      Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

  //      // Create a "RedPlastic" material
  //      Ogre::String datablockName = "RedPlastic";
  //      Ogre::HlmsPbsDatablock* pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
  //          hlmsPbs->createDatablock(datablockName,
  //              datablockName,
  //              Ogre::HlmsMacroblock(),
  //              Ogre::HlmsBlendblock(),
  //              Ogre::HlmsParamVec()));

		//pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);    // Use metallic workflow to avoid throwing assert

  //      pbsDatablock->setDiffuse(Ogre::Vector3(1.0f, 0.0f, 0.0f));
  //      pbsDatablock->setRoughness(0.2f);
  //      pbsDatablock->setMetalness(0.0f);

  //      // Create a "PolishedMetal" material
  //      datablockName = "PolishedMetal";
  //      pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
  //          hlmsPbs->createDatablock(datablockName,
  //              datablockName,
  //              Ogre::HlmsMacroblock(),
  //              Ogre::HlmsBlendblock(),
  //              Ogre::HlmsParamVec()));

  //      pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);    // Use metallic workflow to avoid throwing assert

  //      pbsDatablock->setDiffuse(Ogre::Vector3(0.95f, 0.95f, 0.95f));
  //      pbsDatablock->setRoughness(0.1f);
  //      pbsDatablock->setMetalness(1.0f); // Set to 1.0 for metallic

  //      // --- 4. Create EnTT Entities and attach Ogre Components ---

  //      // Create a 2x2 grid of objects
  //      const int numX = 2;
  //      const int numZ = 2;
  //      const float spacing = 2.5f;

  //      for (int x = 0; x < numX; ++x) // Outer x loop starts
  //      {
  //          for (int z = 0; z < numZ; ++z) // Outer z loop starts
  //          {
  //              // --- Create EnTT Entity ---
  //              entt::entity entity = mRegistry.create();

  //              // --- Create Ogre Renderable ---
  //              Ogre::Item* item;
  //              if ((x + z) % 2 == 0)
  //              {
  //                  item = sceneManager->createItem(
  //                      "Cube_d.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
  //                      Ogre::SCENE_DYNAMIC);
  //                  item->setDatablock("RedPlastic"); // Assign material here
  //              }
  //              else
  //              {
  //                  item = sceneManager->createItem(
  //                      "Sphere1000.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
  //                      Ogre::SCENE_DYNAMIC);
  //                  item->setDatablock("PolishedMetal"); // Assign material here
  //              }

  //              Ogre::SceneNode* sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)
  //                  ->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  //              sceneNode->attachObject(item);

  //              // --- Attach Components to Entity ---

  //              // 1. Attach the Ogre renderable parts
  //              mRegistry.emplace<OgreRenderableComponent>(entity, item, sceneNode);

  //              // 2. Attach the transform data
  //              Ogre::Vector3 initialPos((x - (numX - 1) * 0.5f) * spacing,
  //                  0.0f,
  //                  (z - (numZ - 1) * 0.5f) * spacing);
  //              mRegistry.emplace<TransformComponent>(entity, initialPos);

  //              // 3. Attach the logic component
  //              mRegistry.emplace<SpinComponent>(entity, (x + z * numX) * 0.1f + 0.5f);

  //          }
  //      }

  //      // --- 5. Set up Camera ---
  //      mGraphicsSystem->getCamera()->setPosition(Ogre::Vector3(0, 6, 10));
  //      mGraphicsSystem->getCamera()->lookAt(Ogre::Vector3(0, 0, 0));
  //      mCameraController = new CameraController(mGraphicsSystem, false);

  //      TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::destroyScene(void) // destroyScene is now correctly within scope
    {
        Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();

        // --- EnTT Cleanup System ---
        // We must manually destroy the Ogre objects we created.

        auto view = mRegistry.view<OgreRenderableComponent>();
        for (auto entity : view)
        {
            auto& renderable = view.get<OgreRenderableComponent>(entity);

            // Detach from node
            renderable.sceneNode->detachAllObjects();

            // Destroy the Ogre objects
            sceneManager->destroyItem(renderable.item);
            sceneManager->destroySceneNode(renderable.sceneNode);
        }

        // Clear the registry, which destroys all components
        mRegistry.clear();

        // Call base class cleanup
        TutorialGameState::destroyScene();
    };
    //-----------------------------------------------------------------------------------

    void EngineGameState::update(float timeSinceLast)
    {
        // First, call the base update
        TutorialGameState::update(timeSinceLast);

        // --- 1. Logic System: SpinSystem ---
        // This system updates the "data" (TransformComponent) based on "logic" (SpinComponent).
        // It knows nothing about Ogre.
        {
            auto spinView = mRegistry.view<TransformComponent, const SpinComponent>();
            for (auto [entity, transform, spin] : spinView.each())
            {
                Ogre::Quaternion rot(Ogre::Radian(spin.spinSpeed * timeSinceLast), Ogre::Vector3::UNIT_Y);
                transform.orientation = transform.orientation * rot;
            }
        }

        // --- 2. Render Sync System ---
        // This system reads the "data" (TransformComponent) and updates the
        // "render" (OgreRenderableComponent::sceneNode).
        {
            auto renderView = mRegistry.view<const OgreRenderableComponent, const TransformComponent>();
            for (auto [entity, renderable, transform] : renderView.each())
            {
                renderable.sceneNode->setPosition(transform.position);
                renderable.sceneNode->setOrientation(transform.orientation);
                renderable.sceneNode->setScale(transform.scale);
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nEnTT scene with ";
        outText += Ogre::StringConverter::toString(mRegistry.storage<entt::entity>().size());
        outText += " entities.";
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( ( arg.keysym.mod & ~( KMOD_NUM | KMOD_CAPS ) ) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        TutorialGameState::keyReleased( arg );
    }
}  // namespace Demo
