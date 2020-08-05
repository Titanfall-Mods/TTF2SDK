#include "stdafx.h"

Preloader::Preloader(const std::vector<std::string>& maps) :
    m_state(PRELOADER_STATE_PRELOAD_NEXT),
    m_mapsToPreload(maps),
    m_totalMaps(maps.size())
{
    SDK().GetUIManager().AddDrawCallback("Preload_UI", std::bind(&Preloader::DrawUI, this));
}

Preloader::~Preloader()
{

}

void Preloader::RunFrame()
{
    if (m_state == PRELOADER_STATE_PRELOAD_NEXT)
    {
        if (m_mapsToPreload.size() == 0)
        {
            SDK().GetPakManager().SortCachedMaterialData();
            SDK().GetPakManager().WritePakCache();
            SDK().GetUIManager().RemoveDrawCallback("Preload_UI");
            m_state = PRELOADER_STATE_FINISHED;
        }
        else
        {
            m_state = PRELOADER_STATE_PRELOADING;
            std::string mapName = m_mapsToPreload.back() + ".rpak";
            SDK().GetPakManager().PreloadPak(mapName.c_str());
            m_mapsToPreload.pop_back();
            m_framesToWait = 5;
            m_state = PRELOADER_STATE_WAIT;
        }
    }
    else if (m_state == PRELOADER_STATE_WAIT)
    {
        m_framesToWait--;
        if (m_framesToWait <= 0)
        {
            m_state = PRELOADER_STATE_PRELOAD_NEXT;
        }
    }
}

bool Preloader::IsFinished()
{
    return m_state == PRELOADER_STATE_FINISHED;
}

void Preloader::DrawUI()
{
    std::call_once(m_popupFlag, ImGui::OpenPopup, "Preloading Assets");
    if (ImGui::BeginPopupModal("Preloading Assets", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::Text("Please wait while all game assets are enumerated.");
        ImGui::Separator();
        ImGui::Text("Current pak: %s", m_mapsToPreload.size() > 0 ? m_mapsToPreload.back().c_str() : "");
        ImGui::ProgressBar(1.0f - (float)m_mapsToPreload.size() / (float)m_totalMaps);
        if (m_mapsToPreload.size() == 0)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
