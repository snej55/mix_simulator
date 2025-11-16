#include <iostream>
#include <array>
#include <mutex>
#include <thread>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/util.hpp"

#define MAX_ELEMENTS 100

void mergeSort(std::array<int, MAX_ELEMENTS>& arr, const int start, const int end)
{
    if (start >= end - 1)
	return;

    const int mid {start + static_cast<int>(std::floor(static_cast<float>(end - start) * 0.5f))};

    mergeSort(arr, start, mid);
    mergeSort(arr, mid, end);

    std::vector<int> cache;
    cache.reserve(end - start);
    std::fill(cache.begin(), cache.end(), arr[0]);

    int r{0};
    int k{mid};
    for (int i{start}; i < mid; ++i)
    {
	while (k < end && arr[k] < arr[i])
	{
	    cache[r] = arr[k];
	    r++;
	    k++;
	}
	cache[r] = arr[i];
	r++;
    }

    for (int i{0}; i < k; ++i)
    {
	arr[i + start] = cache[i];
	std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

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
    engine.addModel("light", "data/models/gold_cube.glb");

    Model* light{engine.getModel("light")};
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

    std::array<int, MAX_ELEMENTS> numbers{};
    for (std::size_t i{0}; i < MAX_ELEMENTS; ++i)
        numbers[i] = static_cast<int>(i + 1);

    // randomize
    for (std::size_t i{0}; i < numbers.size(); ++i)
    {
	std::swap(numbers[i], numbers[static_cast<int>(Util::random() * static_cast<float>(numbers.size()))]);
    }

    int bubbleIndex{0};
    while (!engine.getQuit())
    {
        // update game state

        // bubble sort
        if (numbers[bubbleIndex + 1] < numbers[bubbleIndex])
        {
            std::swap(numbers[bubbleIndex], numbers[bubbleIndex + 1]);
        }
        bubbleIndex++;
        if (bubbleIndex >= numbers.size() - 1)
        {
            bubbleIndex = 0;
        }

        // rendering
        engine.enablePostProcessing(); // bind post processing framebuffer
        engine.clear(); // clear screen

        engine.useShader("texturePBR");
        engine.setVec3("viewPos", engine.getCameraPosition(), "texturePBR");

        glm::mat4 model{glm::mat4{1.0f}};
        model = glm::scale(model, glm::vec3{0.2f});
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

	// render the numbers array
        for (std::size_t i{0}; i < numbers.size(); ++i)
        {
            model = glm::scale(glm::mat4{1.0f}, glm::vec3{0.2f, 0.2f * static_cast<float>(numbers[i]), 0.2f});
            model = glm::translate(model, {static_cast<float>(i) * 3.0f, 0.0f, 0.0f});
            engine.setMat4("model", model, "texturePBR");
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
