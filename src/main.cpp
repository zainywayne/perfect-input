#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

static constexpr float BUFFER_WINDOW = 0.25f;
static constexpr float COYOTE_WINDOW = 0.10f;

class $modify(PerfectInputLayer, GJBaseGameLayer) {
    struct Fields {
        float time = 0.f;
        float pressTime = -100.f;
        float groundTime = -100.f;
        bool pending = false;
        bool held = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();

        if (button != 1 || !isPlayer1) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        f->held = down;

        if (!down) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        f->pressTime = f->time;
        f->pending = true;

        bool wasGrounded = m_player1->m_isOnGround;
        if (!wasGrounded && (f->time - f->groundTime) <= COYOTE_WINDOW) {
            m_player1->m_isOnGround = true;
            GJBaseGameLayer::handleButton(true, button, isPlayer1);
            m_player1->m_isOnGround = wasGrounded;
            f->pending = false;
            return;
        }

        GJBaseGameLayer::handleButton(true, button, isPlayer1);
        if (wasGrounded) f->pending = false;
    }

    void update(float dt) {
        GJBaseGameLayer::update(dt);

        auto f = m_fields.self();
        f->time += dt;

        if (!m_player1) return;

        if (m_player1->m_isOnGround) f->groundTime = f->time;

        if (!f->pending) return;

        if (f->time - f->pressTime > BUFFER_WINDOW) {
            f->pending = false;
            return;
        }

        bool canAct = m_player1->m_isOnGround || m_player1->m_touchedRings.size() > 0;
        if (canAct) {
            GJBaseGameLayer::handleButton(true, 1, true);
            if (!f->held) GJBaseGameLayer::handleButton(false, 1, true);
            f->pending = false;
        }
    }
};
