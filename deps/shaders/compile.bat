C:\VulkanSDK\1.1.85.0\Bin\glslangValidator.exe -V %cd%/../deps/shaders/shader.vert -o %cd%/../deps/shaders/vert.spv 
C:\VulkanSDK\1.1.85.0\Bin\glslangValidator.exe -V %cd%/../deps/shaders/shader.frag -o %cd%/../deps/shaders/frag.spv 
set source = %cd%\..\
copy %cd%\..\deps\shaders\frag.spv %cd%\shaders\
copy %cd%\..\deps\shaders\frag.spv %cd%\..\GameOutput\shaders\
copy %cd%\..\deps\shaders\vert.spv %cd%\..\GameOutput\shaders\
copy %cd%\..\deps\shaders\vert.spv %cd%\shaders\