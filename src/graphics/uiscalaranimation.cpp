#include "uiscalaranimation.h"
#include "uitimer.h"
#ifdef _WIN32
# define ANIMATION_INTERVAL USER_TIMER_MINIMUM
#else
# define ANIMATION_INTERVAL 16
#endif


class UIScalarAnimationPrivate
{
public:
    UIScalarAnimationPrivate() :
        timer(new UITimer)
    {
        timer->onTimeout([this]() {
            handleAnimationStep();
        });
    }

    ~UIScalarAnimationPrivate()
    {
        delete timer; timer = nullptr;
    }

    void handleAnimationStep()
    {
        if (!isRunning) return;

        double progress = (double)(getCurrentTime() - startTime) / duration;
        progress = std::max<double>(0.0, std::min<double>(1.0, progress));
        currentValue = startValue + (targetValue - startValue) * progress;

        if (callback) {
            callback(currentValue);
        }

        if (progress >= 1.0) {
            if (loop) {
                restartAnimation();
            } else {
                isRunning = false;
                timer->stop();
                currentValue = targetValue;

                if (callback) {
                    callback(currentValue);
                }
            }
        }
    }

    void restartAnimation()
    {
        currentValue = startValue;
        startTime = getCurrentTime();
        if (callback) {
            callback(currentValue);
        }
    }

    long long getCurrentTime() const
    {
#ifdef _WIN32
        return GetTickCount64();
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
#endif
    }

    UITimer *timer = nullptr;
    double startValue = 0.0;
    double targetValue = 0.0;
    double currentValue = 0.0;
    int duration = 300;
    bool isRunning = false;
    bool loop = false;
    long long startTime = 0;
    std::function<void(double)> callback = nullptr;
};

UIScalarAnimation::UIScalarAnimation() :
    pimpl(new UIScalarAnimationPrivate)
{

}

UIScalarAnimation::~UIScalarAnimation()
{
    stopAnimation();
    delete pimpl; pimpl = nullptr;
}

void UIScalarAnimation::setCurrentValue(double value)
{
    pimpl->currentValue = value;
    pimpl->startValue = value;
}

void UIScalarAnimation::setTargetValue(double value)
{
    pimpl->targetValue = value;
}

void UIScalarAnimation::setDuration(int durationMs)
{
    pimpl->duration = std::max<int>(1, durationMs);
}

void UIScalarAnimation::setLoop(bool loop)
{
    pimpl->loop = loop;
}

void UIScalarAnimation::startAnimation()
{
    if (pimpl->isRunning) return;

    pimpl->startValue = pimpl->currentValue;
    pimpl->startTime = pimpl->getCurrentTime();
    pimpl->isRunning = true;
    pimpl->timer->start(ANIMATION_INTERVAL);
}

void UIScalarAnimation::stopAnimation()
{
    pimpl->isRunning = false;
    pimpl->timer->stop();
}

double UIScalarAnimation::currentValue() const
{
    return pimpl->currentValue;
}

bool UIScalarAnimation::isRunning() const
{
    return pimpl->isRunning;
}

void UIScalarAnimation::onValueChanged(const std::function<void(double)> &callback)
{
    pimpl->callback = callback;
}
