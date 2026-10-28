
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PracticePlusPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* statsLabel = nullptr;
        int deaths = 0;
        int bestProgress = 0;
        int nextAutoCheckpoint = 0;
        bool practiceRun = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        m_fields->practiceRun = m_isPracticeMode;
        m_fields->deaths = 0;
        m_fields->bestProgress = 0;

        if (!m_fields->practiceRun)
            return true;

        auto menu = CCMenu::create();
        if (!menu)
            return true;

        menu->setID("practice-plus-controls"_spr);
        menu->setPosition(CCPointZero);

        auto addButton = [&](const char* text, float x,
                             SEL_MenuHandler callback) {
            auto label = CCLabelBMFont::create(text, "bigFont.fnt");
            if (!label)
                return;

            label->setScale(0.38f);

            auto button = CCMenuItemLabel::create(
                label, this, callback
            );
            if (!button)
                return;

            button->setPosition({x, 0.f});
            menu->addChild(button);
        };

        auto size = CCDirector::sharedDirector()->getWinSize();

        addButton("CP-", size.width - 126.f,
                  menu_selector(PracticePlusPlayLayer::onPreviousCP));
        addButton("SAVE", size.width - 83.f,
                  menu_selector(PracticePlusPlayLayer::onSaveCP));
        addButton("CP+", size.width - 40.f,
                  menu_selector(PracticePlusPlayLayer::onNextCP));

        if (Mod::get()->getSettingValue<bool>("quick-restart")) {
            addButton("RESTART", size.width - 90.f,
                      menu_selector(PracticePlusPlayLayer::onQuickRestart));
        }

        this->addChild(menu, 1000);

        if (Mod::get()->getSettingValue<bool>("show-statistics")) {
            m_fields->statsLabel = CCLabelBMFont::create(
                "Deaths: 0 | Best: 0%", "bigFont.fnt"
            );

            if (m_fields->statsLabel) {
                m_fields->statsLabel->setScale(0.35f);
                m_fields->statsLabel->setAnchorPoint({0.f, 1.f});
                m_fields->statsLabel->setPosition({8.f, size.height - 8.f});
                this->addChild(m_fields->statsLabel, 1000);
            }
        }

        m_fields->nextAutoCheckpoint =
            Mod::get()->getSettingValue<int>("checkpoint-interval");

        return true;
    }

    void onSaveCP(CCObject*) {
        if (!m_isPracticeMode)
            return;

        auto checkpoint = createCheckpoint();
        if (checkpoint) {
            Notification::create(
                "Practice+ checkpoint saved",
                NotificationIcon::Success, 1.5f
            )->show();
        }
    }

    void onPreviousCP(CCObject*) {
        switchCP(-1);
    }

    void onNextCP(CCObject*) {
        switchCP(1);
    }

    void switchCP(int direction) {
        if (!m_isPracticeMode || !m_checkpointArray ||
            m_checkpointArray->count() == 0) {
            Notification::create(
                "No checkpoints saved",
                NotificationIcon::Info, 1.5f
            )->show();
            return;
        }

        int count = static_cast<int>(m_checkpointArray->count());
        int current = -1;

        for (int i = 0; i < count; ++i) {
            auto checkpoint = static_cast<CheckpointObject*>(
                m_checkpointArray->objectAtIndex(i)
            );

            if (checkpoint == m_currentCheckpoint) {
                current = i;
                break;
            }
        }

        int target = current == -1
            ? (direction > 0 ? 0 : count - 1)
            : current + direction;

        if (target < 0 || target >= count) {
            Notification::create(
                "No checkpoint in that direction",
                NotificationIcon::Info, 1.5f
            )->show();
            return;
        }

        auto checkpoint = static_cast<CheckpointObject*>(
            m_checkpointArray->objectAtIndex(target)
        );

        if (checkpoint)
            loadFromCheckpoint(checkpoint);
    }

    void onQuickRestart(CCObject*) {
        if (!m_isPracticeMode)
            return;

        resetLevel();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (m_fields->practiceRun)
            ++m_fields->deaths;

        PlayLayer::destroyPlayer(player, object);
        updatePracticeStats();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_fields->practiceRun || !m_isPracticeMode)
            return;

        int progress = static_cast<int>(getCurrentPercent());
        if (progress > m_fields->bestProgress)
            m_fields->bestProgress = progress;

        if (Mod::get()->getSettingValue<bool>("automatic-checkpoints")) {
            int interval = Mod::get()->getSettingValue<int>(
                "checkpoint-interval"
            );

            interval = std::max(1, interval);
            if (progress >= m_fields->nextAutoCheckpoint &&
                progress < 100) {
                if (createCheckpoint()) {
                    m_fields->nextAutoCheckpoint =
                        (progress / interval + 1) * interval;
                }
            }
        }

        updatePracticeStats();
    }

    void updatePracticeStats() {
        if (!m_fields->statsLabel)
            return;

        auto text = fmt::format(
            "Deaths: {} | Best: {}%",
            m_fields->deaths,
            m_fields->bestProgress
        );

        m_fields->statsLabel->setString(text.c_str());
    }
};
