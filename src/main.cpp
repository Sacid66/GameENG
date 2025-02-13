#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"       
#include "backends/imgui_impl_opengl2.h"     

#include <vector>
#include <cmath>


const int SCREEN_WIDTH  = 1920;
const int SCREEN_HEIGHT = 1080;
const int PANEL_WIDTH   = 300; 


enum ObjectType {
    OBJ_NONE = 0,
    OBJ_CUBE,
    OBJ_TRI_PRISM,
    OBJ_CONE,
    OBJ_CYLINDER,
    OBJ_SPHERE,
    OBJ_DIR_LIGHT  
};


struct SceneObject {
    ObjectType type;
    float posX, posY, posZ;
};


std::vector<SceneObject> sceneObjects;
int selectedObjectIndex = -1; 


int activeAxis = -1;  
int gizmoLastMouseX = 0, gizmoLastMouseY = 0;


float camX = 0.0f, camY = 5.0f, camZ = 15.0f;
float camYaw = 0.0f;   
float camPitch = 0.0f;  


GLUquadric* quadric = nullptr;


bool pendingDrop = false;
int pendingDropX = 0, pendingDropY = 0; 
ObjectType pendingDropType = OBJ_NONE;


void GetGroundIntersection(int mouseX, int mouseY,
                           int viewportX, int viewportY, int viewportWidth, int viewportHeight,
                           double* ix, double* iy, double* iz);
bool ProjectWorldToScreen(double worldX, double worldY, double worldZ, double outScreen[2]);
bool IsMouseNearLine2D(float mouseX, float mouseY, const double screenPos1[2], const double screenPos2[2], float threshold);


bool ProjectWorldToScreen(double worldX, double worldY, double worldZ, double outScreen[2])
{
    double modelview[16], projection[16];
    int vp[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, vp);
    double winX, winY, winZ;
    if (!gluProject(worldX, worldY, worldZ, modelview, projection, vp, &winX, &winY, &winZ))
        return false;
    outScreen[0] = winX;
    outScreen[1] = vp[3] - winY; 
    return true;
}


bool IsMouseNearLine2D(float mouseX, float mouseY, const double screenPos1[2], const double screenPos2[2], float threshold)
{
    double dx = screenPos2[0] - screenPos1[0];
    double dy = screenPos2[1] - screenPos1[1];
    double length2 = dx * dx + dy * dy;
    double t = 0;
    if (length2 > 1e-6) {
        t = ((mouseX - screenPos1[0]) * dx + (mouseY - screenPos1[1]) * dy) / length2;
        if(t < 0) t = 0;
        if(t > 1) t = 1;
    }
    double projX = screenPos1[0] + t * dx;
    double projY = screenPos1[1] + t * dy;
    double dist = sqrt((mouseX - projX) * (mouseX - projX) + (mouseY - projY) * (mouseY - projY));
    return (dist < threshold);
}


void GetGroundIntersection(int mouseX, int mouseY,
                           int viewportX, int viewportY, int viewportWidth, int viewportHeight,
                           double* ix, double* iy, double* iz)
{
    GLint vp[4] = { viewportX, viewportY, viewportWidth, viewportHeight };
    double modelview[16], projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    int realY = viewportY + viewportHeight - mouseY;
    double nearX, nearY, nearZ;
    double farX, farY, farZ;
    gluUnProject(mouseX, realY, 0.0, modelview, projection, vp, &nearX, &nearY, &nearZ);
    gluUnProject(mouseX, realY, 1.0, modelview, projection, vp, &farX, &farY, &farZ);
    double dirX = farX - nearX;
    double dirY = farY - nearY;
    double dirZ = farZ - nearZ;
    double t = (fabs(dirY) < 1e-6) ? 0.0 : -nearY / dirY;
    *ix = nearX + t * dirX;
    *iy = 0;
    *iz = nearZ + t * dirZ;
}



