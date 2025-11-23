#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/bones.hpp"
#include "src/renderer.hpp"

void renderQuad();

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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        dfRenderer.setupGeometryPass(engine.getShader("gBuffer"), engine.getProjectionMatrix(), engine.getViewMatrix());
        // clear screen
        engine.clear();

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
        glBindFramebuffer(GL_READ_FRAMEBUFFER, dfRenderer.getGBuffer());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, engine.getPostProcessor()->getFBO());
        glBlitFramebuffer(0, 0, engine.getWidth(), engine.getHeight(), 0, 0, engine.getWidth(), engine.getHeight(),
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        engine.enablePostProcessing();
        glClear(GL_COLOR_BUFFER_BIT);

        engine.useShader("deferredShading");
        engine.setVec3("viewPos", engine.getCameraPosition(), "deferredShading");
        engine.setInt("gPositionE", 0, "deferredShading");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dfRenderer.getPositionEBuffer());
        engine.setInt("gAlbedo", 1, "deferredShading");
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, dfRenderer.getColorBuffer());
        engine.setInt("gNormalE", 2, "deferredShading");
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dfRenderer.getNormalEBuffer());
        engine.setInt("gARME", 3, "deferredShading");
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, dfRenderer.getARMEBuffer());
        engine.setInt("irradianceMap", 10, "deferredShading");
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iblGenerator.getIrradianceMap());
        engine.setInt("prefilterMap", 11, "deferredShading");
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iblGenerator.getPrefilterMap());
        engine.setInt("brdfLUT", 12, "deferredShading");
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, iblGenerator.getBRDFLutMap());

        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        renderQuad();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

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

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
