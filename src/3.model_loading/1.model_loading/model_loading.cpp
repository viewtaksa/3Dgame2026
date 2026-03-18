#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>

// ---------------- SETTINGS ----------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ---------------- CAMERA ----------------
Camera camera(glm::vec3(0.0f, 5.0f, 8.0f));

// ---------------- TIME ----------------
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ---------------- GAME ----------------
bool gameOver = false;
float startTime = 0.0f;
int score = 0;

// ---------------- PLAYER ----------------
glm::vec3 playerPos(0.0f);
float speed = 5.0f;

// ---------------- ENEMY ----------------
glm::vec3 enemyPos(-10.0f, 0.0f, -10.0f);

// ---------------- COINS ----------------
glm::vec3 coins[3] = {
    {3,0,3}, {-3,0,2}, {2,0,-4}
};
bool collected[3] = { false, false, false };

// ---------------- COLLISION ----------------
bool collide(glm::vec3 a, glm::vec3 b, float d) {
    return glm::length(a - b) < d;
}

// ---------------- INPUT ----------------
void input(GLFWwindow* window) {
    if (gameOver) return;

    float v = speed * deltaTime;
    glm::vec3 prev = playerPos;

    if (glfwGetKey(window, GLFW_KEY_W)) playerPos.z -= v;
    if (glfwGetKey(window, GLFW_KEY_S)) playerPos.z += v;
    if (glfwGetKey(window, GLFW_KEY_A)) playerPos.x -= v;
    if (glfwGetKey(window, GLFW_KEY_D)) playerPos.x += v;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
}

// ---------------- MAIN ----------------
int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cube Game", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // ✅ ใช้ shader ที่มี uniform vec3 color
    Shader shader(
        "C:/Users/khunn/OneDrive/Documents/view/3Dgame/src/3.model_loading/1.model_loading/1.model_loading.vs",
        "C:/Users/khunn/OneDrive/Documents/view/3Dgame/src/3.model_loading/1.model_loading/1.model_loading.fs"
    );

    // -------- cube vertices --------
    float vertices[] = {
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,0.5f,-0.5f,
         0.5f,0.5f,-0.5f, -0.5f,0.5f,-0.5f, -0.5f,-0.5f,-0.5f,

        -0.5f,-0.5f,0.5f,  0.5f,-0.5f,0.5f,  0.5f,0.5f,0.5f,
         0.5f,0.5f,0.5f, -0.5f,0.5f,0.5f, -0.5f,-0.5f,0.5f,

        -0.5f,0.5f,0.5f, -0.5f,0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,0.5f, -0.5f,0.5f,0.5f,

         0.5f,0.5f,0.5f,  0.5f,0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,  0.5f,-0.5f,0.5f,  0.5f,0.5f,0.5f,

        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f,0.5f,
         0.5f,-0.5f,0.5f, -0.5f,-0.5f,0.5f, -0.5f,-0.5f,-0.5f,

        -0.5f,0.5f,-0.5f,  0.5f,0.5f,-0.5f,  0.5f,0.5f,0.5f,
         0.5f,0.5f,0.5f, -0.5f,0.5f,0.5f, -0.5f,0.5f,-0.5f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    startTime = glfwGetTime();

    // ---------------- LOOP ----------------
    while (!glfwWindowShouldClose(window))
    {
        float current = glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;

        input(window);

        // enemy move (delay 2 sec)
        if (!gameOver && current - startTime > 2.0f) {
            glm::vec3 dir = playerPos - enemyPos;
            if (glm::length(dir) > 0.01f)
                enemyPos += glm::normalize(dir) * deltaTime * 2.0f;
        }

        // coin collision
        for (int i = 0; i < 3; i++) {
            if (!collected[i] && collide(playerPos, coins[i], 1.0f)) {
                collected[i] = true;
                score++;
                std::cout << "Score: " << score << "\n";
            }
        }

        // win
        if (score == 3 && !gameOver) {
            std::cout << "YOU WIN!\n";
            gameOver = true;
        }

        // lose
        if (!gameOver && collide(playerPos, enemyPos, 1.2f)) {
            std::cout << "GAME OVER\n";
            gameOver = true;
        }

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        camera.Position = playerPos + glm::vec3(0, 5, 8);
        camera.Front = glm::normalize(playerPos - camera.Position);

        glm::mat4 view = camera.GetViewMatrix();

        shader.setMat4("projection", proj);
        shader.setMat4("view", view);

        glBindVertexArray(VAO);

        // 🔵 PLAYER
        glm::mat4 m = glm::translate(glm::mat4(1), playerPos);
        shader.setMat4("model", m);
        shader.setVec3("color", glm::vec3(0.2f, 0.6f, 1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 🔴 ENEMY
        m = glm::translate(glm::mat4(1), enemyPos);
        m = glm::scale(m, glm::vec3(1.2f));
        shader.setMat4("model", m);
        shader.setVec3("color", glm::vec3(1.0f, 0.2f, 0.2f));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 🟡 COINS
        for (int i = 0; i < 3; i++) {
            if (!collected[i]) {
                m = glm::translate(glm::mat4(1), coins[i]);
                m = glm::scale(m, glm::vec3(0.4f));
                shader.setMat4("model", m);
                shader.setVec3("color", glm::vec3(1.0f, 0.9f, 0.2f));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // 🟩 GROUND
        m = glm::translate(glm::mat4(1), glm::vec3(0, -1, 0));
        m = glm::scale(m, glm::vec3(20, 0.1, 20));
        shader.setMat4("model", m);
        shader.setVec3("color", glm::vec3(0.2f, 0.8f, 0.3f));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}