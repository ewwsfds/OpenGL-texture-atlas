#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>


// Vertex shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
uniform vec3 offset;
uniform vec2 cameraPos; // 2D camera position
void main()
{
    vec3 movemnt = vec3(aPos+offset);
    gl_Position = vec4(movemnt.xy - cameraPos, movemnt.z, 1.0); // subtract camera
    TexCoord = aTexCoord;
}
)";

// Fragment shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}
)";




void addQuadIndexed( std::vector<float>& vertices, std::vector<unsigned int>& indices, float x, float y, float w, float h, float u0, float v0, float u1, float v1) // UV coordinates in atlas
{
    unsigned int start = vertices.size() / 5; // 5 floats per vertex

    // 4 unique vertices
    vertices.push_back(x);     vertices.push_back(y + h); vertices.push_back(0.0f); vertices.push_back(u0); vertices.push_back(v1); // top-left
    vertices.push_back(x);     vertices.push_back(y);     vertices.push_back(0.0f); vertices.push_back(u0); vertices.push_back(v0); // bottom-left
    vertices.push_back(x + w); vertices.push_back(y);     vertices.push_back(0.0f); vertices.push_back(u1); vertices.push_back(v0); // bottom-right
    vertices.push_back(x + w); vertices.push_back(y + h); vertices.push_back(0.0f); vertices.push_back(u1); vertices.push_back(v1); // top-right

    // Indices for the two triangles
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);

    indices.push_back(start + 0);
    indices.push_back(start + 2);
    indices.push_back(start + 3);
}







int main()
{
    // ----------------------------
    // 1️⃣ Initialize GLFW
    // ----------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Textured Triangles", NULL, NULL);
    if (!window) { std::cout << "Failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    // ----------------------------
    // 2️⃣ Initialize GLAD
    // ----------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "GLAD failed\n"; return -1; }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ----------------------------
    // 3️⃣ Vertex data + texture coords
    // ----------------------------
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // First quad uses left half of atlas
    addQuadIndexed(vertices, indices, -1.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.8f, 0.2f, 0.7f); // Important
    addQuadIndexed(vertices, indices, 0.0f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.2f); // Important




    // ----------------------------
    // 4️⃣ Setup VAO and VBO 
    // ----------------------------
    unsigned int VAO, VBO, EBO; // important
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO); //important


    glBindVertexArray(VAO);

    // VBO for vertices --- Important
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // EBO for indices -- IMPORTANT
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);




    // ----------------------------
    // 5️⃣ Load texture
    // ----------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture wrapping/filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // perfect for texture atlas
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // perfect for texture atlas

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("49051001062.png", &width, &height, &nrChannels, 4);
    if (data)
    {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );


        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture\n";
    }
    stbi_image_free(data);

    // ----------------------------
    // 6️⃣ Compile shaders
    // ----------------------------
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //name-depends on fragmentshader/vertex shaders
    int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    int textureLoc = glGetUniformLocation(shaderProgram, "ourTexture");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    float cameraX = 0.0f;
    float cameraY = 0.0f;
    float cameraSpeed = 1.0f; // units per second

    int cameraLoc = glGetUniformLocation(shaderProgram, "cameraPos");
    // ----------------------------
    // 7️⃣ Render loop
    // ----------------------------
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureLoc, 0);

        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraY += cameraSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraY -= cameraSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraX -= cameraSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraX += cameraSpeed * deltaTime;


        float time = (float)glfwGetTime();

        // draw first triangle (moving)
        glUniform2f(cameraLoc, cameraX, cameraY);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ----------------------------
    // 8️⃣ Cleanup
    // ----------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}
