#include "simulator/algo_debugger.h"

using namespace world;

bool isMenu = true;

sf::Font font;
sf::Text createText(std::string value) {
  sf::Text text;
  text.setFont(font);
  text.setString(value);
  text.setCharacterSize(24);
  text.setFillColor(sf::Color::White);
  // text.setStyle(sf::Text::Bold);
  // text.setPosition({GRID_CENTER, GRID_CENTER});
  return text;
}

void centerTextAt(sf::Text& text, const sf::Vector2f& position) {
  auto bounds = text.getLocalBounds();
  text.setPosition(position - sf::Vector2f{bounds.width / 2.f, bounds.height / 2.f});
}

int main() {
  AddSegfaultHandler();
  spdlog::set_level(spdlog::level::trace);

  auto desktopMode = sf::VideoMode::getDesktopMode();
  int screenWidth = desktopMode.width / 2;
  int screenHeight = desktopMode.height / 2;
  spdlog::debug("Screen: {}x{}", screenWidth, screenHeight);
  sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "SFML window");
  auto aspectRatio = 1.0 * desktopMode.width / desktopMode.height;
  sf::View view(sf::Vector2f(GRID_CENTER, GRID_CENTER), sf::Vector2f(GRID_SIZE * aspectRatio, GRID_SIZE));
  window.setView(view);

  InputManager inputManager;
  FrameData frameData;
  SteinerState steinerState(100);
  RenderState renderState;

  auto fontPath = "assets/times-new-roman.ttf";
  if (!font.loadFromFile(fontPath)) {
    spdlog::trace("Failed to load {}", fontPath);
    throw "Failed to load font";
  }

  sf::Text text1 = createText("Steiner tree solver v0");
  centerTextAt(text1, {GRID_CENTER, GRID_CENTER});
  std::vector<sf::Text> texts = {text1};
  std::vector<std::function<void()>> scenes = {[&]() {
    auto stateChange = steinerState.Simulate(frameData, window, inputManager);
    renderState.Render(window, frameData, stateChange);
  }};

  while (window.isOpen()) {
    if (inputManager.IsPress(sf::Keyboard::Escape)) {
      if (!isMenu) {
        isMenu = true;
      } else {
        window.close();
      }
    }

    sf::Event event;
    while (window.pollEvent(event)) {
      // Close window: exit
      if (event.type == sf::Event::Closed)
        window.close();
      else if (event.type == sf::Event::Resized) {
        screenWidth = event.size.width;
        screenHeight = event.size.height;
        aspectRatio = 1.0 * event.size.width / event.size.height;
        spdlog::debug("Resize: {}x{} (aspect ratio {:.3f})", screenWidth, screenHeight, aspectRatio);
        sf::View view(sf::Vector2f(GRID_CENTER, GRID_CENTER), sf::Vector2f(GRID_SIZE * aspectRatio, GRID_SIZE));
        window.setView(view);
      }
    }

    if (inputManager.IsPress(sf::Keyboard::R)) {
      steinerState = SteinerState(100);
      renderState = RenderState();
    }
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && isMenu) {
      isMenu = false;
    }

    window.clear();
    if (isMenu) {
      for (auto text : texts) {
        window.draw(text);
      }
    } else {
      scenes[0]();
      frameData.NextFrame();
    }
    window.display();

    inputManager.NextFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
