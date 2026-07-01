#include "core/DebugUI.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>     // vendored (third_party/imgui)
#include <imgui_impl_opengl3.h>  // from the vcpkg imgui[opengl3-binding] port

bool DebugUI::init(SDL_Window* window, SDL_GLContext context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    if (!ImGui_ImplSDL2_InitForOpenGL(window, context)) return false;
    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) return false;
    m_ready = true;
    return true;
}

DebugUI::~DebugUI() {
    if (!m_ready) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::processEvent(const SDL_Event& e) {
    if (m_ready) ImGui_ImplSDL2_ProcessEvent(&e);
}

void DebugUI::beginFrame() {
    if (!m_ready) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::endFrame() {
    if (!m_ready) return;
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool DebugUI::wantsMouse()    const { return m_ready && ImGui::GetIO().WantCaptureMouse; }
bool DebugUI::wantsKeyboard() const { return m_ready && ImGui::GetIO().WantCaptureKeyboard; }