void DrawCube() {
    glBegin(GL_QUADS);
      glNormal3f(0, 0, 1);
      glColor3f(1, 0, 0);
      glVertex3f(-0.5f, -0.5f, 0.5f);
      glVertex3f(0.5f, -0.5f, 0.5f);
      glVertex3f(0.5f, 0.5f, 0.5f);
      glVertex3f(-0.5f, 0.5f, 0.5f);
    glEnd();
    glBegin(GL_QUADS);
      glNormal3f(0, 0, -1);
      glColor3f(0, 1, 0);
      glVertex3f(-0.5f, -0.5f, -0.5f);
      glVertex3f(-0.5f, 0.5f, -0.5f);
      glVertex3f(0.5f, 0.5f, -0.5f);
      glVertex3f(0.5f, -0.5f, -0.5f);
    glEnd();
    glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glColor3f(0, 0, 1);
      glVertex3f(-0.5f, 0.5f, -0.5f);
      glVertex3f(-0.5f, 0.5f, 0.5f);
      glVertex3f(0.5f, 0.5f, 0.5f);
      glVertex3f(0.5f, 0.5f, -0.5f);
    glEnd();
    glBegin(GL_QUADS);
      glNormal3f(0, -1, 0);
      glColor3f(1, 1, 0);
      glVertex3f(-0.5f, -0.5f, -0.5f);
      glVertex3f(0.5f, -0.5f, -0.5f);
      glVertex3f(0.5f, -0.5f, 0.5f);
      glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();
    glBegin(GL_QUADS);
      glNormal3f(1, 0, 0);
      glColor3f(1, 0, 1);
      glVertex3f(0.5f, -0.5f, -0.5f);
      glVertex3f(0.5f, 0.5f, -0.5f);
      glVertex3f(0.5f, 0.5f, 0.5f);
      glVertex3f(0.5f, -0.5f, 0.5f);
    glEnd();
    glBegin(GL_QUADS);
      glNormal3f(-1, 0, 0);
      glColor3f(0, 1, 1);
      glVertex3f(-0.5f, -0.5f, -0.5f);
      glVertex3f(-0.5f, -0.5f, 0.5f);
      glVertex3f(-0.5f, 0.5f, 0.5f);
      glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();
}


void DrawTriangularPrism() { }
void DrawCone() {  }
void DrawCylinder() {  }
void DrawSphere() {  }

void DrawGrid() {
    int gridSize = 20;
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    for (int i = -gridSize; i <= gridSize; i++) {
        glVertex3f((float)i, 0.0f, (float)-gridSize);
        glVertex3f((float)i, 0.0f, (float)gridSize);
        glVertex3f((float)-gridSize, 0.0f, (float)i);
        glVertex3f((float)gridSize, 0.0f, (float)i);
    }
    glEnd();
}

void DrawGizmo(const SceneObject& obj) {
    float arrowLength = 1.0f;
    glLineWidth(3.0f);
    glBegin(GL_LINES);
      glColor3f(1, 0, 0); 
      glVertex3f(obj.posX, obj.posY, obj.posZ);
      glVertex3f(obj.posX + arrowLength, obj.posY, obj.posZ);
      glColor3f(0, 1, 0); 
      glVertex3f(obj.posX, obj.posY, obj.posZ);
      glVertex3f(obj.posX, obj.posY + arrowLength, obj.posZ);
      glColor3f(0, 0, 1); 
      glVertex3f(obj.posX, obj.posY, obj.posZ);
      glVertex3f(obj.posX, obj.posY, obj.posZ + arrowLength);
    glEnd();
    glLineWidth(1.0f);
}


