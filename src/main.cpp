
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(PracticePlusPauseLayer, PauseLayer) {
    bool init(bool unk) {
        if (!PauseLayer::init(unk))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Practice+ button
        auto buttonSprite = CCSprite::createWithSpriteFrameName(
            "GJ_button_01.png"
        );

        if (!buttonSprite)
            buttonSprite = CCSprite::create("GJ_button_01.png");

        if (!buttonSprite)
            return true;

        auto label = CCLabelBMFont::create(
            "Practice+", "bigFont.fnt"
        );
        label->setScale(0.45f);

        auto button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(PracticePlusPauseLayer::onPracticePlus)
        );

        button->addChild(label);
        label->setPosition(buttonSprite->getContentSize() / 2);

        auto menu = CCMenu::create();
        menu->addChild(button);
        menu->setPosition({winSize.width / 2, winSize.height / 2});

        this->addChild(menu, 100);

        return true;
    }

    void onPracticePlus(CCObject*) {
        Notification::create(
            "Practice+ opened!",
            NotificationIcon::Success,
            2.f
        )->show();
    }
};
