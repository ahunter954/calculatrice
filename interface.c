#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// --- Structure des Boutons ---
typedef struct {
    int x, y, w, h;
    char label[50];
    char tooltip[100]; // Info-bulle associée
} Button;

// --- Vérification de la position de la souris ---
int isMouseOver(Button *btn, int mouseX, int mouseY) {
    return mouseX >= btn->x && mouseX <= btn->x + btn->w &&
           mouseY >= btn->y && mouseY <= btn->y + btn->h;
}

int isMouseOverRect(SDL_Rect *rect, int mouseX, int mouseY) {
    return mouseX >= rect->x && mouseX <= rect->x + rect->w &&
           mouseY >= rect->y && mouseY <= rect->y + rect->h;
}

// --- Rendu d'un caractère ---
void renderChar(SDL_Renderer *renderer, SDL_Texture *fontTexture, char c, int x, int y, int charW, int charH) {
    int charIndex = -1;

    if (c >= 'A' && c <= 'Z') {
        charIndex = c - 'A'; // Indices pour A-Z
    } else if (c >= 'a' && c <= 'z') {
        charIndex = 26 + (c - 'a'); // Indices pour a-z
    } else if (c >= '0' && c <= '9') {
        charIndex = 52 + (c - '0'); // Indices pour 0-9
    } else if (c == '/') {
        charIndex = 62; // Indice pour /
    } else if (c == '.') {
        charIndex = 63; // Indice pour .
    }

    if (charIndex == -1) return; // Caractère non supporté

    int cols = 16; // Nombre de colonnes dans la bitmap
    SDL_Rect srcRect = {
        (charIndex % cols) * charW,       // Colonne
        (charIndex / cols) * charH,       // Ligne
        charW, charH                      // Taille d'un caractère
    };

    SDL_Rect destRect = {x, y, charW, charH};
    SDL_RenderCopy(renderer, fontTexture, &srcRect, &destRect);
}

// --- Rendu d'une chaîne de caractères ---
void renderText(SDL_Renderer *renderer, SDL_Texture *fontTexture, const char *text, int x, int y, int charW, int charH) {
    int offsetX = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        renderChar(renderer, fontTexture, text[i], x + offsetX, y, charW, charH);
        offsetX += charW;
    }
}

// --- Rendu d'un bouton ---
void renderButton(SDL_Renderer *renderer, SDL_Texture *fontTexture, Button *btn, int highlighted, int charW, int charH) {
    SDL_SetRenderDrawColor(renderer, highlighted ? 0 : 100, highlighted ? 200 : 100, 100, 255);
    SDL_Rect rect = {btn->x, btn->y, btn->w, btn->h};
    SDL_RenderFillRect(renderer, &rect);

    int textX = btn->x + (btn->w - strlen(btn->label) * charW) / 2;
    int textY = btn->y + (btn->h - charH) / 2;
    renderText(renderer, fontTexture, btn->label, textX, textY, charW, charH);
}

// --- Rendu d'une info-bulle ---
void renderTooltip(SDL_Renderer *renderer, SDL_Texture *fontTexture, const char *tooltip, int x, int y, int charW, int charH) {
    int tooltipWidth = strlen(tooltip) * charW + 10;
    int tooltipHeight = charH + 10;

    // Dessiner le fond de l'info-bulle
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour le fond
    SDL_Rect tooltipRect = {x, y, tooltipWidth, tooltipHeight};
    SDL_RenderFillRect(renderer, &tooltipRect);

    // Dessiner le bord de l'info-bulle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanc pour le bord
    SDL_RenderDrawRect(renderer, &tooltipRect);

    // Afficher le texte de l'info-bulle
    renderText(renderer, fontTexture, tooltip, x + 5, y + 5, charW, charH);
}

// --- Rendu de la barre de texte ---
void renderTextBox(SDL_Renderer *renderer, SDL_Texture *fontTexture, SDL_Rect *textbox, const char *text, int focused, int charW, int charH, int blink) {
    SDL_SetRenderDrawColor(renderer, focused ? 200 : 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, textbox);

    renderText(renderer, fontTexture, text, textbox->x + 5, textbox->y + (textbox->h - charH) / 2, charW, charH);

    if (focused && blink) {
        int textWidth = strlen(text) * charW;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect cursor = {textbox->x + 5 + textWidth, textbox->y + (textbox->h - charH) / 2, 2, charH};
        SDL_RenderFillRect(renderer, &cursor);
    }
}

