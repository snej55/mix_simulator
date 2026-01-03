#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/bounds.hpp"
#include "src/camera.hpp"
#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/renderer.hpp"
#include "src/scene.hpp"

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

    engine.addModel("cube", "data/models/gold_cube.glb");

    // ----------- Scene ------------ //
    // NOTE: Load scene before IBL and DeferredRenderer so models load correctly
    Scene scene{&engine};
    scene.init("data/maps/0.json");

    // ----------- IBL ------------ //
    IBLGenerator iblGenerator{&engine};
    iblGenerator.init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png", &engine);

    // ----------- Deferred Rendering ----------- //
    const DeferredRenderer* dfRenderer{engine.getDeferredRenderer()};
    RenderQueue renderQueue{&engine};

    // reset window viewport
    glViewport(0, 0, engine.getWidth(), engine.getHeight());

    while (!engine.getQuit())
    {
        const Bounds::Frustum camFrustum{engine.getCameraFrustum()};

        // update entities
        scene.updateEntities(engine.getDeltaTime());
        scene.cleanupEmptyChunks();

        // get visible chunks
        const Bounds::AABB camFrustumBV{Bounds::getFrustumBV(
            camFrustum, engine.getCamera(), CAMERA_Z_FAR, glm::radians(engine.getCamera()->getZoom()),
            static_cast<float>(engine.getWidth()) / static_cast<float>(engine.getHeight()))};
        std::vector<SceneChunk*> visibleChunks{};
        scene.getVisibleChunks(camFrustum, camFrustumBV, visibleChunks);

        for (SceneChunk* chunk : visibleChunks)
        {
            renderQueue.addChunk(chunk, camFrustum);
        }

        renderQueue.renderFrame(engine.getShader("gBuffer"), dfRenderer, engine.getShader("texturePBR"),
                                engine.getPostProcessor(), &engine, &iblGenerator, engine.getCameraPosition());

        // update engine
        renderQueue.update();
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}
