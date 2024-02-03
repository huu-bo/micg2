#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "main.h"
#include "stb_image.h"

static SDL_Texture* textures[10];
static const unsigned int t_width = SIZE / 10 * 5, t_height = SIZE / 10 * 10;

static unsigned char* resize_image(const unsigned char* in, size_t stride, unsigned int factor) {
	printf("resize_image stride = %lu\n", stride);

	if (in == NULL || factor == 0) {
		return NULL;
	}

	unsigned int width = 5 * factor;
	unsigned int height = 10 * factor;

	printf("\tsize: %ux%u\n", width, height);

	unsigned char* out = malloc(width*height*4);
	if (out == NULL) {
		return NULL;
	}

	for (unsigned int out_y = 0; out_y < height; out_y++) {
		for (unsigned int out_x = 0; out_x < width; out_x++) {
			// out[out_y*size + out_x] = in[out_y/factor*size + out_x/factor];
			// for (unsigned int i = 0; i < 4; i++) {
			//	out[(out_y*width + out_x)*4+i] = 0xFF;
			// }
			// out[out_y*width + out_x] = 0xFF;

			for (unsigned int i = 0; i < 4; i++) {
				// printf("\t\t%lu\n", (out_y/factor*stride + out_x/factor)*4+i);

				out[(out_y*width + out_x)*4+i] = in[(out_y/factor*stride + out_x/factor)*4+i];
			}
		}
	}

	return out;
}

int number__init(void) {
	int width, height, channels;
	unsigned char *data = stbi_load("mod/numbers.png", &width, &height, &channels, 4);
	if (data == NULL) {
		fprintf(stderr, "loading font image failed '%s'\n", stbi_failure_reason());
		return 1;
	}
	if (width != 50 || height != 10) {
		fprintf(stderr, "number font file not the right size (%dx%d)\n", width, height);
		return 1;
	}

	for (unsigned int i = 0; i < 10; i++) {
		unsigned int factor = SIZE / 10;

		unsigned char* image_scaled = resize_image(&data[5*4*i], 50, factor);

		// TODO: endiannes, see block.c
		SDL_Surface* image = SDL_CreateRGBSurfaceFrom(image_scaled, 5*factor, 10*factor, 8*4 /* RGBA */, 4*5*factor, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		if (image == NULL) {
			fprintf(stderr, "turning image to surface failed for number font:\n\t%s\n", SDL_GetError());
			return 1;
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(render, image);
		SDL_FreeSurface(image);
		free(image_scaled);
		if (texture == NULL) {
			fprintf(stderr, "turning surface to texture failed for number font:\n\t%s\n", SDL_GetError());
			return 1;
		}

		textures[(i + 1) % 10] = texture;
	}

	stbi_image_free(data);

	printf("number width: %u, height: %u\n", t_width, t_height);

	return 0;
}

void number__del(void) {
	for (unsigned int i = 0; i < 10; i++) {
		SDL_DestroyTexture(textures[i]);
	}
}

void number__render(SDL_Renderer* render, int x, int y, unsigned char num) {
	if (num > 99) {
		fprintf(stderr, "WARNING: number %u too large to render\n", num);
		return;
	}

	for (unsigned int i = 0; i < 2; i++) {
		unsigned int n;
		if (i == 0) {
			n = num / 10;
		} else {
			n = num % 10;
		}

		if (!(i == 0 && n == 0) /* leading zero */) {
			SDL_RenderCopy(render, textures[n], NULL, &(SDL_Rect){x + t_width * i, y, t_width, t_height});
		}
	}
}
