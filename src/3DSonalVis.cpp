// Std. Includes
#include <string>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "ArcballCamera.h"
#include "pointCloud.h"
#include "FileIO.h"
#include "utilities.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ImGui
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <ImGuiFileBrowser.h>

// Properties
GLuint screenWidth = 1280, screenHeight = 720;
const char* glsl_version = "#version 130";
float pRadius = 6.0f, pThreshold = 2.5f, bRadius = 1.0f, ampThreshold = 0.1f, curAmp = 0.0f;
glm::vec3 worldCoord(0.0f, 0.0f, 0.0f);
GLfloat rectangle[] = {
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
};

ArcballCamera arcballCamera;
glm::mat4 model = glm::mat4(1.0);
glm::mat4 projection = glm::perspective(70.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
int mouseEvent = 0;
glm::vec2 prevMouse(-2.f), curMouse(-2.f);

PointCloud* pointCloud = NULL;
string openFilePath, saveFilePath;
bool toRebind = true;
bool isEditMode = false;

// Function prototypes
void setupImGuiContext(GLFWwindow* window);
void errorCallback(int error, const char* desc);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void scrollCallback(GLFWwindow* window, double x, double y);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorCallback(GLFWwindow* window, double x, double y);
void clip(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
glm::vec2 transformMouse(glm::vec2 in);
glm::vec3 screenCoords2WorldCoords(GLFWwindow* window, double x, double y);

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Point Cloud Viewer", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    /* Set the callback functions */
    glfwSetErrorCallback(errorCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Initialize GLAD to setup the OpenGL Function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup Dear ImGui context
    setupImGuiContext(window);

    // Setup and compile our shaders
    Shader shader("shader/shader.vs", "shader/shader.frag");
    
    GLuint VBOs[2], VAOs[2];
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    // Imgui state
    static imgui_addons::ImGuiFileBrowser file_dialog;
    static bool showOpenFileDialog = false;
    static bool showSaveFileDialog = false;
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check and call events
        glfwPollEvents();

        // Clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw our first triangle
        shader.bind();

        // Create camera transformation
        shader.setUniform("model", model);
        shader.setUniform("view", arcballCamera.transform());
        shader.setUniform("projection", projection);
        if (pointCloud != NULL) {
            if (toRebind) {
                // Point cloud vertices setup
                // Bind our Vertex Array Object first, then bind and set our buffers and pointers.
                glBindVertexArray(VAOs[0]);
                glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * pointCloud->pvNum, 0, GL_STATIC_DRAW);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * pointCloud->pvNum, pointCloud->vPoints);
                glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * pointCloud->pvNum, sizeof(GLfloat) * 3 * pointCloud->pvNum, pointCloud->pColor);
                // Position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                // Color attribute
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * 3 * pointCloud->pvNum));
                glEnableVertexAttribArray(1);
                glBindVertexArray(0); // Unbind VAO
                toRebind = true;
                bRadius = pointCloud->boundingBoxSize * sqrt(20.0f / pointCloud->pvNum);
            }
            glPointSize(4.0f);
            glBindVertexArray(VAOs[0]);
            glDrawArrays(GL_POINTS, 0, pointCloud->pvNum);
        }

        if (isEditMode) {
            shader.setUniform("model", model);
            shader.setUniform("view",model);
            shader.setUniform("projection", model);
            // Rectangle setup (Edit mode)
            glBindVertexArray(VAOs[1]);
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_DYNAMIC_DRAW);
            // Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            // Color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(1);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
        }
        glBindVertexArray(0);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // first window
        {
            ImGui::Begin("Point Cloud Viewer", NULL, ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open..", "Ctrl+O", &showOpenFileDialog)) {}
                    if (ImGui::MenuItem("Save", "Ctrl+S", &showSaveFileDialog)) { /* Do stuff */ }
                    if (ImGui::MenuItem("Close", "Ctrl+W")) { openFilePath = ""; }
                    if (ImGui::MenuItem("Exit", "Alt+F4")) { glfwSetWindowShouldClose(window, GL_TRUE); }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Filter"))
                {
                    if (ImGui::MenuItem("Bilateral Filter")) { /* Do stuff */ }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::MenuItem("Point Cloud")) { /* Do stuff */ }
                    if (ImGui::MenuItem("RANSAC")) { /* Do stuff */ }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            if (showOpenFileDialog)
                ImGui::OpenPopup("Open File");
            if (showSaveFileDialog)
                ImGui::OpenPopup("Save File");
            if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(600, 300)))
            {
                openFilePath = file_dialog.selected_path;    // The absolute path to the selected file
                int size = FileIO::getFileSize(openFilePath);
                float* points = FileIO::openBinaryPointsFile(openFilePath, size);
                if (pointCloud != NULL) {
                    delete pointCloud;
                }
                pointCloud = new PointCloud();
                pointCloud->init(points, size / 4 / sizeof(float), ampThreshold);
                arcballCamera.setParams(pointCloud->centerPoint + glm::vec3(100.0f, 0.0f, 0.0f), pointCloud->centerPoint, glm::vec3(0., 1., 0.));
                showOpenFileDialog = false;
            }
            if (file_dialog.showFileDialog("Save File", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310)))
            {
                saveFilePath = file_dialog.selected_path;    // The absolute path to the selected file
                FileIO::savePointsAsObj(saveFilePath, pointCloud->vPoints, pointCloud->pRegions, pointCloud->pvNum);
                showSaveFileDialog = false;
            }
            if (pointCloud != NULL) {
                ImGui::Text("Total points: %d\n", pointCloud->pvNum);
                ImGui::Text("Isolate points threshold: %d\n", int(pThreshold / 100.0f * pointCloud->pvNum));
                ImGui::Text("Neighbor radius: %.2f cm\n", pRadius / 100.0f * pointCloud->boundingBoxSize);
                ImGui::Text("Coordinate of current point: (%.2f, %.2f, %.2f, %.0e)\n", worldCoord.x, worldCoord.y, worldCoord.z, curAmp);
                ImGui::Spacing();
                if (ImGui::Button("Edit")) {
                    isEditMode = true;
                }
                if (ImGui::Button("View")) {
                    isEditMode = false;
                    rectangle[0] = rectangle[6] = rectangle[12] = rectangle[18] = rectangle[1] = rectangle[7] = rectangle[13] = rectangle[19] = 0;
                }
                if (ImGui::Button("Cut")) {
                }
                if (ImGui::Button("Clear sonar noise")) {
                    pointCloud->clearSonarNoise();
                    toRebind = true;
                }
                if (ImGui::Button("Clear scattering points")) {
                    pointCloud->segment(pRadius / 100.0f * pointCloud->boundingBoxSize, pThreshold / 100.0f * pointCloud->pvNum);
                    toRebind = true;
                }
                if (ImGui::Button("Use bilateral filter")) {
                    pointCloud->useBilateralFilter(bRadius, bRadius);
                    toRebind = true;
                }
                if (ImGui::Button("Reload")) {
                    pointCloud->reset(ampThreshold);
                    toRebind = true;
                }

                ImGui::SliderFloat("(%)isolate points threshold (ANN param)", &pThreshold, 0.0f, 10.0f);
                ImGui::SliderFloat("(%)neighbor radius (ANN param)", &pRadius, 0.0f, 100.0f);
                ImGui::SliderFloat("(cm)bilateral filter radius", &bRadius, 0.0f, 5.0f);
            }
            ImGui::SliderFloat("amplitude threshold", &ampThreshold, 0.0f, 1.0f);

            ImGui::End();
        }
        // second window
        {

        }

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Swap the buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}


