#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/bones.hpp"
#include "src/renderer.hpp"
#include "src/util.hpp"

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

    Model* spartan{engine.getModel("light")};
    BoneAnimation spartanAnimation{"data/models/spartan.glb", spartan};
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
    DeferredRenderer* dfRenderer{engine.getDeferredRenderer()};

    // reset window viewport
    glViewport(0, 0, engine.getWidth(), engine.getHeight());

    while (!engine.getQuit())
    {
        // update game state
        spartanAnimator.updateAnimation(engine.getDeltaTime());

        // do rendering
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        dfRenderer->setupGeometryPass(engine.getShader("gBuffer"), engine.getProjectionMatrix(),
                                      engine.getViewMatrix());
        // clear screen
        engine.clear();

        const std::vector<glm::mat4>& transforms{spartanAnimator.getFinalBoneMatrices()};
        for (std::size_t i{0}; i < transforms.size(); ++i)
            engine.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i], "gBuffer");

        engine.setMat4("view", engine.getViewMatrix(), "gBuffer");
        engine.setMat4("projection", engine.getProjectionMatrix(), "gBuffer");

        for (std::size_t z{0}; z < 10; ++z)
        {
            for (std::size_t x{0}; x < 10; ++x)
            {
                glm::mat4 model;
                model = glm::translate(
                    glm::mat4{1.0f},
                    {static_cast<float>(x) * 100.0f + std::sin(static_cast<float>(x * (z + 1))) * 10.f, 0.0f,
                     static_cast<float>(z) * 100.0f + std::cos(static_cast<float>(x * (z + 1))) * 10.f});
                engine.setMat4("model", model, "gBuffer");
                engine.setMat3("normalMat", engine.getNormalMatrix(Util::stripScale(model)), "gBuffer");
                spartan->renderFull(engine.getShader("gBuffer"), engine.getCameraPosition(), model);
            }
        }

        dfRenderer->closeGeometryPass();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, dfRenderer->getGBuffer());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, engine.getPostProcessor()->getFBO());
        glBlitFramebuffer(0, 0, engine.getWidth(), engine.getHeight(), 0, 0, engine.getWidth(), engine.getHeight(),
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        engine.enablePostProcessing();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClear(GL_COLOR_BUFFER_BIT);

        engine.renderGBuffer(&iblGenerator);
        glDisable(GL_BLEND);

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        iblGenerator.renderSkybox(&engine);
        glDepthMask(GL_TRUE);

        engine.disablePostProcessing();
        engine.renderPostProcessing();

        // update engine
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}
