#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h" // Using v2.07

class Main : public olc::PixelGameEngine
{
  public:
    Main()
    {
        sAppName = "DVD";
        std::srand(std::time(nullptr));
        std::int32_t const screen_w = WINDOW_W / PIXEL_WH;
        std::int32_t const screen_h = WINDOW_H / PIXEL_WH;
        if (!Construct(screen_w, screen_h, PIXEL_WH, PIXEL_WH)) {
            throw std::string("Failed to construct application");
        }
    }

  private:
    static constexpr std::int32_t WINDOW_W = 1080;
    static constexpr std::int32_t WINDOW_H = 720;
    static constexpr std::int32_t PIXEL_WH = 2;
    static constexpr float MAX_FPS = 60.0f;
    static constexpr float TIMESTEP = 1 / MAX_FPS;
    static constexpr char const* DVD_SPRITE_PATH = "dvd-logo-90x40.png";
    static constexpr std::size_t DVD_SPRITE_W = 90;
    static constexpr std::size_t DVD_SPRITE_H = 40;

    std::unique_ptr<olc::Sprite> dvd_sprite = nullptr;
    std::int32_t dvd_x = 0;
    std::int32_t dvd_y = 20;
    std::int32_t dvd_dx = 1;
    std::int32_t dvd_dy = 1;

    olc::Pixel RandColor()
    {
        static auto const RandRGB = []() -> std::uint8_t {
            return std::max<std::uint8_t>(std::rand() % UINT8_MAX, 32);
        };
        return olc::Pixel(RandRGB(), RandRGB(), RandRGB());
    }

    void UpdateColor(std::optional<olc::Pixel> color = std::nullopt)
    {
        if (!color.has_value()) {
            color = RandColor();
        }
        for (std::size_t x = 0; x < DVD_SPRITE_W; ++x) {
            for (std::size_t y = 0; y < DVD_SPRITE_H; ++y) {
                if (dvd_sprite->GetPixel(x, y).a != 0xFF) {
                    continue;
                }
                dvd_sprite->SetPixel(x, y, color.value());
            }
        }
    }

    bool OnUserCreate() override
    {
        dvd_sprite = std::make_unique<olc::Sprite>(DVD_SPRITE_PATH);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        static float time = 0.0f;
        time += fElapsedTime;
        if (time < TIMESTEP) {
            return true;
        }
        time -= TIMESTEP;

        if (GetKey(olc::Key::ESCAPE).bHeld) { // Quit.
            return false;
        }
        if (GetKey(olc::Key::Z).bHeld) { // White logo color.
            UpdateColor(olc::Pixel(UINT8_MAX, UINT8_MAX, UINT8_MAX));
        }
        if (GetKey(olc::Key::X).bHeld) { // Random logo color.
            UpdateColor(RandColor());
        }

        Clear(olc::BLACK);
        DrawSprite(dvd_x, dvd_y, dvd_sprite.get());

        dvd_x += dvd_dx;
        dvd_y += dvd_dy;

        if (dvd_x <= 0 || (dvd_x + dvd_sprite->width) >= ScreenWidth()) {
            dvd_dx *= -1;
            UpdateColor();
        }
        if (dvd_y <= 0 || (dvd_y + dvd_sprite->height) >= ScreenHeight()) {
            dvd_dy *= -1;
            UpdateColor();
        }

        return true;
    }
};

int
main()
{
    try {
        Main main;
        main.Start();
    }
    catch (std::string s) {
        std::cerr << s << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
