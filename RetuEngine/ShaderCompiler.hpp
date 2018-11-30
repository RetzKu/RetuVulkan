#include <glslang/public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

std::string GetFilePath(const std::string& str)
{
	size_t found = str.find_last_of("/\\");
	return str.substr(0, found);
	//size_t FileName = str.substr(found+1);
}

std::string GetSuffix(const std::string& name)
{
	const size_t pos = name.rfind('.');
	return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
}

EShLanguage GetShaderStage(const std::string& stage)
{
	if (stage == "vert") {
		return EShLangVertex;
	}
	else if (stage == "tesc") {
		return EShLangTessControl;
	}
	else if (stage == "tese") {
		return EShLangTessEvaluation;
	}
	else if (stage == "geom") {
		return EShLangGeometry;
	}
	else if (stage == "frag") {
		return EShLangFragment;
	}
	else if (stage == "comp") {
		return EShLangCompute;
	}
	else {
		assert(0 && "Unknown shader stage");
		return EShLangCount;
	}
}


static bool glslangInitialized = false;

//TODO: Multithread, manage SpirV that doesn't need recompiling (only recompile when dirty)
const std::vector<unsigned int> CompileGLSL(const std::string& filename)
{
	//TODO: Handle initialization / finalization once
	// from source: "ShInitialize() should be called exactly once per process, not per thread."
	if (!glslangInitialized)
	{
		glslang::InitializeProcess();
		glslangInitialized = true;
	}

	//Load GLSL into a string
	std::ifstream file(filename);

	if (!file.is_open())
	{
		std::cout << "Failed to load shader: " << filename << std::endl;
		throw std::runtime_error("failed to open file: " + filename);
	}

	std::string InputGLSL((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	const char* InputCString = InputGLSL.c_str();

	EShLanguage ShaderType = GetShaderStage(GetSuffix(filename));
	glslang::TShader Shader(ShaderType);
	Shader.setStrings(&InputCString, 1);

	//Set up Vulkan/SpirV Environment
	int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;    // maps to, say, SPIR-V 1.0

	Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

	TBuiltInResource Resources;
	TBuiltInResource DefaultTBuiltInResource = {};
	Resources = DefaultTBuiltInResource;
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	const int DefaultVersion = 100;

	DirStackFileIncluder Includer;

	//Get Path of File
	std::string Path = GetFilePath(filename);
	Includer.pushExternalLocalDirectory(Path);

	std::string PreprocessedGLSL;

	if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		std::cout << "GLSL Preprocessing Failed for: " << filename << std::endl;
		std::cout << Shader.getInfoLog() << std::endl;
		std::cout << Shader.getInfoDebugLog() << std::endl;
	}

	//std::cout << PreprocessedGLSL << std::endl;

	const char* PreprocessedCStr = PreprocessedGLSL.c_str();
	Shader.setStrings(&PreprocessedCStr, 1);

	if (!Shader.parse(&Resources, 100, false, messages))
	{
		std::cout << "GLSL Parsing Failed for: " << filename << std::endl;
		std::cout << Shader.getInfoLog() << std::endl;
		std::cout << Shader.getInfoDebugLog() << std::endl;
	}

	glslang::TProgram Program;
	Program.addShader(&Shader);

	if (!Program.link(messages))
	{
		std::cout << "GLSL Linking Failed for: " << filename << std::endl;
		std::cout << Shader.getInfoLog() << std::endl;
		std::cout << Shader.getInfoDebugLog() << std::endl;
	}

	// if (!Program.mapIO())
	// {
	// 	std::cout << "GLSL Linking (Mapping IO) Failed for: " << filename << std::endl;
	// 	std::cout << Shader.getInfoLog() << std::endl;
	// 	std::cout << Shader.getInfoDebugLog() << std::endl;
	// }

	std::vector<unsigned int> SpirV;
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);

	//TODO: Manage spirv that's already compiled
	//glslang::OutputSpvBin(SpirV, (GetSuffix(filename) + std::string(".spv")).c_str());

	if (logger.getAllMessages().length() > 0)
	{
		std::cout << logger.getAllMessages() << std::endl;
	}

	//TODO: Handle startup shutdown separately from compile function
	//glslang::FinalizeProcess();

	return SpirV;
}