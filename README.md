# RomilsVulkanRenderer
 Vulkan Renderer using Forward Plus Pipeline

##Renderer Details

* Language: **C++**
* Graphics API: **Vulkan**
* Math Library: **GLM**
* Window Abstraction: **GLFW3**
* Texture Loading: **STB Image**
* Object Loading: **TinyOBJLoader**

The Vulkan SDK used for the project was the LunarG SDK.

I have used the Sponza Model by NVIDIA for testing and presentation.

The Renderer uses Forward Plus Rendering, the Renderer is currently set to shade using 10,000 point lights.
Adjust this in the VulkanApplication file as required. WASD is used for movement and Q,E are used to change direction.


![SS1](Screenshots/FP1.png)
![SS2](Screenshots/FP2.png)
![SS3](Screenshots/FP3.png)
![SS4](Screenshots/FP4.png)

###Please Extract the libraries in the dependancies folder to run the project

##Links to Libraries and SDK used.
* [LunarG Vulkan SDK](https://vulkan.lunarg.com/)
* [GLM](https://glm.g-truc.net/0.9.9/index.html)
* [GLFW](https://www.glfw.org/)
* [STB Image](https://github.com/nothings/stb)
* [TinyOBJLoader](https://github.com/tinyobjloader/tinyobjloader)

##Future Plans
There are no set plans to expand upon the current implementation.
However, if I do decide on it, I may expand with the following techniques.

* Soft shadows.
* PBL
* AA implementations
* Parallax Mapping
