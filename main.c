#include <stdio.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

static const char *name = "SDL Template";
static const char *version = "1";
static const char *appid = "io.damians-eng.demo";

static SDL_AudioDeviceID audio_device = 0;
static SDL_AudioStream *stream = NULL;

#define SDL_WINDOW_WIDTH 800
#define SDL_WINDOW_HEIGHT 600

const double target_fps = 60.0;
const Uint64 target_fpns = (Uint64)(1e9 / target_fps);

typedef struct
{
        SDL_Window *window;
        SDL_Renderer *renderer;
        Uint64 last_time;
} AppState;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {

  if(!SDL_Init(SDL_INIT_AUDIO) || !SDL_Init(SDL_INIT_VIDEO) || !SDL_Init(SDL_INIT_EVENTS)) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); 
    return SDL_APP_FAILURE;
  }

  if(!SDL_SetAppMetadata(name, version, appid)) {
    fprintf(stderr, "Some problem setting metadata: %s\n", SDL_GetError()); 
    return SDL_APP_FAILURE;
  }

  AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
  if (!as) {
    SDL_Log("something wrong with appstate: %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  *appstate = as;

  SDL_AudioSpec wavespec;
  Uint8 *wavebuff = NULL;
  Uint32 wavelen = 0;

  SDL_AudioSpec desired;
  SDL_zero(desired);
  desired.freq = 48000;             // Hz
  desired.format = SDL_AUDIO_F32;       // floating point
  desired.channels = 2;             // stereo
  
  audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
  if(!audio_device) {
    fprintf(stderr, "Audio device error: %s\n", SDL_GetError()); 
    SDL_free(wavebuff); 
    return SDL_APP_FAILURE; 
  }

  stream = SDL_CreateAudioStream(&wavespec, &desired);
  if(!stream) {
    fprintf(stderr, "Stream Problem. %s\n", SDL_GetError());
    SDL_free(wavebuff);
    return SDL_APP_FAILURE;
  }

  if (SDL_PutAudioStreamData(stream, wavebuff, wavelen) < 0) { 
    fprintf(stderr, "Stream put error: %s\n", SDL_GetError()); 
    SDL_free(wavebuff); 
    return SDL_APP_FAILURE; 
  }

  SDL_free(wavebuff);

  if (SDL_BindAudioStream(audio_device, stream) < 0) { 
    fprintf(stderr, "Failed to bind stream to device: %s\n", SDL_GetError()); 
    return SDL_APP_FAILURE; 
  }

  if (!SDL_CreateWindowAndRenderer(
        "Demo", SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &as->window, &as->renderer)) {
        fprintf(stderr, "Cannot create window or renderer./.  %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
  }

  SDL_ResumeAudioDevice(audio_device);

  as->last_time = SDL_GetTicksNS();

  return SDL_APP_CONTINUE;
};

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *as = (AppState *)appstate;

  const Uint64 now = SDL_GetTicksNS();
  const Uint64 elapsed = now - as->last_time;
  if (elapsed >= target_fpns) {
        as->last_time = now;
        SDL_SetRenderDrawColor(as->renderer, 30, 30, 30, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(as->renderer);
        
        SDL_SetRenderDrawColor(as->renderer, 30, 100, 200, SDL_ALPHA_OPAQUE);
	const int rect_width = 100;
	const int rect_height = 200;
        SDL_FRect rect = { 
		// Make the rectangle start in the middle of the window
		((SDL_WINDOW_WIDTH / 2) - (rect_width / 2)), 
		((SDL_WINDOW_HEIGHT / 2) - (rect_height / 2)),
		rect_width, 
		rect_height
		// this starts out fine, but this doesn't work if the window is resized later
	};
        SDL_RenderFillRect(as->renderer, &rect);
        SDL_RenderPresent(as->renderer);
  } else {
        const Uint64 remaining = target_fpns - elapsed;
        SDL_DelayNS(remaining);
  }
  return SDL_APP_CONTINUE;
};

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->common.type == SDL_EVENT_QUIT) {
      return SDL_APP_SUCCESS;
    }
  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
    {
      return SDL_APP_SUCCESS;
    }
  if ((event->type == SDL_EVENT_KEY_DOWN) && (event->key.key == SDLK_ESCAPE))
    {
      return SDL_APP_SUCCESS;
    }
  return SDL_APP_CONTINUE;
};

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (audio_device) {
    SDL_CloseAudioDevice(audio_device);
    audio_device = 0;
  }
  if (stream) {
    SDL_DestroyAudioStream(stream);
    stream = NULL;
  }
  if (appstate != NULL) {
    AppState *as = (AppState *)appstate;
    if (as->renderer) {
      SDL_DestroyRenderer(as->renderer);
    }
    if (as->window) {
      SDL_DestroyWindow(as->window);
    }
    //SDL_DestroyGPUDevice(device);
  }
};
