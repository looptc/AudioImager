/*
 * Author: Abel Romero Pérez
 * Contact: Visit https://abelromero.com for inquiries.
 * Description: This code implements a WAV audio player and processing utility
 *              using the SDL2 library. It allows precise adjustments to the
 *              stereo representation of WAV audio files by manipulating mono
 *              and cancellation channels. This code is distributed under the
 *              MIT License as a gesture of gratitude towards the open-source
 *              community, which has enabled the development of this project
 *              through collaboration, effort, and available resources. The
 *              author hopes to keep this code open and freely accessible,
 *              and it is not intended to be privatized, even if considered
 *              for integration into Apple products.
 */

#include <SDL.h>

#define IN_CHA 2
#define OUT_CHA 2
#define LEFT_CHA 0
#define RIGHT_CHA 1

Uint8 *wav_buffer = NULL; // Buffer to store WAV file data
Uint32 wav_length = 0; // Length of the WAV file
Uint32 wav_sample = 0; // Current sample pointer in the WAV file
short audio_volume = 1; // Audio volume (currently unused)
int sample = 0; // Sample variable (currently unused)
int monos = 1; // Mono strength (mono channel adjustment)
int cancels = 1; // Cancellation strength (cancellation channel adjustment)

// Audio callback function
void AudioCallback(void* userdata, Uint8* stream, int len) {
    short* outbu = (short*) stream;
    short* inbu = (short*) wav_buffer;

    // Check if end of WAV buffer is reached
    if (wav_sample * sizeof(short) >= wav_length) {
        printf("end of buffer.\n");
        return;
    }

    // Process each audio sample in the output buffer
    for (int i = 0; i * sizeof(short) < len; i += IN_CHA) {
        // Get samples from left and right channels
        int A = inbu[wav_sample + LEFT_CHA];
        int B = inbu[wav_sample + RIGHT_CHA];
        // Calculate samples for sum and difference channels
        int C = A + B;
        int D = A - B;

        // Apply mono and cancellation strength adjustments
        int a = (monos * C + cancels * D);
        int b = (monos * C - cancels * D);

        // Normalize samples to prevent overflow
        if (monos < 0) monos *= -1;
        if (cancels < 0) cancels *= -1;
        int div = (monos > cancels) ? (2 * monos) : ((cancels > monos) ? (2 * cancels) : (monos + cancels));
        a /= div;
        b /= div;

        // Assign samples to output channels
        outbu[LEFT_CHA + i] = a;
        outbu[RIGHT_CHA + i] = b;

        // Increment sample pointer
        wav_sample += IN_CHA;
    }
}

int main(int argc, char* argv[]) {
    // Check if correct number of command-line arguments are provided
    if (argc != 4) {
        fprintf(stderr, "use: %s <WAV file> <mono_strength> <cancelation_strength>\n", argv[0]);
        return -1;
    }

    char *inf = argv[1];
    monos = atoi(argv[2]);
    cancels = atoi(argv[3]);

    // Initialize SDL for audio
    if (SDL_Init(SDL_INIT_AUDIO)) {
        return 1;
    }

    // Create audio specification structure
    SDL_AudioSpec wav_spec;

    /* Load the WAV */
    if (SDL_LoadWAV(inf, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        fprintf(stderr, "Could not open %s: %s\n", inf, SDL_GetError());
        return -2;
    }

    // Configure audio callback function
    wav_spec.callback = AudioCallback;

    // Open audio device
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (!dev) {
        SDL_Quit();
        return 1;
    }

    // Play audio
    SDL_PauseAudioDevice(dev, 0);

    // Wait for quit event
    SDL_Event event;
    int quit = 0;
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                quit = 1;
            }
        }

        // Wait a brief period to prevent the loop from consuming all CPU
        SDL_Delay(10);
    }

    // Free memory and close audio device
    SDL_FreeWAV(wav_buffer);
    SDL_CloseAudioDevice(dev);
    SDL_Quit();

    return 0;
}
