#ifndef _Demo_EngineGameState_H_
#define _Demo_EngineGameState_H_

#include "OgrePrerequisites.h"

#include "GameState.h"

// Include the full definitions for Ogre::Vector3 and Ogre::Quaternion
#include "OgreVector3.h"
#include "OgreQuaternion.h"

// Include the EnTT header
#include "entt/entt.hpp"

// Include the central components header for EnTT components
#include "Components/Components.h"

#include "rapidjson/document.h"

namespace Ogre
{
    namespace v1
    {
        class TextAreaOverlayElement;
    }
}  // namespace Ogre

namespace Demo
{
    // Forward declarations
    class GraphicsSystem;
    class CameraController;

    void processResourcesFromJson(const rapidjson::Document& document, Demo::GraphicsSystem* graphicsSystem);

    // --- Game State Class ---
    class EngineGameState : public GameState
    {
    protected:
		GraphicsSystem* mGraphicsSystem;

        /// @brief This is the heart of the Entity Component System.
        /// It manages all entities and their associated components.
        entt::registry mRegistry;

        Ogre::String mSceneToLoad;
        int mArgc;
        const char** mArgv;

        // Members previously inherited from TutorialGameState
        CameraController*   mCameraController;

        Ogre::String        mHelpDescription;
        Ogre::uint16        mDisplayHelpMode;
        Ogre::uint16        mNumDisplayHelpModes;

        Ogre::v1::TextAreaOverlayElement* mDebugText;
        Ogre::v1::TextAreaOverlayElement* mDebugTextShadow;

        virtual void createDebugTextOverlay();
        virtual void generateDebugText(float timeSinceLast, Ogre::String& outText);

        // Internal helper methods
        void parseCommandLineArgs(int argc, const char* argv[]);
        void loadSceneFromJson(const Ogre::String& filename);

    public:
        EngineGameState( const Ogre::String& helpDescription, int argc, const char* argv[] );
        ~EngineGameState() override;

        void _notifyGraphicsSystem( GraphicsSystem* graphicsSystem );

        void createScene01() override;
        void destroyScene() override;

        void update( float timeSinceLast ) override;

        // NEW: We must override the base input methods to handle camera movement
        // (Previously, TutorialGameState handled these)
        //void generateDebugText(float timeSinceLast, Ogre::String& outText) override;

        void keyPressed( const SDL_KeyboardEvent& arg ) override;
        void keyReleased( const SDL_KeyboardEvent& arg ) override;

        void mouseMoved(const SDL_Event& arg) override;

        /*
        void mouseMoved(const SDL_MouseMotionEvent& arg) override;

        void mousePressed(const SDL_MouseButtonEvent& arg) override;
        void mouseReleased(const SDL_MouseButtonEvent& arg) override;

        void textInput(const SDL_TextInputEvent& arg) override;
        void controllerAxisMoved(const SDL_ControllerAxisEvent& arg) override;
        void controllerButtonPressed(const SDL_ControllerButtonEvent& arg) override;
        void controllerButtonReleased(const SDL_ControllerButtonEvent& arg) override;
        */
    };
}  // namespace Demo

#endif