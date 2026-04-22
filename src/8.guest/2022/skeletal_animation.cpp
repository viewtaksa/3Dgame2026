#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>

// --- Functions ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// --- Settings ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- Camera ---
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// --- Timing ---
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- Playable Character State ---
glm::vec3 characterPos = glm::vec3(0.0f, -0.4f, 0.0f);
float characterRotation = 0.0f;
bool isMoving = false;

int main()
{
    // 1. Initializing GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Character Animation Control", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    // 2. Load Shaders
    Shader ourShader("anim_model.vs", "anim_model.fs");
    std::cout << "Shader loaded successfully!" << std::endl;

    // 3. Load Model & Animation (แก้ไขจุดประกาศซ้ำตรงนี้แล้ว)
    std::string modelPath = FileSystem::getPath("resources/objects/vampire/dancing_vampire.dae");
    Model ourModel(modelPath);

    if (ourModel.meshes.size() == 0) {
        std::cout << "MODEL ERROR: Model not loaded! Check path: " << modelPath << std::endl;
    }
    else {
        std::cout << "Model loaded: " << ourModel.meshes.size() << " meshes found." << std::endl;
    }

    Animation danceAnimation(modelPath, &ourModel);
    Animator animator(&danceAnimation);

    // 4. Render Loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Update Animation
        float animationSpeed = isMoving ? 1.4f : 0.6f;
        animator.UpdateAnimation(deltaTime * animationSpeed);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // Matrix: View & Projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Animation Bones
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < (int)transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // --- Model Matrix (Movement & Rotation) ---
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, characterPos);
        model = glm::rotate(model, glm::radians(characterRotation), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
        ourShader.setMat4("model", model);

        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    isMoving = false;
    float velocity = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movement: Arrow Keys
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        characterPos.z -= velocity;
        characterRotation = 180.0f;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        characterPos.z += velocity;
        characterRotation = 0.0f;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        characterPos.x -= velocity;
        characterRotation = -90.0f;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        characterPos.x += velocity;
        characterRotation = 90.0f;
        isMoving = true;
    }

    // Camera: WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float xoffset = (float)xpos - lastX; float yoffset = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos; camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll((float)yoffset); }