int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL Başlatılamadı: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    SDL_Window* window = SDL_CreateWindow("Indie Game Engine",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
 
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();
    
    quadric = gluNewQuadric();
    
    while (true)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                goto exit_loop;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                goto exit_loop;
        }
        
        
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        float moveSpeed = 0.5f;
        if (!io.WantCaptureKeyboard) {
            float forwardX = sinf(camYaw);
            float forwardZ = -cosf(camYaw);
            float rightX = cosf(camYaw);
            float rightZ = sinf(camYaw);
            if (keystate[SDL_SCANCODE_W]) { camX += forwardX * moveSpeed; camZ += forwardZ * moveSpeed; }
            if (keystate[SDL_SCANCODE_S]) { camX -= forwardX * moveSpeed; camZ -= forwardZ * moveSpeed; }
            if (keystate[SDL_SCANCODE_A]) { camX -= rightX * moveSpeed; camZ -= rightZ * moveSpeed; }
            if (keystate[SDL_SCANCODE_D]) { camX += rightX * moveSpeed; camZ += rightZ * moveSpeed; }
        }
        
        int mouseX, mouseY;
        Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
        if ((mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) && !io.WantCaptureMouse) {
            int xrel, yrel;
            SDL_GetRelativeMouseState(&xrel, &yrel);
            float sensitivity = 0.005f;
            camYaw += xrel * sensitivity;
            camPitch += yrel * sensitivity;
            if (camPitch > 1.57f) camPitch = 1.57f;
            if (camPitch < -1.57f) camPitch = -1.57f;
        }
        
        
        if ((mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) && activeAxis == -1 && selectedObjectIndex != -1) {
            SceneObject& selObj = sceneObjects[selectedObjectIndex];
            double baseScreen[2], xScreen[2], yScreen[2], zScreen[2];
            if (ProjectWorldToScreen(selObj.posX, selObj.posY, selObj.posZ, baseScreen) &&
                ProjectWorldToScreen(selObj.posX + 1.0, selObj.posY, selObj.posZ, xScreen) &&
                ProjectWorldToScreen(selObj.posX, selObj.posY + 1.0, selObj.posZ, yScreen) &&
                ProjectWorldToScreen(selObj.posX, selObj.posY, selObj.posZ + 1.0, zScreen)) {
                float threshold = 10.0f;
                if (IsMouseNearLine2D((float)mouseX, (float)mouseY, baseScreen, xScreen, threshold)) {
                    activeAxis = 0;
                    gizmoLastMouseX = mouseX;
                    gizmoLastMouseY = mouseY;
                } else if (IsMouseNearLine2D((float)mouseX, (float)mouseY, baseScreen, yScreen, threshold)) {
                    activeAxis = 1;
                    gizmoLastMouseX = mouseX;
                    gizmoLastMouseY = mouseY;
                } else if (IsMouseNearLine2D((float)mouseX, (float)mouseY, baseScreen, zScreen, threshold)) {
                    activeAxis = 2;
                    gizmoLastMouseX = mouseX;
                    gizmoLastMouseY = mouseY;
                }
            }
        }
        
        if (activeAxis != -1 && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            int dx = mouseX - gizmoLastMouseX;
            int dy = mouseY - gizmoLastMouseY;
            float factor = 0.01f;
            if (activeAxis == 0)
                sceneObjects[selectedObjectIndex].posX += dx * factor;
            else if (activeAxis == 1)
                sceneObjects[selectedObjectIndex].posY -= dy * factor;
            else if (activeAxis == 2)
                sceneObjects[selectedObjectIndex].posZ += dx * factor;
            gizmoLastMouseX = mouseX;
            gizmoLastMouseY = mouseY;
        }
        
        
        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui::NewFrame();
        
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f,0.2f,0.2f,1.0f));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((float)PANEL_WIDTH, (float)SCREEN_HEIGHT), ImGuiCond_Always);
        ImGui::Begin("Elements", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::Text("Drag and drop elements:");
            if (ImGui::Button("Cube", ImVec2(-1, 40))) { }
            if (ImGui::BeginDragDropSource()) {
                ObjectType type = OBJ_CUBE;
                ImGui::SetDragDropPayload("OBJECT_TYPE", &type, sizeof(ObjectType));
                ImGui::Text("Cube");
                ImGui::EndDragDropSource();
            }
            if (ImGui::Button("Triangular Prism", ImVec2(-1, 40))) { }
            if (ImGui::BeginDragDropSource()) {
                ObjectType type = OBJ_TRI_PRISM;
                ImGui::SetDragDropPayload("OBJECT_TYPE", &type, sizeof(ObjectType));
                ImGui::Text("Triangular Prism");
                ImGui::EndDragDropSource();
            }
            if (ImGui::Button("Cone", ImVec2(-1, 40))) { }
            if (ImGui::BeginDragDropSource()) {
                ObjectType type = OBJ_CONE;
                ImGui::SetDragDropPayload("OBJECT_TYPE", &type, sizeof(ObjectType));
                ImGui::Text("Cone");
                ImGui::EndDragDropSource();
            }
            if (ImGui::Button("Cylinder", ImVec2(-1, 40))) { }
            if (ImGui::BeginDragDropSource()) {
                ObjectType type = OBJ_CYLINDER;
                ImGui::SetDragDropPayload("OBJECT_TYPE", &type, sizeof(ObjectType));
                ImGui::Text("Cylinder");
                ImGui::EndDragDropSource();
            }
            if (ImGui::Button("Sphere", ImVec2(-1, 40))) { }
            if (ImGui::BeginDragDropSource()) {
                ObjectType type = OBJ_SPHERE;
                ImGui::SetDragDropPayload("OBJECT_TYPE", &type, sizeof(ObjectType));
                ImGui::Text("Sphere");
                ImGui::EndDragDropSource();
            }
            if (ImGui::Button("Sun Light", ImVec2(-1, 40))) { }
            if (ImGui::BeginDragDropSource()) {
                ObjectType type = OBJ_DIR_LIGHT;
                ImGui::SetDragDropPayload("OBJECT_TYPE", &type, sizeof(ObjectType));
                ImGui::Text("Sun Light");
                ImGui::EndDragDropSource();
            }
        ImGui::End();
        ImGui::PopStyleColor();
        

ImGui::SetNextWindowPos(ImVec2((float)PANEL_WIDTH, 0), ImGuiCond_Always);
ImGui::SetNextWindowSize(ImVec2((float)(SCREEN_WIDTH - PANEL_WIDTH), (float)SCREEN_HEIGHT), ImGuiCond_Always);
ImGui::Begin("Scene", NULL,
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);


if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT_TYPE")) {
        
        ObjectType droppedType = *(const ObjectType*)payload->Data;
        
        ImVec2 dropPos = ImGui::GetMousePos();
        
        pendingDrop = true;
        pendingDropX = (int)dropPos.x;
        pendingDropY = (int)dropPos.y;
        pendingDropType = droppedType;
    }
    ImGui::EndDragDropTarget();
}
ImGui::End();



        
        ImGui::Render();
        
       
