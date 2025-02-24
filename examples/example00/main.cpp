namespace {

const char* WINDOW_TITLE = "Example 00";
const int WINDOW_WIDTH = 16 * 60;
const int WINDOW_HEIGHT = 9 * 60;

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
#if defined(_DEBUG)
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif

  // SDLを初期化する
  SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Initialize SDL.");
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "Failed to initialize SDL: %s",
                    SDL_GetError());
    return EXIT_FAILURE;
  }

  // ウィンドウを生成する
  SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Create window: %s (%d x %d)",
              WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
  auto* window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                                  WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s",
                    SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // レンダラーを生成する
  SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Create renderer.");
  auto* renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Failed to create renderer: %s",
                    SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  bool running = true;
  while (running) {
    {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        switch (e.type) {
          case SDL_QUIT:
            running = false;
            break;
          case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_CLOSE &&
                e.window.windowID == SDL_GetWindowID(window)) {
              running = false;
            }
            break;
          default:
            break;
        }
      }
    }

    // レンダラーの出力サイズを取得する
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    // 画面をクリアする
    SDL_SetRenderDrawColor(renderer, 0x2b, 0x2b, 0x2b, 0xff);
    SDL_RenderClear(renderer);

    // 矩形を描画する
    {
      const SDL_Rect rect = {width / 4, height / 4, width / 2, height / 2};
      SDL_SetRenderDrawColor(renderer, 0xfb, 0xfa, 0xf5, 0xff);
      SDL_RenderFillRect(renderer, &rect);
    }

    // レンダラーを更新する
    SDL_RenderPresent(renderer);
  }

  // レンダラーを破棄する
  SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Destroy renderer.");
  SDL_DestroyRenderer(renderer);

  // ウィンドウを破棄する
  SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Destroy window.");
  SDL_DestroyWindow(window);

  // SDLを終了する
  SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Shutdown SDL.");
  SDL_Quit();

  return EXIT_SUCCESS;
}
