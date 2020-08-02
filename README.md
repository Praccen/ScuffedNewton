# ScuffedNewton
A kind of wonky physics engine in development

Scuffed Newton is compiled as a DLL for easy usage. Support functions are implemented to allow explicit linking. 
So far Scuffed Newton is mostly a component-based collision engine that uses Separating Axis Theorem to find collisions (including continuous collisions) and provides a very simple collision response. However, most of the detection part is there (like collision manifolds, etc) to start implementing everything needed for dynamic rigid-body collisions.

Things I plan on doing:
* Rework of the collision system to do all collisions in the correct order as well as calculating friction and other forces with every collision.
* Implementation of proper forces and impulses.
* Expansion of collision detection to include mesh vs mesh checking (so far only box vs box and box vs mesh is implemented)
* Implementation of spheres, capsules, and other shapes.
* Multi-threading
* And a lot more! :)