// --- Main ---
int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Interface graphique", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

    SDL_Surface *fontSurface = IMG_Load("font_black_letters.png");
    if (!fontSurface) {
        printf("Erreur chargement font_black_letters.png : %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    SDL_FreeSurface(fontSurface);

    int charW = 16, charH = 16;

    Button buttons[] = {
        {50, 50, 200, 50, "entrainement", "Pas de parametres"},
        {300, 50, 200, 50, "resoudre", "Pas de parametres"},
        {550, 50, 200, 50, "decoupe", "Nom image"},
        {50, 150, 200, 50, "noir_blanc", "Nom image"},
        {300, 150, 200, 50, "rotation", "Nom image"},
        {550, 150, 200, 50, "bruit", "Nom image"},
        {300, 250, 200, 50, "contrast", "Nom image"}
    };
    int numButtons = sizeof(buttons) / sizeof(Button);

    SDL_Rect textBox = {50, 400, 700, 50};
    char parameters[256] = "";
    int textBoxFocused = 0;
    int blink = 1;
    Uint32 lastBlinkTime = SDL_GetTicks();

    SDL_StartTextInput();

    int running = 1;
    SDL_Event e;

    while (running) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        const char *currentTooltip = NULL;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                for (int i = 0; i < numButtons; i++) {
                    if (isMouseOver(&buttons[i], mouseX, mouseY)) {
                        char command[512];
                        if (strcmp(buttons[i].label, "entrainement") == 0) 
                        {
                            snprintf(command, sizeof(command), "./train");
                        } 
                        else if (strcmp(buttons[i].label, "resoudre") == 0) 
                        {
                            int result = system("clear");
                            if (result != 0) {
                                printf("Erreur lors de l'exécution de %s\n", buttons[i].label);
                            }
                            snprintf(command, sizeof(command), "./final_solver");
                        }
                        else if (strcmp(buttons[i].label, "decoupe") == 0) 
                        {
                            snprintf(command, sizeof(command), "./Image_segmenter %s",  parameters);
                            printf("Exécution : %s\n", command);
                            int result = system(command);
                            if (result != 0) {
                                printf("Erreur lors de l'exécution de %s\n", buttons[i].label);
                            }
                            snprintf(command, sizeof(command), "./decoupage %s",  parameters);
                        }
                        else if (strcmp(buttons[i].label, "noir_blanc") == 0) 
                        {
                            snprintf(command, sizeof(command), "./Color_deletion %s",  parameters);
                        }
                        else if (strcmp(buttons[i].label, "rotation") == 0) 
                        {
                            snprintf(command, sizeof(command), "./pas_encore_implementer %s",  parameters);
                        }
                        else if (strcmp(buttons[i].label, "bruit") == 0) 
                        {
                            snprintf(command, sizeof(command), "./Noises %s",  parameters);
                        }
                        else
                        {
                          snprintf(command, sizeof(command), "./Increase_c %s",  parameters);
                        }

                        printf("Exécution : %s\n", command);
                        int result = system(command);
                        if (result != 0) {
                            printf("Erreur lors de l'exécution de %s\n", buttons[i].label);
                        }
                    }
                }

                if (isMouseOverRect(&textBox, mouseX, mouseY)) {
                    textBoxFocused = 1;
                } else {
                    textBoxFocused = 0;
                }
            } else if (e.type == SDL_TEXTINPUT && textBoxFocused) {
                if (strlen(parameters) + strlen(e.text.text) < sizeof(parameters) - 1) {
                    strcat(parameters, e.text.text);
                }
            } else if (e.type == SDL_KEYDOWN && textBoxFocused) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(parameters) > 0) {
                    parameters[strlen(parameters) - 1] = '\0';
                }
            }
        }

        for (int i = 0; i < numButtons; i++) {
            if (isMouseOver(&buttons[i], mouseX, mouseY)) {
                currentTooltip = buttons[i].tooltip;
                break;
            }
        }

        if (SDL_GetTicks() - lastBlinkTime >= 500) {
            blink = !blink;
            lastBlinkTime = SDL_GetTicks();
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < numButtons; i++) {
            int highlighted = isMouseOver(&buttons[i], mouseX, mouseY);
            renderButton(renderer, fontTexture, &buttons[i], highlighted, charW, charH);
        }

        if (currentTooltip != NULL) {
            renderTooltip(renderer, fontTexture, currentTooltip, mouseX + 10, mouseY + 10, charW, charH);
        }

        renderTextBox(renderer, fontTexture, &textBox, parameters, textBoxFocused, charW, charH, blink);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();

    SDL_DestroyTexture(fontTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}