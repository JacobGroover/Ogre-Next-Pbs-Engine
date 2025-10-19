#ifndef _Engine_Component_OgreRenderable_H_
#define _Engine_Component_OgreRenderable_H_

// Forward-declare Ogre classes to avoid including heavy headers here
namespace Ogre
{
    class Item;
    class SceneNode;
}

/**
 @brief This component holds the Ogre-Next rendering objects.
 The ECS logic systems will update these based on other components.
*/
struct OgreRenderableComponent
{
    Ogre::Item* item;
    Ogre::SceneNode* sceneNode;
};

#endif // _Engine_Component_OgreRenderable_H_
