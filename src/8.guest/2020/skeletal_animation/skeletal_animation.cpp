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
#include <vector>

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// --- Window Settings ---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// --- Global Camera Object ---
Camera camera(glm::vec3(0.0f, 3.0f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// --- Timing ---
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- Character Control States ---
glm::vec3 characterPos = glm::vec3(0.0f, 0.0f, 0.0f);
float characterRotation = 0.0f;
const float BASE_SPEED = 3.0f;
const float RUN_MULTIPLIER = 2.0f;

int main()
{
    // 1. GLFW Initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modern Animation Framework", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    // 2. Load Shaders
    Shader animationShader("anim_model.vs", "anim_model.fs");

    // 3. Load Character Assets (ปรับ Path ให้ตรงกับเครื่องคุณ)
    Model characterModel(FileSystem::getPath("resources/objects/homework4/Dancing_Twerk/Dancing_Twerk.dae"));

    // ใช้ Vector/Array เก็บ Animations เพื่อให้จัดการง่ายและโค้ดไม่รกรุงรัง
    std::vector<Animation> animationList;
    animationList.emplace_back(FileSystem::getPath("resources/objects/homework4/Dancing_Twerk/Dancing_Twerk.dae"), &characterModel);       // Index 0: Idle
    animationList.emplace_back(FileSystem::getPath("resources/objects/homework4/Female_Standing_Pose/Female_Standing_Pose.dae"), &characterModel); // Index 1: Walk
    animationList.emplace_back(FileSystem::getPath("resources/objects/homework4/jumping_down/Jumping_Down.dae"), &characterModel);       // Index 2: Action 1
    animationList.emplace_back(FileSystem::getPath("resources/objects/homework4/Silly_Dancing/Silly_Dancing.dae"), &characterModel);       // Index 3: Action 2
    animationList.emplace_back(FileSystem::getPath("resources/objects/homework4/Twist_Dance/Twist_Dance.dae"), &characterModel);        // Index 4: Action 3

    Animator animator(&animationList[0]);
    int activeAnimationIndex = 0;

    // 4. Render Loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // --- Logic: Character Movement & Animation States ---
        bool isMoving = false;
        float currentSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? (BASE_SPEED * RUN_MULTIPLIER) : BASE_SPEED;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { characterPos.z -= currentSpeed * deltaTime; characterRotation = 180.0f; isMoving = true; }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { characterPos.z += currentSpeed * deltaTime; characterRotation = 0.0f;   isMoving = true; }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { characterPos.x -= currentSpeed * deltaTime; characterRotation = -90.0f; isMoving = true; }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { characterPos.x += currentSpeed * deltaTime; characterRotation = 90.0f;  isMoving = true; }

        // Animation Selector
        int targetAnimIndex = 0; // Default to Idle
        if (isMoving) targetAnimIndex = 1; // Change to Walk/Run
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) targetAnimIndex = 4;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) targetAnimIndex = 2;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) targetAnimIndex = 3;

        // Switch animation only when index changes
        if (activeAnimationIndex != targetAnimIndex) {
            activeAnimationIndex = targetAnimIndex;
            animator.PlayAnimation(&animationList[activeAnimationIndex]);
        }

        animator.UpdateAnimation(deltaTime);

        // --- Render Stage ---
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f); // Dark Slate Blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        animationShader.use();

        // Matrix Setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        animationShader.setMat4("projection", projection);
        animationShader.setMat4("view", view);

        // Bone Transformation
        auto boneTransforms = animator.GetFinalBoneMatrices();
        for (unsigned int i = 0; i < boneTransforms.size(); ++i) {
            animationShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", boneTransforms[i]);
        }

        // Model Matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, characterPos);
        model = glm::rotate(model, glm::radians(characterRotation), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(1.0f));
        animationShader.setMat4("model", model);

        characterModel.Draw(animationShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --- Callback Implementations ---
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}