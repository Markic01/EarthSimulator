#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> &faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutoff;
    float outerCutOff;

    glm::vec3 specular;
    glm::vec3 diffuse;
    glm::vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0.0f);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 earthPosition = glm::vec3(0.0f);
    float earthScale = 0.9f;
    glm::vec3 sunPosition = glm::vec3(0.31f, 0.90f, 0.86f);
    float sunScale = 0.15f;
    glm::vec3 moonPosition = glm::vec3(-0.32f, 1.73f, -0.05f);
    float moonScale = 0.05f;

    DirectionalLight directionalLight;
    SpotLight sunSpotLight;
    SpotLight moonSpotLight;

    ProgramState(): directionalLight(), sunSpotLight(), moonSpotLight(){

    }

    void SaveToFile(const std::string &filename) const;

    void LoadFromFile(const std::string &filename);
};

void ProgramState::SaveToFile(const std::string &filename) const {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(const std::string &filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui();

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // build and compile shaders
    // -------------------------
    Shader modelsShader("resources/shaders/models.vs", "resources/shaders/models.fs");
    Shader earthShader("resources/shaders/flat_earth.vs", "resources/shaders/flat_earth.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");


    // load models
    // -----------
    Model earthModel("resources/objects/earth/flat_earth.obj");
    earthModel.SetShaderTextureNamePrefix("material.");

    Model sunModel("resources/objects/sun/sun.obj");
    sunModel.SetShaderTextureNamePrefix("material.");

    Model moonModel("resources/objects/moon/moon.obj");
    moonModel.SetShaderTextureNamePrefix("material.");

    DirectionalLight& directionalLight = programState->directionalLight;
    directionalLight.direction = glm::vec3(0.0f, -0.5f, 0.0f);
    directionalLight.ambient = glm::vec3(0.2f);
    directionalLight.diffuse = glm::vec3(0.6f);
    directionalLight.specular = glm::vec3(1.0f);

    SpotLight& sunSpotLight = programState->sunSpotLight;
    sunSpotLight.ambient = glm::vec3(0.1f);
    sunSpotLight.diffuse = glm::vec3(1.0f);
    sunSpotLight.specular = glm::vec3(1.0f);
    sunSpotLight.position = programState->sunPosition;
    sunSpotLight.direction = programState->earthPosition - programState->sunPosition;
    sunSpotLight.cutoff = glm::cos(glm::radians(10.5f));
    sunSpotLight.outerCutOff = glm::cos(glm::radians(12.5f));
    sunSpotLight.constant = 1.0f;
    sunSpotLight.linear = 0.35f;
    sunSpotLight.quadratic = 0.44f;

    SpotLight& moonSpotLight = programState->moonSpotLight;
    moonSpotLight.ambient = glm::vec3(0.0f);
    moonSpotLight.diffuse = glm::vec3(0.5f);
    moonSpotLight.specular = glm::vec3(1.0f);
    moonSpotLight.position = programState->moonPosition;
    moonSpotLight.direction = programState->earthPosition - programState->moonPosition;
    moonSpotLight.cutoff = glm::cos(glm::radians(7.5f));
    moonSpotLight.outerCutOff = glm::cos(glm::radians(10.5f));
    moonSpotLight.constant = 1.0f;
    moonSpotLight.linear = 0.22f;
    moonSpotLight.quadratic = 0.20f;

    vector<std::string> faces {
            FileSystem::getPath("resources/textures/skybox/stars_right.png"),
            FileSystem::getPath("resources/textures/skybox/stars_left.png"),
            FileSystem::getPath("resources/textures/skybox/stars_up.png"),
            FileSystem::getPath("resources/textures/skybox/stars_down.png"),
            FileSystem::getPath("resources/textures/skybox/stars_front.png"),
            FileSystem::getPath("resources/textures/skybox/stars_back.png")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        auto currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelsShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        modelsShader.setMat4("projection", projection);
        modelsShader.setMat4("view", view);

        // render the sun model
        programState->sunPosition=glm::vec3(sin(glfwGetTime())-0.2,1.0f,cos(glfwGetTime()));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,programState->sunPosition);
        model = glm::scale(model, glm::vec3(programState->sunScale));
        modelsShader.setMat4("model", model);
        sunModel.Draw(modelsShader);

        // render the moon model
        programState->moonPosition=glm::vec3(-sin(glfwGetTime())-0.2f,1.0f,-cos(glfwGetTime()));
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->moonPosition);
        model = glm::scale(model, glm::vec3(programState->moonScale));
        modelsShader.setMat4("model", model);
        moonModel.Draw(modelsShader);

        earthShader.use();
        earthShader.setVec3("directionalLight.direction", directionalLight.direction);
        earthShader.setVec3("directionalLight.ambient", directionalLight.ambient);
        earthShader.setVec3("directionalLight.diffuse", directionalLight.diffuse);
        earthShader.setVec3("directionalLight.specular", directionalLight.specular);
        earthShader.setVec3("sunLight.ambient", sunSpotLight.ambient);
        earthShader.setVec3("sunLight.diffuse", sunSpotLight.diffuse);
        earthShader.setVec3("sunLight.specular", sunSpotLight.specular);
        earthShader.setVec3("sunLight.position", programState->sunPosition);
        earthShader.setVec3("sunLight.direction", glm::vec3(sin(glfwGetTime())/5.0f-0.2f,-1.0f,cos(glfwGetTime())/5.0f) - programState->sunPosition);
        earthShader.setFloat("sunLight.cutOff", sunSpotLight.cutoff);
        earthShader.setFloat("sunLight.outerCutOff", sunSpotLight.outerCutOff);
        earthShader.setFloat("sunLight.constant", sunSpotLight.constant);
        earthShader.setFloat("sunLight.linear", sunSpotLight.linear);
        earthShader.setFloat("sunLight.quadratic", sunSpotLight.quadratic);

        earthShader.setVec3("moonLight.ambient", moonSpotLight.ambient);
        earthShader.setVec3("moonLight.diffuse", moonSpotLight.diffuse);
        earthShader.setVec3("moonLight.specular", moonSpotLight.specular);
        earthShader.setVec3("moonLight.position", programState->moonPosition);
        earthShader.setVec3("moonLight.direction", glm::vec3(-sin(glfwGetTime())/4.0f-0.2f,-1.0f,-cos(glfwGetTime())/4.0f) - programState->moonPosition);
        earthShader.setFloat("moonLight.cutOff", moonSpotLight.cutoff);
        earthShader.setFloat("moonLight.outerCutOff", moonSpotLight.outerCutOff);
        earthShader.setFloat("moonLight.constant", moonSpotLight.constant);
        earthShader.setFloat("moonLight.linear", moonSpotLight.linear);
        earthShader.setFloat("moonLight.quadratic", moonSpotLight.quadratic);

        earthShader.setVec3("viewPosition", programState->camera.Position);
        earthShader.setFloat("material.shininess", 32.0f);
        earthShader.setVec3("material.specular", 0.05f);
        earthShader.setMat4("projection", projection);
        earthShader.setMat4("view", view);

        // render the flatEarth model
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->earthPosition);
        model = glm::scale(model, glm::vec3(programState->earthScale));
        earthShader.setMat4("model", model);
        earthModel.Draw(earthShader);

        // draw skybox
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        if (programState->ImGuiEnabled)
            DrawImGui();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(UP, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

    lastX = (float)xpos;
    lastY = (float)ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *, double , double yoffset) {
    programState->camera.ProcessMouseScroll((float)yoffset);
}

void DrawImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float f = 0.0f;
    ImGui::Begin("Hello window");
    ImGui::Text("Hello text");
    ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
    ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
    ImGui::DragFloat3("Earth position", (float*)&programState->earthPosition);
    ImGui::DragFloat("Earth scale", &programState->earthScale, 0.05, 0.1, 4.0);
    ImGui::End();

    ImGui::Begin("Camera info");
    const Camera& c = programState->camera;
    ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
    ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
    ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
    ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int , int action, int ) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        programState->CameraMouseMovementUpdateEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadCubemap(vector<std::string> &faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}