#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/bones.hpp"
#include "src/renderer.hpp"

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
    engine.addModel("light", "data/models/spartan.glb");

    Model* light{engine.getModel("light")};
    BoneAnimation spartanAnimation{"data/models/spartan.glb", light};
    BoneAnimator spartanAnimator{&spartanAnimation};
    // engine.enableWireframe();
    const std::vector<glm::vec3> spheres{{1.f, 4.f, 2.f}};

    engine.useShader("lightPBR");
    engine.setVec3("albedo", glm::vec3{0.5, 0.0f, 0.0f}, "lightPBR");
    engine.setFloat("ao", 1.0f, "lightPBR");

    // ----------- IBL ------------ //
    IBLGenerator iblGenerator{&engine};
    iblGenerator.init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png", &engine);

    // ----------- Deferred Rendering ----------- //
    DeferredRenderer dfRenderer{&engine};
    dfRenderer.init(engine.getWidth(), engine.getHeight());

    // reset window viewport
    glViewport(0, 0, engine.getWidth(), engine.getHeight());

    while (!engine.getQuit())
    {
        // update game state
        spartanAnimator.updateAnimation(engine.getDeltaTime());

        // do rendering
        engine.enablePostProcessing();
        // clear screen
        engine.clear();

        // render skybox first
        glDepthMask(GL_FALSE);
        iblGenerator.renderSkybox(&engine);
        glDepthMask(GL_TRUE);
        engine.disablePostProcessing();

        dfRenderer.setupGeometryPass(engine.getShader("gBuffer"), engine.getProjectionMatrix(), engine.getViewMatrix());

        const std::vector<glm::mat4>& transforms{spartanAnimator.getFinalBoneMatrices()};
        for (std::size_t i{0}; i < transforms.size(); ++i)
            engine.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i], "gBuffer");

        glm::mat4 model{glm::mat4{1.0f}};
        engine.setMat4("model", model, "gBuffer");
        engine.setMat4("view", engine.getViewMatrix(), "gBuffer");
        engine.setMat4("projection", engine.getProjectionMatrix(), "gBuffer");
        engine.setMat3("normalMat", engine.getNormalMatrix(model), "gBuffer");

        light->renderFull(engine.getShader("gBuffer"), engine.getCameraPosition(), model);

        dfRenderer.closeGeometryPass();

        engine.renderPostProcessing();

        // update engine
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}
