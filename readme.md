Textures in scene:
Our brick building and the ground have textures. They are of grass and brick, respectively.

https://opengameart.org/node/8015 for our brick texture, it is open source
https://opengameart.org/content/grass-hedge for our grass texture on ground

Mipmapping:
Yes, we have mipmapping supported in the project.

Image credits:
Yes the sources can be found above.

Texture-object match:
Ground with grass texture and building with brick texture, seems reasonable to me.

Lighting in scene:

We have two stationary lights. One is a green one coming from below the ground. This can be seen inside our brick building which has a green tint inside.
Another is a generic white light placed above the scene. The orange top of the tower has some specular and the figure standing on top of the brick structure has a ton of specular, he is visibly more shiny than the other figure. All other objects are diffuse.

Lighting implementation:
Yes, each node can have many lights, all lights are transformed properly, and they are sent to the shader first.

Spotlight:

Yes there is a spotlight in our scene, the edge of it can be seen upon our red building.

Spotlight implementation:

Our math is correct.

XML support for lighting and texturing:
We fully support creating lights, textures, and importing images through xml.