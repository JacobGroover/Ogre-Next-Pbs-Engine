
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

                // ---------------------------------------------------------
                // SANITIZE (Zero out previous state)
                // ---------------------------------------------------------
                // We clear ALL relevant texture slots to prevent Ghost Textures from previous loads.
                // This ensures we start with a clean slate before applying new JSON data.
                /*pbsDatablock->setTexture(Ogre::PBSM_DIFFUSE, static_cast<Ogre::TextureGpu*>(nullptr));
                pbsDatablock->setTexture(Ogre::PBSM_NORMAL, static_cast<Ogre::TextureGpu*>(nullptr));
                pbsDatablock->setTexture(Ogre::PBSM_METALLIC, static_cast<Ogre::TextureGpu*>(nullptr));
                pbsDatablock->setTexture(Ogre::PBSM_ROUGHNESS, static_cast<Ogre::TextureGpu*>(nullptr));
                pbsDatablock->setTexture(Ogre::PBSM_DETAIL0, static_cast<Ogre::TextureGpu*>(nullptr));
                pbsDatablock->setTexture(Ogre::PBSM_DETAIL1, static_cast<Ogre::TextureGpu*>(nullptr));
                pbsDatablock->setTexture(Ogre::PBSM_DETAIL_WEIGHT, static_cast<Ogre::TextureGpu*>(nullptr));*/

                // --- Workflow Settings ---
                pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);

                // ---------------------------------------------------------
                // Base Properties
                // ---------------------------------------------------------
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
                    //pbsDatablock->setDiffuse(Ogre::Vector3(albedoColor.r, albedoColor.g, albedoColor.b));
                    pbsDatablock->setBackgroundDiffuse(albedoColor);
                    // Fix for dark rendering: set Specular to match Diffuse
                    //pbsDatablock->setSpecular(Ogre::Vector3(albedoColor.r, albedoColor.g, albedoColor.b));
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

                // ---------------------------------------------------------
                // Standard Textures (Albedo & Normal)
                // ---------------------------------------------------------
                if (resData.HasMember("albedoTexture") && resData["albedoTexture"].IsString() && resData["albedoTexture"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["albedoTexture"]);
                    if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_DIFFUSE, filename);
                }

                if (resData.HasMember("normalMap") && resData["normalMap"].IsString() && resData["normalMap"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["normalMap"]);
                    if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_NORMAL, filename);
                }

                // ---------------------------------------------------------
                // PBR Workflow (Packed ORM vs. Separate Maps)
                // ---------------------------------------------------------

                // Load Packed ORM (Occlusion, Roughness, Metallic)
                // Determine if a packed map is used for AO, Roughness, and Metallic
                // If so, set that first for AO, Roughness, and Metallic channels
                bool hasOrmMap = false;
                if (resData.HasMember("packedMap") && resData["packedMap"].IsString() && resData["packedMap"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["packedMap"]);
                    if (!filename.empty()) {
                        // Ogre PBS Metallic Workflow: 
                        // Specular Texture Red = AO
                        // Specular Texture Green = Roughness
                        // Specular Texture Blue = Metallic

                        Ogre::LogManager::getSingleton().logMessage("Assigning Packed Map: " + filename);

                        // Assign/bind to DETAIL0 (Slot 6)
                        pbsDatablock->setTexture(Ogre::PBSM_DETAIL0, filename);
                        //pbsDatablock->setTexture(Ogre::PBSM_DETAIL_WEIGHT, filename);
                        //pbsDatablock->setCustomPieceFile("Custom_ORM_piece_ps.any", );

                        // Configure UVs (Critical to prevent black render)
                        // Tell Ogre to use UV Set 0 for Detail Map 0.
                        pbsDatablock->setTextureUvSource(Ogre::PBSM_DETAIL0, 0);
                        //pbsDatablock->setTextureUvSource(Ogre::PBSM_DETAIL_WEIGHT, 0);

                        /*if (pbsDatablock->getTexture(Ogre::PBSM_DIFFUSE) == nullptr)
                        {
                            pbsDatablock->setTexture(Ogre::PBSM_DIFFUSE, filename);
                        }*/

                        pbsDatablock->setDetailMapBlendMode(0, Ogre::PBSM_BLEND_NORMAL_NON_PREMUL);

                        hasOrmMap = true;

                        pbsDatablock->setTexture(Ogre::PBSM_DETAIL0, static_cast<Ogre::TextureGpu*>(nullptr));
                    }
                }

                // Fallback: Set AO, Roughness, and Metallic maps individually if not using packed map
                if (!hasOrmMap)
                {
                    /*if (resData.HasMember("ambientOcclusionMap") && resData["ambientOcclusionMap"].IsString() && resData["ambientOcclusionMap"].GetStringLength() > 0) {
                        std::string filename = resolveTextureFile(resData["ambientOcclusionMap"]);
                        if (!filename.empty()) {
                            pbsDatablock->setTexture(Ogre::PBSM_DETAIL_WEIGHT, filename);   // This currently does nothing in ogre-next unless a custom shader is used (ogre-next has no native AO implementation)
                        }
                    }*/
                    if (resData.HasMember("metallicMap") && resData["metallicMap"].IsString() && resData["metallicMap"].GetStringLength() > 0) {
                        std::string filename = resolveTextureFile(resData["metallicMap"]);
                        if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_METALLIC, filename);
                    }
                    if (resData.HasMember("roughnessMap") && resData["roughnessMap"].IsString() && resData["roughnessMap"].GetStringLength() > 0) {
                        std::string filename = resolveTextureFile(resData["roughnessMap"]);
                        if (!filename.empty()) pbsDatablock->setTexture(Ogre::PBSM_ROUGHNESS, filename);
                    }
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
                        float intensity = static_cast<float>(nodeData["ambientLight"].GetDouble());

                        // Create a color from the intensity value directly
                        Ogre::ColourValue ambientColor(intensity, intensity, intensity);

                        // Apply to both hemispheres (Upper and Lower)
                        // Can make the Lower hemisphere darker (e.g., ambientColor * 0.6f) to simulate ground absorption for more realism.
                        sceneManager->setAmbientLight(
                            ambientColor,           // Upper Hemisphere
                            ambientColor,           // Lower Hemisphere
                            Ogre::Vector3::UNIT_Y,  // Hemisphere Direction (Up)
                            1.0f                    // Envmap Scale
                        );

                        Ogre::LogManager::getSingleton().logMessage(
                            "Set Ambient Light to: " + Ogre::StringConverter::toString(intensity));
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
                    std::string materialName = "";

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

                    // Only set the datablock if we actually found a name in the JSON. 
                    // Otherwise, Ogre::Item uses its default datablock automatically.
                    if (!materialName.empty()) {
                        item->setDatablockOrMaterialName(materialName);
                    }

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
        // 1. Parse command line arguments to get scene file
        parseCommandLineArgs(mArgc, mArgv);

        // 2. Load Scene from JSON
        try {
            loadSceneFromJson(mSceneToLoad); // Assuming the file is passed as a command line argument, or is in a location Ogre can find (e.g., bin/Data if added as resource path)
        }
        catch (Ogre::Exception& e) {
            Ogre::LogManager::getSingleton().logMessage("Failed to load scene from JSON: " + e.getFullDescription(), Ogre::LML_CRITICAL);
            // Handle the error, maybe load a fallback scene or show an error message
            // For now, we'll just log and continue with an potentially empty scene
        }


        // 3. Set up Camera Controller (after potential camera setup from JSON)
        // Camera position might be overridden by JSON, so create controller after loading
        mCameraController = new CameraController(mGraphicsSystem, false);

        // 4. Call base class setup AFTER loading our scene, which initializes debug text
        TutorialGameState::createScene01();

        // [DEBUG START] ISOLATE AMBIENT OCCLUSION
        //Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();

        //// 1. Set a flat, mid-grey Ambient Light. 
        //// If AO is working, the object will be grey with BLACK cracks.
        //// If AO is NOT working, the object will be completely flat grey.
        //sceneManager->setAmbientLight(
        //    Ogre::ColourValue(0.5f, 0.5f, 0.5f), // Upper Hemisphere
        //    Ogre::ColourValue(0.5f, 0.5f, 0.5f), // Lower Hemisphere
        //    Ogre::Vector3::UNIT_Y
        //);

        //// 2. Disable or Dim the Sun (Directional Lights)
        //// We iterate through lights to turn off the sun loaded from JSON
        //Ogre::SceneManager::MovableObjectIterator itor = sceneManager->getMovableObjectIterator("Light");
        //while (itor.hasMoreElements())
        //{
        //    Ogre::Light* light = static_cast<Ogre::Light*>(itor.getNext());
        //    if (light->getType() == Ogre::Light::LT_DIRECTIONAL)
        //    {
        //        light->setPowerScale(0.0f); // Turn off the sun
        //    }
        //}
        // [DEBUG END]
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
