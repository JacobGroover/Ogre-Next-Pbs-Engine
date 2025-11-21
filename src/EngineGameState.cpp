
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
//#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

// Forward declaration for cleaner code
void processResourcesFromJson(const rapidjson::Document& document, Demo::GraphicsSystem* graphicsSystem);

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

    Ogre::ColourValue parseColourValue(const rapidjson::Value& val, bool includesAlpha = true) {
        if (val.IsArray()) {
            // Handle [r, g, b, a] (0.0 - 1.0)
            if ((includesAlpha && val.Size() != 4) || (!includesAlpha && val.Size() != 3)) {
                Ogre::LogManager::getSingleton().logMessage(
                    "WARNING: Failed to parse ColourValue from JSON array.", Ogre::LML_CRITICAL);
                return Ogre::ColourValue::White;
            }
            return Ogre::ColourValue(
                static_cast<Ogre::Real>(val[0].GetDouble()),
                static_cast<Ogre::Real>(val[1].GetDouble()),
                static_cast<Ogre::Real>(val[2].GetDouble()),
                includesAlpha ? static_cast<Ogre::Real>(val[3].GetDouble()) : 1.0f
            );
        }
        else if (val.IsObject()) {
            // Handle {"r": 255, "g": 0, "b": 0, "a": 255} (0 - 255)
            float r = val.HasMember("r") ? static_cast<float>(val["r"].GetInt()) / 255.0f : 1.0f;
            float g = val.HasMember("g") ? static_cast<float>(val["g"].GetInt()) / 255.0f : 1.0f;
            float b = val.HasMember("b") ? static_cast<float>(val["b"].GetInt()) / 255.0f : 1.0f;
            float a = (includesAlpha && val.HasMember("a")) ? static_cast<float>(val["a"].GetInt()) / 255.0f : 1.0f;
            return Ogre::ColourValue(r, g, b, a);
        }

        Ogre::LogManager::getSingleton().logMessage("WARNING: ColourValue is neither Array nor Object.", Ogre::LML_CRITICAL);
        return Ogre::ColourValue::White;
    }

    // Helper to process resources (Materials and Textures) from the JSON
    void processResourcesFromJson(const rapidjson::Document& document, Demo::GraphicsSystem* graphicsSystem)
    {
        if (!document.HasMember("resources") || !document["resources"].IsArray()) {
            return;
        }

        const auto& resources = document["resources"];
        Ogre::Root* root = graphicsSystem->getRoot();
        Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
        Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

        // --- Pass 1: Map TextureResource Names to File Paths ---
        // Editor definition: "Texture.9" -> "wood.jpg"
        std::map<std::string, std::string> textureNameToFileMap;

        for (rapidjson::SizeType i = 0; i < resources.Size(); ++i)
        {
            const auto& resData = resources[i];
            if (!resData.IsObject() || !resData.HasMember("type") || !resData["type"].IsString()) continue;

            std::string resType = resData["type"].GetString();
            std::string resName = resData.HasMember("name") ? resData["name"].GetString() : ("Resource_" + Ogre::StringConverter::toString(i));

            if (resType == "TextureResource")
            {
                if (resData.HasMember("path") && resData["path"].IsString()) {
                    textureNameToFileMap[resName] = resData["path"].GetString();
                }
            }
        }

        // Use LML_NORMAL so this appears in the log
        Ogre::LogManager::getSingleton().logMessage("Processing " + Ogre::StringConverter::toString(resources.Size()) + " resources.", Ogre::LML_NORMAL);

        // --- Pass 2: Create Materials using the Map ---
        for (rapidjson::SizeType i = 0; i < resources.Size(); ++i)
        {
            const auto& resData = resources[i];
            if (!resData.IsObject() || !resData.HasMember("type") || !resData["type"].IsString()) continue;

            std::string resType = resData["type"].GetString();
            std::string resName = resData.HasMember("name") ? resData["name"].GetString() : ("Resource_" + Ogre::StringConverter::toString(i));

            if (resType == "MaterialResource")
            {
                Ogre::HlmsPbsDatablock* pbsDatablock = nullptr;
                Ogre::HlmsDatablock* existing = hlmsPbs->getDatablock(resName);
                if (existing) {
                    pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(existing);
                }
                else {
                    pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock(resName, resName, Ogre::HlmsMacroblock(),
                            Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));
                }

                // --- Workflow Settings ---
                pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);

                // --- Base Properties ---
                if (resData.HasMember("roughness") && resData["roughness"].IsNumber()) {
                    pbsDatablock->setRoughness(static_cast<float>(resData["roughness"].GetDouble()));
                }
                else {
                    pbsDatablock->setRoughness(0.5f);
                }

                if (resData.HasMember("metallic") && resData["metallic"].IsNumber()) {
                    pbsDatablock->setMetalness(static_cast<float>(resData["metallic"].GetDouble()));
                }
                else {
                    pbsDatablock->setMetalness(0.0f);
                }

                // --- Colors (Albedo) ---
                if (resData.HasMember("albedo") && resData["albedo"].IsObject()) {
                    Ogre::ColourValue albedoColor = parseColourValue(resData["albedo"], true);
                    pbsDatablock->setDiffuse(Ogre::Vector3(albedoColor.r, albedoColor.g, albedoColor.b));
                    // Fix for dark rendering: set Specular to match Diffuse
                    pbsDatablock->setSpecular(Ogre::Vector3(albedoColor.r, albedoColor.g, albedoColor.b));
                }

                // --- Texture Maps ---
                // Helper lambda: Strips "res://" and looks up the actual filename
                auto resolveTextureFile = [&](const rapidjson::Value& val) -> std::string {
                    std::string refName = val.GetString();
                    if (refName.rfind("res://", 0) == 0) refName = refName.substr(6);

                    // Look up the actual file path
                    if (textureNameToFileMap.find(refName) != textureNameToFileMap.end()) {
                        return textureNameToFileMap[refName];
                    }
                    return refName; // Fallback
                    };

                if (resData.HasMember("albedoTexture") && resData["albedoTexture"].IsString() && resData["albedoTexture"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["albedoTexture"]);
                    if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_DIFFUSE, filename);
                }
                if (resData.HasMember("normalMap") && resData["normalMap"].IsString() && resData["normalMap"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["normalMap"]);
                    if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_NORMAL, filename);
                }
                if (resData.HasMember("metallicMap") && resData["metallicMap"].IsString() && resData["metallicMap"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["metallicMap"]);
                    if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_METALLIC, filename);
                }
                if (resData.HasMember("roughnessMap") && resData["roughnessMap"].IsString() && resData["roughnessMap"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["roughnessMap"]);
                    if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_ROUGHNESS, filename);
                }

                Ogre::LogManager::getSingleton().logMessage("Configured Material: " + resName, Ogre::LML_NORMAL);
            }
        }
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

        // --- 1. Load JSON File ---
        std::string jsonData;
        std::ifstream inFile(filename.c_str(), std::ios::binary | std::ios::in);

        if (inFile.is_open())
        {
            inFile.seekg(0, std::ios::end);
            jsonData.resize(static_cast<size_t>(inFile.tellg()));
            inFile.seekg(0, std::ios::beg);
            inFile.read(&jsonData[0], static_cast<std::streamsize>(jsonData.size()));
            inFile.close();
        }
        else
        {
            Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(
                filename, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, true);
            if (!stream)
                OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "Could not open scene file: " + filename, "EngineGameState::loadSceneFromJson");
            jsonData = stream->getAsString();
            stream->close();
        }

        // --- 2. Parse JSON ---
        rapidjson::Document document;
        document.Parse(jsonData.c_str());

        if (document.HasParseError())
        {
            Ogre::LogManager::getSingleton().logMessage("JSON Parse Error: " +
                std::string(rapidjson::GetParseError_En(document.GetParseError())), Ogre::LML_CRITICAL);
            return;
        }

        // --- 2.5. Register Project Path as Resource Location ---
        // This ensures textures referenced in the JSON can be found by Ogre
        if (document.HasMember("path") && document["path"].IsString()) {
            std::string projectPath = document["path"].GetString();
            if (!projectPath.empty()) {
                Ogre::ResourceGroupManager& rgm = Ogre::ResourceGroupManager::getSingleton();
                Ogre::String groupName = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;

                // Add the location
                rgm.addResourceLocation(projectPath, "FileSystem", groupName);
                Ogre::LogManager::getSingleton().logMessage("Registered project resource path: " + projectPath);

                // FIX: Check if initialized to avoid runtime crash, and provide the missing boolean argument
                if (!rgm.isResourceGroupInitialised(groupName)) {
                    rgm.initialiseResourceGroup(groupName, true);
                }
                // Note: If the group is already initialized, addResourceLocation still allows 
                // textures to be found immediately, so no else/re-init is required for textures.
            }
        }

        // --- 3. Process Resources (Materials) ---
        // Must be done before processing nodes so materials are available
        processResourcesFromJson(document, mGraphicsSystem);

        // --- 4. Process Nodes ---
        if (document.HasMember("nodes") && document["nodes"].IsArray())
        {
            const auto& nodes = document["nodes"];
            Ogre::LogManager::getSingleton().logMessage("Processing " + Ogre::StringConverter::toString(nodes.Size()) + " nodes.", Ogre::LML_TRIVIAL);

            for (rapidjson::SizeType i = 0; i < nodes.Size(); ++i)
            {
                const auto& nodeData = nodes[i];
                if (!nodeData.IsObject() || !nodeData.HasMember("type") || !nodeData["type"].IsString()) continue;

                Ogre::String nodeType = nodeData["type"].GetString();

                // --- 4a. Handle SceneNode (Global Settings) ---
                if (nodeType == "SceneNode")
                {
                    // SceneNode contains ambientLight (default 0.5)
                    if (nodeData.HasMember("ambientLight") && nodeData["ambientLight"].IsNumber())
                    {
                        float ambientScale = static_cast<float>(nodeData["ambientLight"].GetDouble());

                        // Apply this scale to the existing ambient light settings
                        // Note: We use the upper hemisphere as a baseline reference
                        Ogre::ColourValue upperHemi = sceneManager->getAmbientLightUpperHemisphere();
                        Ogre::ColourValue lowerHemi = sceneManager->getAmbientLightLowerHemisphere();
                        Ogre::Vector3 dir = sceneManager->getAmbientLightHemisphereDir();

                        // Assuming current values are "base" values, we scale them by the factor from JSON
                        // Multiplying by 2.0 because 0.5 is default in editor, resulting in 1.0 multiplier
                        sceneManager->setAmbientLight(upperHemi * ambientScale * 2.0f, lowerHemi * ambientScale * 2.0f, dir);
                    }
                    continue; // SceneNode is not an entity with a transform, so we skip the rest
                }

                // --- 4b. Extract Transform Data (Node3D interface) ---
                Ogre::Vector3 pos = Ogre::Vector3::ZERO;
                Ogre::Vector3 scale = Ogre::Vector3::UNIT_SCALE;
                Ogre::Quaternion ori = Ogre::Quaternion::IDENTITY;

                if (nodeData.HasMember("positionX")) pos.x = static_cast<Ogre::Real>(nodeData["positionX"].GetDouble());
                if (nodeData.HasMember("positionY")) pos.y = static_cast<Ogre::Real>(nodeData["positionY"].GetDouble());
                if (nodeData.HasMember("positionZ")) pos.z = static_cast<Ogre::Real>(nodeData["positionZ"].GetDouble());

                if (nodeData.HasMember("scaleX")) scale.x = static_cast<Ogre::Real>(nodeData["scaleX"].GetDouble());
                if (nodeData.HasMember("scaleY")) scale.y = static_cast<Ogre::Real>(nodeData["scaleY"].GetDouble());
                if (nodeData.HasMember("scaleZ")) scale.z = static_cast<Ogre::Real>(nodeData["scaleZ"].GetDouble());

                Ogre::Radian rotX(0), rotY(0), rotZ(0);
                if (nodeData.HasMember("rotationX")) rotX = Ogre::Degree(static_cast<Ogre::Real>(nodeData["rotationX"].GetDouble()));
                if (nodeData.HasMember("rotationY")) rotY = Ogre::Degree(static_cast<Ogre::Real>(nodeData["rotationY"].GetDouble()));
                if (nodeData.HasMember("rotationZ")) rotZ = Ogre::Degree(static_cast<Ogre::Real>(nodeData["rotationZ"].GetDouble()));

                // Z-X-Y Euler order
                ori = Ogre::Quaternion(rotY, Ogre::Vector3::UNIT_Y) * Ogre::Quaternion(rotX, Ogre::Vector3::UNIT_X) * Ogre::Quaternion(rotZ, Ogre::Vector3::UNIT_Z);

                // --- 4c. Create EnTT Entity ---
                entt::entity entity = mRegistry.create();
                mRegistry.emplace<TransformComponent>(entity, pos, ori, scale);

                // --- 4d. Node Type Dispatch ---
                if (nodeType == "PrimitiveNode")
                {
                    std::string meshName = "Cube_d.mesh";
                    std::string materialName = "BaseWhite";

                    if (nodeData.HasMember("meshType") && nodeData["meshType"].IsInt()) {
                        int meshType = nodeData["meshType"].GetInt();
                        if (meshType == 0) meshName = "Cube_d.mesh";
                        else if (meshType == 1) meshName = "Sphere1000.mesh";
                    }

                    // Strip "res://" prefix to find Ogre datablock
                    if (nodeData.HasMember("material") && nodeData["material"].IsString()) {
                        materialName = nodeData["material"].GetString();
                        if (materialName.rfind("res://", 0) == 0) {
                            materialName = materialName.substr(6);
                        }
                    }

                    Ogre::Item* item = sceneManager->createItem(meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
                    item->setDatablockOrMaterialName(materialName);
                    Ogre::SceneNode* sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
                    sceneNode->attachObject(item);

                    sceneNode->setPosition(pos);
                    sceneNode->setOrientation(ori);
                    sceneNode->setScale(scale);

                    mRegistry.emplace<OgreRenderableComponent>(entity, item, sceneNode);
                }
                else if (nodeType == "CameraNode")
                {
                    if (nodeData.HasMember("fov") && nodeData["fov"].IsNumber()) {
                        Ogre::Real fov = static_cast<Ogre::Real>(nodeData["fov"].GetDouble());
                        mGraphicsSystem->getCamera()->setFOVy(Ogre::Degree(fov));
                    }
                    mGraphicsSystem->getCamera()->setPosition(pos);
                    mGraphicsSystem->getCamera()->setOrientation(ori);
                }
                else if (nodeType == "LightNode")
                {
                    Ogre::Light* light = sceneManager->createLight();
                    Ogre::SceneNode* lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
                    lightNode->attachObject(light);
                    lightNode->setPosition(pos);

                    // LightType Enum: Point=0, Sun=1
                    if (nodeData.HasMember("lightType") && nodeData["lightType"].IsInt()) {
                        int typeVal = nodeData["lightType"].GetInt();
                        if (typeVal == 1) { // Sun
                            light->setType(Ogre::Light::LT_DIRECTIONAL);
                            // Directional lights use orientation (Forward is negative Z)
                            light->setDirection((ori * Ogre::Vector3::NEGATIVE_UNIT_Z).normalisedCopy());
                        }
                        else { // Point (0)
                            light->setType(Ogre::Light::LT_POINT);
                        }
                    }

                    if (nodeData.HasMember("color") && nodeData["color"].IsObject()) {
                        Ogre::ColourValue col = parseColourValue(nodeData["color"], true);
                        light->setDiffuseColour(col);
                        light->setSpecularColour(col);
                    }

                    // Intensity mapping
                    if (nodeData.HasMember("intensity") && nodeData["intensity"].IsNumber()) {
                        Ogre::Real intensity = static_cast<Ogre::Real>(nodeData["intensity"].GetDouble());
                        light->setPowerScale(intensity * 5.0f); // Boost for visibility
                    }
                    else {
                        light->setPowerScale(5.0f);
                    }
                }
            }
        }

        Ogre::LogManager::getSingleton().logMessage("Finished loading scene.", Ogre::LML_TRIVIAL);
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