glViewport(PANEL_WIDTH, 0, SCREEN_WIDTH - PANEL_WIDTH, SCREEN_HEIGHT);
glClearColor(0.1f, 0.1f, 0.1f, 1);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glEnable(GL_DEPTH_TEST);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluPerspective(45.0, (double)(SCREEN_WIDTH - PANEL_WIDTH) / SCREEN_HEIGHT, 0.1, 1000.0);
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
float viewDirX = sinf(camYaw) * cosf(camPitch);
float viewDirY = sinf(camPitch);
float viewDirZ = -cosf(camYaw) * cosf(camPitch);
gluLookAt(camX, camY, camZ, camX + viewDirX, camY + viewDirY, camZ + viewDirZ, 0, 1, 0);


if (pendingDrop) {
    int viewportX = PANEL_WIDTH;  
    int viewportY = 0;
    int viewportWidth = SCREEN_WIDTH - PANEL_WIDTH;
    int viewportHeight = SCREEN_HEIGHT;
    double ix, iy, iz;
    GetGroundIntersection(pendingDropX, pendingDropY,
                          viewportX, viewportY, viewportWidth, viewportHeight,
                          &ix, &iy, &iz);
    sceneObjects.push_back({ pendingDropType, (float)ix, (float)iy, (float)iz });
    selectedObjectIndex = sceneObjects.size() - 1;
    pendingDrop = false;
}
        
        
        glDisable(GL_LIGHTING);
        DrawGrid();
        
        
        bool hasDirLight = false;
        for (const auto &obj : sceneObjects) {
            if (obj.type == OBJ_DIR_LIGHT) {
                hasDirLight = true;
                float lightDir[4] = { obj.posX, obj.posY, obj.posZ, 0.0f };
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
                float ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
                float diffuse[4] = {0.8f, 0.8f, 0.8f, 1.0f};
                glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
                break;
            }
        }
        if (!hasDirLight)
            glDisable(GL_LIGHTING);
        
        
        for (size_t i = 0; i < sceneObjects.size(); i++) {
            if (sceneObjects[i].type != OBJ_DIR_LIGHT) {
                glPushMatrix();
                    glTranslatef(sceneObjects[i].posX, sceneObjects[i].posY, sceneObjects[i].posZ);
                    switch(sceneObjects[i].type) {
                        case OBJ_CUBE:       DrawCube(); break;
                        case OBJ_TRI_PRISM:  DrawTriangularPrism(); break;
                        case OBJ_CONE:       DrawCone(); break;
                        case OBJ_CYLINDER:   DrawCylinder(); break;
                        case OBJ_SPHERE:     DrawSphere(); break;
                        default: break;
                    }
                glPopMatrix();
            }
        }
        glDisable(GL_LIGHTING);
        
        
        if (selectedObjectIndex != -1) {
            glDisable(GL_DEPTH_TEST);
            DrawGizmo(sceneObjects[selectedObjectIndex]);
            glEnable(GL_DEPTH_TEST);
        }
        
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }
    
exit_loop:
    gluDeleteQuadric(quadric);
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