void setupImGuiContext(GLFWwindow* window) {

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

}

void errorCallback(int error, const char* desc) {
    fputs(desc, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            //glfwSetWindowShouldClose(window, GL_TRUE);
            return;
        case GLFW_KEY_C:
            clip(rectangle[0], rectangle[1], rectangle[12], rectangle[13]);
            toRebind = true;
            return;
        case GLFW_KEY_V:
            isEditMode = false;
            rectangle[0] = rectangle[6] = rectangle[12] = rectangle[18] = rectangle[1] = rectangle[7] = rectangle[13] = rectangle[19] = 0;
            return;
        case GLFW_KEY_E:
            isEditMode = true;
            return;
        case GLFW_KEY_R:
            pointCloud->reset(ampThreshold);
            toRebind = true;
            return;
        case GLFW_KEY_S:
            return;
        default:
            break;
        }
    }
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow* window, double x, double y) {
    arcballCamera.zoom(y * 1.5);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        mouseEvent = 1;
        if (isEditMode) {
            rectangle[0] = rectangle[6] = rectangle[12] = rectangle[18] = curMouse.x;
            rectangle[1] = rectangle[7] = rectangle[13] = rectangle[19] = curMouse.y;
        }
    }
    else if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
        mouseEvent = 2;
    }
    else {
        mouseEvent = 0;
    }

}

