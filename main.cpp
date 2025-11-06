#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"

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

    // ----------- IBL ------------ //
    IBLGenerator iblGenerator{&engine};
    iblGenerator.init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png", &engine);

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
            // model = glm::rotate(model, engine.getTime() * 0.6f, {0.0f, 1.0f, 0.0f});
            engine.setMat4("model", model, "texturePBR");
            engine.setMat4("view", engine.getViewMatrix(), "texturePBR");
            engine.setMat4("projection", engine.getProjectionMatrix(), "texturePBR");
            engine.setMat3("normalMat", engine.getNormalMatrix(model), "texturePBR");
            engine.setInt("irradianceMap", 10, "texturePBR");
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_CUBE_MAP, iblGenerator.getIrradianceMap());
            engine.setInt("prefilterMap", 11, "texturePBR");
            glActiveTexture(GL_TEXTURE11);
            glBindTexture(GL_TEXTURE_CUBE_MAP, iblGenerator.getPrefilterMap());
            engine.setInt("brdfLUT", 12, "texturePBR");
            glActiveTexture(GL_TEXTURE12);
            glBindTexture(GL_TEXTURE_2D, iblGenerator.getBRDFLutMap());
            light->renderPBR(engine.getShader("texturePBR"));
        }

        iblGenerator.renderSkybox(&engine);

        engine.disablePostProcessing();
        engine.renderPostProcessing();

        // update engine
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}
