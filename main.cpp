#include <iostream>
#include <iterator>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glm/common.hpp"
#include "src/engine.hpp"
#include "src/texture.hpp"
#include "src/util.hpp"

void renderCube();

int main()
{
    // initialize engine
    Engine engine{};
    if (!engine.init(640, 480, "OpenGL Window"))
    {
        std::cout << "Failed to initialize engine!\n";
        return 1;
    }

    std::cout << "Initialized engine!\n";
    engine.setCameraEnabled(true);

    // use only gltf files for now
    engine.addModel("cube", "data/models/monkey.glb");
    engine.addModel("light", "data/models/spartan.glb");

    const Model* light{engine.getModel("light")};

    // engine.enableWireframe();
    const std::vector<glm::vec3> spheres{{1.f, 4.f, 2.f}};

    engine.useShader("lightPBR");
    engine.setVec3("albedo", glm::vec3{0.5, 0.0f, 0.0f}, "lightPBR");
    engine.setFloat("ao", 1.0f, "lightPBR");

    glm::vec3 lightPos{1.0f, 1.0f, 1.0f};

    // ----------- IBL ------------ //
    // hdr irradiance map
    bool success;
    unsigned int skyboxTexture{TextureN::loadHDRMap("data/skyboxes/clouds.hdr", &success)};
    if (!success)
    {
        Util::beginError();
        std::cout << "ERROR: Failed to load HDR map!" << std::endl;
        Util::endError();
    }

    unsigned int irradianceTexture{TextureN::loadHDRMap("data/IBL/clouds/output_iem.hdr", &success)};
    if (!success)
    {
        Util::beginError();
        std::cout << "ERROR: Failed to load irradiance texture!" << std::endl;
        Util::endError();
    }

    unsigned int prefilterTexture{TextureN::loadHDRMap("data/IBL/clouds/output_pmrem.hdr", &success)};
    if (!success)
    {
        Util::beginError();
        std::cout << "ERROR: Failed to load prefiltered texture!" << std::endl;
        Util::endError();
    }

    unsigned int brdfLutMap{TextureN::loadFromFile("data/IBL/brdf_lut.png", nullptr, nullptr, nullptr, &success)};
    if (!success)
    {
        Util::beginError();
        std::cout << "ERROR: Failed to load BDRF LUT map!" << std::endl;
        Util::endError();
    }

    // generate framebuffer to capture skybox cubemap
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // generate cubemap color textures
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i{0}; i < 6; ++i)
    {
        // NOTE: 16F values for tex
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // capture equirectangular texture onto cubemap faces
    glm::mat4 captureProjection{glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f)};
    const glm::mat4 captureViews[]{
        glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, -1.0f, 0.0f}),
        glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{-1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, -1.0f, 0.0f}),
        glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}),
        glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, -1.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -1.0f}),
        glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, -1.0f, 0.0f}),
        glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -1.0f}, glm::vec3{0.0f, -1.0f, 0.0f})};

    // convert HDR environment map to cubemap equivalent
    engine.useShader("erCubeMapConvert");
    engine.setMat4("projection", captureProjection, "erCubeMapConvert");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);
    engine.setInt("equirectangularMap", 0, "erCubeMapConvert");

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i{0}; i < 6; ++i)
    {
        engine.setMat4("view", captureViews[i], "erCubeMapConvert");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render 1x1 cube
        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // diffuse irradiance map
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i{0}; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // convert irradiance texture to cube map equivalent
    engine.useShader("erCubeMapConvert");
    engine.setMat4("projection", captureProjection, "erCubeMapConvert");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, irradianceTexture);
    engine.setInt("equirectangularMap", 0, "erCubeMapConvert");

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i{0}; i < 6; ++i)
    {
        engine.setMat4("view", captureViews[i], "erCubeMapConvert");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap,
                               0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // prefiltered specular map
    /*unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i{0}; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // convert prefilter texture to cube map equivalent
    engine.useShader("erCubeMapConvert");
    engine.setMat4("projection", captureProjection, "erCubeMapConvert");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, prefilterTexture);
    engine.setInt("equirectangularMap", 0, "erCubeMapConvert");

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    constexpr unsigned int maxLevels{5}; // max number of mipmap levels
    for (unsigned int mip{0}; mip < maxLevels; ++mip)
    {
        // resize framebuffer to mip-level size
        unsigned int width{static_cast<unsigned int>(128 * std::pow(0.5, mip))};
        unsigned int height{static_cast<unsigned int>(128 * std::pow(0.5, mip))};
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glViewport(0, 0, width, height);

        for (unsigned int i{0}; i < 6; ++i)
        {
            engine.setMat4("view", captureViews[i], "erCubeMapConvert");
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderCube();
        }
    }*/

    unsigned int prefilterMap;
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i{0}; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    engine.useShader("prefilterMap");
    engine.setInt("environmentMap", 0, "prefilterMap");
    engine.setMat4("projection", captureProjection, "prefilterMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    constexpr unsigned int maxLevels{5};
    for (unsigned int mip{0}; mip < maxLevels; ++mip)
    {
	const unsigned int mipWidth{static_cast<unsigned int>(128 * std::pow(0.5, mip))};
	const unsigned int mipHeight{static_cast<unsigned int>(128 * std::pow(0.5, mip))};
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
	glViewport(0, 0, mipWidth, mipHeight);

	const float roughness {static_cast<float>(mip) / static_cast<float>(maxLevels - 1)};
	engine.setFloat("roughness", roughness, "prefilterMap");
	for (unsigned int i{0}; i < 6; ++i)
	{
	    engine.setMat4("view", captureViews[i], "prefilterMap");
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    renderCube();
	}
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset window viewport
    glViewport(0, 0, engine.getWidth(), engine.getHeight());
    while (!engine.getQuit())
    {
        engine.enablePostProcessing();
        // clear screen
        engine.clear();



        engine.useShader("texturePBR");
        engine.setVec3("viewPos", engine.getCameraPosition(), "texturePBR");

	glm::mat4 model{1.0f};
        for (std::size_t i{0}; i < spheres.size(); ++i)
        {
            model = glm::mat4{1.0f};
            model = glm::scale(model, glm::vec3{0.2f});
            model = glm::translate(model, spheres[i]);
            // model = glm::rotate(model, engine.getTime() * 0.6f, {1.0f, 0.7f, 0.3f});
            engine.setMat4("model", model, "texturePBR");
            engine.setMat4("view", engine.getViewMatrix(), "texturePBR");
            engine.setMat4("projection", engine.getProjectionMatrix(), "texturePBR");
            engine.setMat3("normalMat", engine.getNormalMatrix(model), "texturePBR");
            engine.setInt("irradianceMap", 10, "texturePBR");
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	    engine.setInt("prefilterMap", 11, "texturePBR");
	    glActiveTexture(GL_TEXTURE11);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	    engine.setInt("brdfLUT", 12, "texturePBR");
	    glActiveTexture(GL_TEXTURE12);
	    glBindTexture(GL_TEXTURE_2D, brdfLutMap);
            light->renderPBR(engine.getShader("texturePBR"));
        }

        engine.useShader("skybox");
        engine.setMat4("view", engine.getViewMatrix(), "skybox");
        engine.setMat4("projection", engine.getProjectionMatrix(), "skybox");
        engine.setInt("environmentMap", 0, "skybox");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        renderCube();
        engine.disablePostProcessing();
        engine.renderPostProcessing();

        // update engine
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
