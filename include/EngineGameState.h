
#ifndef _Demo_EngineGameState_H_
#define _Demo_EngineGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

// Include the full definitions for Ogre::Vector3 and Ogre::Quaternion
#include "OgreVector3.h"
#include "OgreQuaternion.h"

// Include the EnTT header
#include "entt/entt.hpp"

// Forward-declare Ogre classes
namespace Ogre
{
    class Item;
    class SceneNode;
}

namespace Demo
{
    // --- EnTT Component Definitions ---

    /**
     @brief This component holds the Ogre-Next rendering objects.
     The ECS logic systems will update these based on other components.
    */
    struct OgreRenderableComponent
    {
        Ogre::Item* item;
        Ogre::SceneNode* sceneNode;
    };

    /**
     @brief This component stores the entity's position, orientation, and scale.
     Game logic systems (like physics or AI) should read/write to this.
     A separate "render sync" system will copy this data to the Ogre::SceneNode.
    */
    struct TransformComponent
    {
        Ogre::Vector3 position;
        Ogre::Quaternion orientation;
        Ogre::Vector3 scale;

        // Default constructor for easy emplacement
        TransformComponent(const Ogre::Vector3& p = Ogre::Vector3::ZERO,
            const Ogre::Quaternion& o = Ogre::Quaternion::IDENTITY,
            const Ogre::Vector3& s = Ogre::Vector3::UNIT_SCALE) :
            position(p),
            orientation(o),
            scale(s)
        {
        }
    };

    /**
     @brief A simple "logic" component. Any entity with this component
     will be spun by the `SpinSystem` in the update loop.
    */
    struct SpinComponent
    {
        Ogre::Real spinSpeed; // Radians per second
    };

    // --- Game State Class ---
    class EngineGameState : public TutorialGameState
    {
        /// @brief This is the heart of the Entity Component System.
        /// It manages all entities and their associated components.
        entt::registry mRegistry;

        void generateDebugText( float timeSinceLast, Ogre::String &outText ) override;

    public:
        EngineGameState( const Ogre::String &helpDescription );

        void createScene01() override;
        void destroyScene() override;

        void update( float timeSinceLast ) override;

        void keyReleased( const SDL_KeyboardEvent &arg ) override;
    };
}  // namespace Demo

#endif
