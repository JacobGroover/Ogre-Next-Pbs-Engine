
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

using namespace Demo;

namespace Demo
{
    EngineGameState::EngineGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
    }
    //-----------------------------------------------------------------------------------
    void EngineGameState::createScene01()
    {
        Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();

        // --- 1. Set up Lighting (from PbsMaterials sample) ---

        // Set ambient light
        sceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f,
            Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f,
            Ogre::Vector3::UNIT_Y);

        // Create a directional light
        Ogre::Light* light = sceneManager->createLight();
        Ogre::SceneNode* lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject(light);
        light->setPowerScale(1.0f);
        light->setType(Ogre::Light::LT_DIRECTIONAL);
        light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());

        // --- 2. Create HLMS Materials (from PbsMaterials sample) ---

        Ogre::Root* root = mGraphicsSystem->getRoot();
        Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
        Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

        // Create a "RedPlastic" material
        Ogre::String datablockName = "RedPlastic";
        Ogre::HlmsPbsDatablock* pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
            hlmsPbs->createDatablock(datablockName,
                datablockName,
                Ogre::HlmsMacroblock(),
                Ogre::HlmsBlendblock(),
                Ogre::HlmsParamVec()));

		pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);    // Use metallic workflow to avoid throwing assert

        pbsDatablock->setDiffuse(Ogre::Vector3(1.0f, 0.0f, 0.0f));
        pbsDatablock->setRoughness(0.2f);
        pbsDatablock->setMetalness(0.0f);

        // Create a "PolishedMetal" material
        datablockName = "PolishedMetal";
        pbsDatablock = static_cast<Ogre::HlmsPbsDatablock*>(
            hlmsPbs->createDatablock(datablockName,
                datablockName,
                Ogre::HlmsMacroblock(),
                Ogre::HlmsBlendblock(),
                Ogre::HlmsParamVec()));

        pbsDatablock->setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);    // Use metallic workflow to avoid throwing assert

        pbsDatablock->setDiffuse(Ogre::Vector3(0.95f, 0.95f, 0.95f));
        pbsDatablock->setRoughness(0.1f);
        pbsDatablock->setMetalness(1.0f); // Set to 1.0 for metallic

        // --- 4. Create EnTT Entities and attach Ogre Components ---

        // Create a 2x2 grid of objects
        const int numX = 2;
        const int numZ = 2;
        const float spacing = 2.5f;

        for (int x = 0; x < numX; ++x) // Outer x loop starts
        {
            for (int z = 0; z < numZ; ++z) // Outer z loop starts
            {
                // --- Create EnTT Entity ---
                entt::entity entity = mRegistry.create();

                // --- Create Ogre Renderable ---
                Ogre::Item* item;
                if ((x + z) % 2 == 0)
                {
                    item = sceneManager->createItem(
                        "Cube_d.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                        Ogre::SCENE_DYNAMIC);
                    item->setDatablock("RedPlastic"); // Assign material here
                }
                else
                {
                    item = sceneManager->createItem(
                        "Sphere1000.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                        Ogre::SCENE_DYNAMIC);
                    item->setDatablock("PolishedMetal"); // Assign material here
                }

                Ogre::SceneNode* sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)
                    ->createChildSceneNode(Ogre::SCENE_DYNAMIC);
                sceneNode->attachObject(item);

                // --- Attach Components to Entity ---

                // 1. Attach the Ogre renderable parts
                mRegistry.emplace<OgreRenderableComponent>(entity, item, sceneNode);

                // 2. Attach the transform data
                Ogre::Vector3 initialPos((x - (numX - 1) * 0.5f) * spacing,
                    0.0f,
                    (z - (numZ - 1) * 0.5f) * spacing);
                mRegistry.emplace<TransformComponent>(entity, initialPos);

                // 3. Attach the logic component
                mRegistry.emplace<SpinComponent>(entity, (x + z * numX) * 0.1f + 0.5f);

            }
        }

        // --- 5. Set up Camera ---
        mGraphicsSystem->getCamera()->setPosition(Ogre::Vector3(0, 6, 10));
        mGraphicsSystem->getCamera()->lookAt(Ogre::Vector3(0, 0, 0));
        mCameraController = new CameraController(mGraphicsSystem, false);

        TutorialGameState::createScene01();
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
