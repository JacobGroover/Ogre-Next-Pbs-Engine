#include "EngineGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreItem.h"
#include "OgreLight.h"

#include "OgreOverlay.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayManager.h"
#include "OgreTextAreaOverlayElement.h"

#include "OgreFrameStats.h"
#include "OgreRoot.h"

// Ogre-Next HLMS includes
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreGpuProgramManager.h"
#include "OgreHlmsCompute.h"
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

#include "rapidjson/error/en.h"

// Forward declaration for cleaner code
void Demo::processResourcesFromJson(const rapidjson::Document& document, Demo::GraphicsSystem* graphicsSystem);

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
        // Editor definition mapping: "Texture.9" -> "wood.jpg"
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
                    pbsDatablock->setBackgroundDiffuse(albedoColor);
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
                if (resData.HasMember("ambientOcclusionMap") && resData["ambientOcclusionMap"].IsString() && resData["ambientOcclusionMap"].GetStringLength() > 0) {
                    std::string filename = resolveTextureFile(resData["ambientOcclusionMap"]);
                    if (!filename.empty()) {
                        pbsDatablock->setTexture(Ogre::PBSM_AO, filename);
                    }
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
        mGraphicsSystem( 0 ),
        mSceneToLoad("scene.json"),
        mArgc(argc),
        mArgv(argv),
        mHelpDescription(helpDescription),
        mDisplayHelpMode(1),
        mNumDisplayHelpModes(2),
        mCameraController( 0 ),
        mDebugText( 0 ),
		mDebugTextShadow( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    EngineGameState::~EngineGameState()
    {
        // Cleanup CameraController if it wasn't already cleaned up in destroyScene
        if (mCameraController)
            delete mCameraController;
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::_notifyGraphicsSystem(GraphicsSystem* graphicsSystem)
    {
        mGraphicsSystem = graphicsSystem;
	}
    //-----------------------------------------------------------------------------------
    void EngineGameState::loadSceneFromJson(const Ogre::String& filename)
    {
        Ogre::LogManager::getSingleton().logMessage("Loading scene from JSON: " + filename, Ogre::LML_TRIVIAL);
        Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();

        // ---  Load JSON File ---
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

        // --- Parse JSON ---
        rapidjson::Document document;
        document.Parse(jsonData.c_str());

        if (document.HasParseError())
        {
            Ogre::LogManager::getSingleton().logMessage("JSON Parse Error: " +
                std::string(rapidjson::GetParseError_En(document.GetParseError())), Ogre::LML_CRITICAL);
            return;
        }

        // --- Register Project Path as Resource Location ---
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

        // --- Process Resources (Materials) ---
        // Must be done before processing nodes so materials are available
        processResourcesFromJson(document, mGraphicsSystem);

        // --- Process Nodes ---
        if (document.HasMember("nodes") && document["nodes"].IsArray())
        {
            const auto& nodes = document["nodes"];

            for (rapidjson::SizeType i = 0; i < nodes.Size(); ++i)
            {
                const auto& nodeData = nodes[i];
                if (!nodeData.IsObject() || !nodeData.HasMember("type") || !nodeData["type"].IsString()) continue;

                Ogre::String nodeType = nodeData["type"].GetString();

                // --- Handle SceneNode (Global Settings) ---
                if (nodeType == "SceneNode")
                {
                    if (nodeData.HasMember("ambientLight") && nodeData["ambientLight"].IsNumber())
                    {
                        float intensity = static_cast<float>(nodeData["ambientLight"].GetDouble());

                        // Create a color from the intensity value directly
                        Ogre::ColourValue ambientColor(intensity, intensity, intensity);
                        sceneManager->setAmbientLight(ambientColor, ambientColor, Ogre::Vector3::UNIT_Y, 1.0f);
                    }
                    continue; // SceneNode is not an entity with a transform, so we skip the rest
                }

                // --- Extract Transform Data (Node3D interface) ---
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

                // --- Create EnTT Entity ---
                entt::entity entity = mRegistry.create();
                mRegistry.emplace<TransformComponent>(entity, pos, ori, scale);

                // --- Create the SceneNode (Common to all types) ---
                // Note: We attach the specific object (Item/Light/Camera etc.) to this node below
                Ogre::SceneNode* sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
                sceneNode->setPosition(pos);
                sceneNode->setOrientation(ori);
                sceneNode->setScale(scale);

                // Register the SceneNodeComponent so it gets updated (so the Render Sync System can find and update it)
                mRegistry.emplace<SceneNodeComponent>(entity, sceneNode);

                // --- Node Type Dispatch ---
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

                    sceneNode->attachObject(item);

                    // Add specific Item component to ECS registry
                    mRegistry.emplace<ItemComponent>(entity, item);
                }
                else if (nodeType == "CameraNode")
                {
                    if (nodeData.HasMember("fov") && nodeData["fov"].IsNumber()) {
                        Ogre::Real fov = static_cast<Ogre::Real>(nodeData["fov"].GetDouble());
                        mGraphicsSystem->getCamera()->setFOVy(Ogre::Degree(fov));
                    }
                    mGraphicsSystem->getCamera()->setPosition(pos);
                    mGraphicsSystem->getCamera()->setOrientation(ori);
                    // Usually main camera isn't attached to a node in this specific demo structure,
                    // but if it were a game object camera:
                    // mRegistry.emplace<CameraComponent>(entity, mGraphicsSystem->getCamera());
                }
                else if (nodeType == "LightNode")
                {
                    Ogre::Light* light = sceneManager->createLight();
                    sceneNode->attachObject(light);

                    // LightType Enum: Point=0, Sun=1
                    if (nodeData.HasMember("lightType") && nodeData["lightType"].IsInt()) {
                        int typeVal = nodeData["lightType"].GetInt();
                        if (typeVal == 1) { // Sun
                            light->setType(Ogre::Light::LT_DIRECTIONAL);
                            // Directional lights use orientation (Forward is negative Z)
                            light->setDirection((ori * Ogre::Vector3::NEGATIVE_UNIT_Z).normalisedCopy());
                        }
                        else { // Point
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
                        light->setPowerScale(intensity * 1.0f);
                    }
                    else {
                        light->setPowerScale(1.0f);
                    }

                    // Add specific Light component to ECS registry
                    mRegistry.emplace<LightComponent>(entity, light);
                }
            }
        }
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
            loadSceneFromJson(mSceneToLoad);
        }
        catch (Ogre::Exception& e) {
            Ogre::LogManager::getSingleton().logMessage("Failed to load scene from JSON: " + e.getFullDescription(), Ogre::LML_CRITICAL);
            // Handle the error, maybe load a fallback scene or show an error message
            // For now, we'll just log and continue with an potentially empty scene
        }

        // 3. Set up Camera Controller (after potential camera setup from JSON)
        // Camera position might be overridden by JSON, so create controller after loading
        mCameraController = new CameraController(mGraphicsSystem, false);

        // 4. Create Debug Text Overlay
        createDebugTextOverlay();
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::createDebugTextOverlay()
    {
        Ogre::v1::OverlayManager& overlayManager = Ogre::v1::OverlayManager::getSingleton();
        Ogre::v1::Overlay* overlay = overlayManager.create("DebugText");

        Ogre::v1::OverlayContainer* panel = static_cast<Ogre::v1::OverlayContainer*>(
            overlayManager.createOverlayElement("Panel", "DebugPanel"));
        mDebugText = static_cast<Ogre::v1::TextAreaOverlayElement*>(
            overlayManager.createOverlayElement("TextArea", "DebugText"));
        mDebugText->setFontName("DebugFont");
        mDebugText->setCharHeight(0.025f);

        mDebugTextShadow = static_cast<Ogre::v1::TextAreaOverlayElement*>(
            overlayManager.createOverlayElement("TextArea", "0DebugTextShadow"));
        mDebugTextShadow->setFontName("DebugFont");
        mDebugTextShadow->setCharHeight(0.025f);
        mDebugTextShadow->setColour(Ogre::ColourValue::Black);
        mDebugTextShadow->setPosition(0.002f, 0.002f);

        panel->addChild(mDebugTextShadow);
        panel->addChild(mDebugText);
        overlay->add2D(panel);
        overlay->show();
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::destroyScene()
    {
        Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();

        // Destroy Items (Meshes)
        auto itemView = mRegistry.view<ItemComponent>();
        for (auto [entity, itemComp] : itemView.each())
        {
            sceneManager->destroyItem(itemComp.item);
        }

        // Destroy Lights
        auto lightView = mRegistry.view<LightComponent>();
        for (auto [entity, lightComp] : lightView.each())
        {
            sceneManager->destroyLight(lightComp.light);
        }

        // Destroy Cameras
        auto camView = mRegistry.view<CameraComponent>();
        for (auto [entity, camComp] : camView.each())
        {
            sceneManager->destroyCamera(camComp.camera);
        }

        // Destroy SceneNodes
        // We do this LAST. Since we destroyed the attached objects above, 
        // the nodes are empty and safe to destroy.
        auto nodeView = mRegistry.view<SceneNodeComponent>();
        for (auto [entity, nodeComp] : nodeView.each())
        {
            sceneManager->destroySceneNode(nodeComp.sceneNode);
        }

        // Wipe Registry
        mRegistry.clear();

        if (mCameraController)
        {
            delete mCameraController;
            mCameraController = nullptr;
        }
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::update(float timeSinceLast)
    {
        if (mDisplayHelpMode != 0)
        {
            // Show FPS
            Ogre::String finalText;
            generateDebugText(timeSinceLast, finalText);
            mDebugText->setCaption(finalText);
            mDebugTextShadow->setCaption(finalText);
        }

        if (mCameraController)
            mCameraController->update(timeSinceLast);

        // --- 1. Logic System: SpinSystem ---
        {
            auto spinView = mRegistry.view<TransformComponent, const SpinComponent>();
            for (auto&& [entity, transform, spin] : spinView.each())
            {
                Ogre::Quaternion rot(Ogre::Radian(spin.spinSpeed * timeSinceLast), Ogre::Vector3::UNIT_Y);
                transform.orientation = transform.orientation * rot;
            }
        }

        // --- 2. Render Sync System ---
        {
            auto renderView = mRegistry.view<const SceneNodeComponent, const TransformComponent>();
            for (auto [entity, nodeComp, transform] : renderView.each())
            {
                nodeComp.sceneNode->setPosition(transform.position);
                nodeComp.sceneNode->setOrientation(transform.orientation);
                nodeComp.sceneNode->setScale(transform.scale);
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::generateDebugText( float timeSinceLast, Ogre::String& outText )
    {
        if (mDisplayHelpMode == 0)
        {
            outText = mHelpDescription;
            outText += "\n\nPress F1 to toggle help\n\n";
            outText +=
                "WASD  : Move Camera\n"
                "Shift : Speed Boost\n"
                "Mouse : Look Around\n"
                "Q     : Toggle Spin on all objects (cubes and spheres, not lights or cameras)\n";
                /*"\n\nProtip: Ctrl+F1 will reload PBS shaders (for real time template editing).\n"
                "Ctrl+F2 reloads Unlit shaders.\n"
                "Ctrl+F3 reloads Compute shaders.\n"
                "Note: If the modified templates produce invalid shader code, "
                "crashes or exceptions can happen.\n";*/
            return;
        }

        const Ogre::FrameStats* frameStats = mGraphicsSystem->getRoot()->getFrameStats();

        Ogre::String finalText;
        finalText.reserve(128);
        finalText = "Frame time:\t";
        finalText += Ogre::StringConverter::toString(timeSinceLast * 1000.0f);
        finalText += " ms\n";
        finalText += "Frame FPS:\t";
        finalText += Ogre::StringConverter::toString(1.0f / timeSinceLast);
        finalText += "\nAvg time:\t";
        finalText += Ogre::StringConverter::toString(frameStats->getRollingAverage() * 1000.0);
        finalText += " ms\n";
        finalText += "Avg FPS:\t";
        finalText += Ogre::StringConverter::toString(frameStats->getRollingAverageFps());
        finalText += "\n\nPress F1 to toggle help";

        finalText += "\n\nEnTT scene with ";
        finalText += Ogre::StringConverter::toString(mRegistry.storage<entt::entity>().size());
        finalText += " entities.";
        finalText += "\n\nPress ESC key to exit";

        outText.swap(finalText);

        mDebugText->setCaption(finalText);
        mDebugTextShadow->setCaption(finalText);
    }
    //-----------------------------------------------------------------------------------
    // INPUT HANDLING
    //-----------------------------------------------------------------------------------
    void EngineGameState::keyPressed(const SDL_KeyboardEvent& arg)
    {
        // Press Q to toggle spinning on all scene objects
        if (arg.keysym.sym == SDLK_q)
        {
            // Iterate over all ItemComponent entities that have a SceneNode attached 
            auto view = mRegistry.view<ItemComponent>();

            for (auto entity : view)
            {
                // Check if the entity already has a SpinComponent
                if (mRegistry.all_of<SpinComponent>(entity))
                {
                    // If it does, stop spinning by removing the component
                    mRegistry.remove<SpinComponent>(entity);
                }
                else
                {
                    // If it doesn't, start spinning by adding the component
                    // Speed: 1.0 radian per second
                    mRegistry.emplace<SpinComponent>(entity, 1.0f);
                }
            }
        }

        bool handledEvent = false;

        if (mCameraController)
            handledEvent = mCameraController->keyPressed(arg);

        if (!handledEvent)
            GameState::keyPressed(arg);
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::keyReleased(const SDL_KeyboardEvent& arg)
    {
        // ESC Key to Exit
        if (arg.keysym.scancode == SDL_SCANCODE_ESCAPE)
        {
            mGraphicsSystem->setQuit();
        }

        if (mCameraController)
            mCameraController->keyReleased(arg);

        if (arg.keysym.scancode == SDL_SCANCODE_F1 &&
            (arg.keysym.mod & ~(KMOD_NUM | KMOD_CAPS)) == 0)
        {
            mDisplayHelpMode = (mDisplayHelpMode + 1) % mNumDisplayHelpModes;

            Ogre::String finalText;
            generateDebugText(0, finalText);
            mDebugText->setCaption(finalText);
            mDebugTextShadow->setCaption(finalText);
        }
        else if (arg.keysym.scancode == SDL_SCANCODE_F1 &&
            (arg.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
        {
            // Hot reload of PBS shaders. We need to clear the microcode cache
            // to prevent using old compiled versions.
            Ogre::Root* root = mGraphicsSystem->getRoot();
            Ogre::HlmsManager* hlmsManager = root->getHlmsManager();

            Ogre::Hlms* hlms = hlmsManager->getHlms(Ogre::HLMS_PBS);
            Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom(hlms->getDataFolder());
        }
        else if (arg.keysym.scancode == SDL_SCANCODE_F2 &&
            (arg.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
        {
            // Hot reload of Unlit shaders.
            Ogre::Root* root = mGraphicsSystem->getRoot();
            Ogre::HlmsManager* hlmsManager = root->getHlmsManager();

            Ogre::Hlms* hlms = hlmsManager->getHlms(Ogre::HLMS_UNLIT);
            Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom(hlms->getDataFolder());
        }
        else if (arg.keysym.scancode == SDL_SCANCODE_F3 &&
            (arg.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
        {
            // Hot reload of Compute shaders.
            Ogre::Root* root = mGraphicsSystem->getRoot();
            Ogre::HlmsManager* hlmsManager = root->getHlmsManager();

            Ogre::Hlms* hlms = hlmsManager->getComputeHlms();
            Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom(hlms->getDataFolder());
        }
        else if (arg.keysym.scancode == SDL_SCANCODE_F5 &&
            (arg.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
        {
            // Force device reelection
            Ogre::Root* root = mGraphicsSystem->getRoot();
            root->getRenderSystem()->validateDevice(true);
        }
        else
        {
            bool handledEvent = false;

            if (mCameraController)
                handledEvent = mCameraController->keyReleased(arg);

            if (!handledEvent)
                GameState::keyReleased(arg);
        }
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::mouseMoved( const SDL_Event& arg )
    {
        if ( mCameraController )
            mCameraController->mouseMoved ( arg );

        GameState::mouseMoved( arg );
    }
    //-----------------------------------------------------------------------------------

    /*
    void EngineGameState::mouseMoved(const SDL_MouseMotionEvent& arg)
    {
        if (mCameraController)
            mCameraController->mouseMoved(arg);
    }

    void EngineGameState::mousePressed(const SDL_MouseButtonEvent& arg)
    {
        if (mCameraController)
            mCameraController->mousePressed(arg);
    }

    void EngineGameState::mouseReleased(const SDL_MouseButtonEvent& arg)
    {
        if (mCameraController)
            mCameraController->mouseReleased(arg);
    }

    void EngineGameState::textInput(const SDL_TextInputEvent& arg)
    {
        if (mCameraController)
            mCameraController->textInput(arg);
    }

    void EngineGameState::controllerAxisMoved(const SDL_ControllerAxisEvent& arg)
    {
        if (mCameraController)
            mCameraController->controllerAxisMoved(arg);
    }

    void EngineGameState::controllerButtonPressed(const SDL_ControllerButtonEvent& arg)
    {
        if (mCameraController)
            mCameraController->controllerButtonPressed(arg);
    }

    void EngineGameState::controllerButtonReleased(const SDL_ControllerButtonEvent& arg)
    {
        if (mCameraController)
            mCameraController->controllerButtonReleased(arg);
    }
    */

}  // namespace Demo