void cursorCallback(GLFWwindow* window, double x, double y) {
    if (pointCloud == NULL) { return; }
    worldCoord = screenCoords2WorldCoords(window, x, y);
    curAmp = pointCloud->ampMap[getPointStr(worldCoord.x, worldCoord.y, worldCoord.z)];
    curMouse = transformMouse(glm::vec2(x, y));
    if (!isEditMode) {
        if (mouseEvent == 1) {
            arcballCamera.rotate(prevMouse, curMouse);
        }
        else if (mouseEvent == 2) {
            arcballCamera.pan((curMouse - prevMouse) * 0.1f);
        }
    }
    else {
        if (mouseEvent == 1) {
            rectangle[12] = curMouse.x;
            rectangle[13] = curMouse.y;
            rectangle[6] = rectangle[0];
            rectangle[7] = rectangle[13];
            rectangle[18] = rectangle[12];
            rectangle[19] = rectangle[1];
        }
        
    }
    prevMouse = curMouse;
    
}

void clip(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
    if (x1 == x2 || y1 == y2) { return; }

    GLfloat minx = min(x1, x2);
    GLfloat maxx = max(x1, x2);
    GLfloat miny = min(y1, y2);
    GLfloat maxy = max(y1, y2);

    glm::mat4 view = arcballCamera.transform();

    for (int i = 0; i < pointCloud->pvNum; i++) {
        glm::vec4 pos = projection * view * model* glm::vec4(pointCloud->vPoints[i*6], pointCloud->vPoints[i*6+1], pointCloud->vPoints[i*6+2], 1.0f);
        pos.x = pos.x / pos.w;
        pos.y = pos.y / pos.w;
        if (pos.x >= minx && pos.x <= maxx && pos.y >= miny && pos.y <= maxy) {
            pointCloud->pFlag[i] = false;
        }
    }
    pointCloud->transform();
}

glm::vec2 transformMouse(glm::vec2 in)
{
    return glm::vec2(in.x * 2.f / screenWidth - 1.f, 1.f - 2.f * in.y / screenHeight);
}

glm::vec3 screenCoords2WorldCoords(GLFWwindow* window, double x, double y) {
    int screen_w, screen_h, pixel_w, pixel_h;
    //double xpos, ypos;
    glfwGetWindowSize(window, &screen_w, &screen_h); // better use the callback and cache the values 
    glfwGetFramebufferSize(window, &pixel_w, &pixel_h); // better use the callback and cache the values
    //glfwGetCursorPos(window, &xpos, &ypos);
    //glm::vec2 screen_pos = glm::vec2(xpos, ypos);
    glm::vec2 screen_pos = glm::vec2(x, y);
    glm::vec2 pixel_pos = screen_pos * glm::vec2(pixel_w, pixel_h) / glm::vec2(screen_w, screen_h); // note: not necessarily integer
    pixel_pos = pixel_pos + glm::vec2(0.5f, 0.5f); // shift to GL's center convention
    glm::vec3 win = glm::vec3(pixel_pos.x, pixel_h - pixel_pos.y, 0.0f);
    glReadPixels((GLint)win.x, (GLint)win.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win.z);
    glm::vec3 world = glm::unProject(win, arcballCamera.transform(), projection, glm::vec4(0.0f, 0.0f, (float)screen_w, (float)screen_h));
    return world;
}
