set path=%~dp0
C:\VulkanSDK\1.1.85.0\Bin\glslangValidator.exe -V %path:~0,-1%\shader.vert
copy %path:~0,-1%\vert.spv %path:~0,-1%\..\..\Game\shaders\
copy %path:~0,-1%\vert.spv %path:~0,-1%\..\..\GameOutput\shaders\

C:\VulkanSDK\1.1.85.0\Bin\glslangValidator.exe -V %path:~0,-1%\shader.frag
copy %path:~0,-1%\frag.spv %path:~0,-1%\..\..\Game\shaders\
copy %path:~0,-1%\frag.spv %path:~0,-1%\..\..\GameOutput\shaders\

C:\VulkanSDK\1.1.85.0\Bin\glslangValidator.exe -V %path:~0,-1%\terrain.vert -o %path:~0,-1%\terrainvert.spv
copy %path:~0,-1%\terrainvert.spv %path:~0,-1%\..\..\Game\shaders\
copy %path:~0,-1%\terrainvert.spv %path:~0,-1%\..\..\GameOutput\shaders\

C:\VulkanSDK\1.1.85.0\Bin\glslangValidator.exe -V %path:~0,-1%\terrain.frag -o %path:~0,-1%\terrainfrag.spv
copy %path:~0,-1%\terrainfrag.spv %path:~0,-1%\..\..\Game\shaders\
copy %path:~0,-1%\terrainfrag.spv %path:~0,-1%\..\..\GameOutput\shaders\

pause