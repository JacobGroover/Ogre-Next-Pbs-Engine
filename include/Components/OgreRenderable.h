#ifndef _Engine_Component_OgreRenderable_H_
#define _Engine_Component_OgreRenderable_H_

// Forward-declare Ogre classes to avoid including heavy headers here
namespace Ogre
{
    class SceneNode;
    class Item;
    class Light;
    class Camera;
}

// The Anchor: Holds the Node. 
// REQUIRED for the RenderSyncSystem to update position/rotation.
struct SceneNodeComponent
{
    Ogre::SceneNode* sceneNode;
};

// The Mesh: Holds the Item (Visual geometry)
struct ItemComponent
{
    Ogre::Item* item;
};

// The Light: Holds the Light
struct LightComponent
{
    Ogre::Light* light;
};

// The Camera: Holds the Camera
struct CameraComponent
{
    Ogre::Camera* camera;
};

#endif // _Engine_Component_OgreRenderable_H_
