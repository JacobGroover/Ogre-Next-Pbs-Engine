
#ifndef _Demo_EngineGameState_H_
#define _Demo_EngineGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

// Include the full definitions for Ogre::Vector3 and Ogre::Quaternion
#include "OgreVector3.h"
#include "OgreQuaternion.h"

// Include the EnTT header
#include "entt/entt.hpp"

// Include the central components header for EnTT components
#include "Components/Components.h"

namespace Demo
{

    // --- Game State Class ---
    class EngineGameState : public TutorialGameState
    {
        /// @brief This is the heart of the Entity Component System.
        /// It manages all entities and their associated components.
        entt::registry mRegistry;

        Ogre::String mSceneToLoad;
        int mArgc;
        const char** mArgv;

        void generateDebugText( float timeSinceLast, Ogre::String &outText ) override;

        void parseCommandLineArgs( int argc, const char* argv[] );
        void loadSceneFromJson( const Ogre::String& filename );

    public:
        EngineGameState( const Ogre::String &helpDescription, int argc, const char *argv[] );

        void createScene01() override;
        void destroyScene() override;

        void update( float timeSinceLast ) override;

        void keyReleased( const SDL_KeyboardEvent &arg ) override;

    };
}  // namespace Demo

#endif
