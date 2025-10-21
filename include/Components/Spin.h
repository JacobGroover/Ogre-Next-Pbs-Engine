#ifndef _Engine_Component_Spin_H_
#define _Engine_Component_Spin_H_

#include "OgrePrerequisites.h" // For Ogre::Real

/**
 @brief A simple "logic" component. Any entity with this component
 will be spun by the `SpinSystem` in the update loop.
*/
struct SpinComponent
{
    Ogre::Real spinSpeed; // Radians per second
};

#endif // _Engine_Component_Spin_H_
