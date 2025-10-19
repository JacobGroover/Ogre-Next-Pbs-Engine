#ifndef _Engine_Component_Transform_H_
#define _Engine_Component_Transform_H_

// Include necessary Ogre headers for Vector3 and Quaternion
#include "OgreVector3.h"
#include "OgreQuaternion.h"

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

#endif // _Engine_Component_Transform_H_
