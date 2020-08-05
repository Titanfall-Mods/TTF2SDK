#pragma once

enum PreloaderState
{
    PRELOADER_STATE_PRELOAD_NEXT,
    PRELOADER_STATE_WAIT,
    PRELOADER_STATE_PRELOADING,
    PRELOADER_STATE_FINISHED
};

class Preloader : public IFrameTask
{
public:
    Preloader(const std::vector<std::string>& maps);
    virtual ~Preloader();
    virtual void RunFrame();
    virtual bool IsFinished();
    void DrawUI();

private:
    PreloaderState m_state;
    std::vector<std::string> m_mapsToPreload;
    size_t m_totalMaps;
    int m_framesToWait;
    std::once_flag m_popupFlag;
};
