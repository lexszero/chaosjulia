/*
 * ChaosJulia by LexsZero
 *
 * for CC2012 Realtime Coding
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include <complex.h> 

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

enum {
	NEED_NOTHING,
	NEED_DRAW,
	NEED_PROCESS
} need;

uint16_t *raw;

struct pixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct pixel *tex_ptr, *palette;

int res_x = 640, res_y = 480;
int n_colors = 16;

double complex c, d, c1, c2;
float zoom = 3.5, step = 4;
int delay = 40, period = 40;
int n_iter = 32;

void draw() {
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,
			res_x,res_y,0,
			GL_RGB,
			GL_UNSIGNED_BYTE, tex_ptr);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0); glVertex2f(0.0,0.0);
		glTexCoord2f(1.0,0.0); glVertex2f(res_x,0.0);
		glTexCoord2f(1.0,1.0); glVertex2f(res_x,res_y);
		glTexCoord2f(0.0,1.0); glVertex2f(0.0,res_y);
	glEnd();
	SDL_GL_SwapBuffers();

	need = NEED_NOTHING;
}

uint8_t init_opengl(int res_x, int res_y)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != -1)	{
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		
		if(SDL_SetVideoMode(res_x, res_y, 32, SDL_ANYFORMAT|SDL_OPENGL))
		{
			glEnable(GL_TEXTURE_2D);
			glViewport(0,0,res_x,res_y);
			gluOrtho2D(0,res_x,0,res_y);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			return 1;
		}
	}
	return 0;
}

void fill_palette() {
	int i, ii;
	float hue, fr;
	unsigned char c1, c2, c3;
	struct pixel *p;
	for (i = 1; i < n_colors; i++) {
		p = palette + i;
		hue = (float)(n_colors-i)/n_colors * 360;
		ii = (int)(hue /= 60.0);
		fr = hue - ii;
		c1 = 0;
		c2 = 255 * (1.0-fr);
		c3 = 255 - 255 * (1.0 - fr);
		switch (ii) {
			case 0: p->r = 255; p->g = c3; p->b = c1; break;
			case 1: p->r = c2; p->g = 255; p->b = c1; break;
			case 2: p->r = c1; p->g = 255; p->b = c3; break;
			case 3: p->r = c1; p->g = c2; p->b = 255; break;
			case 4: p->r = c3; p->g = c1; p->b = 255; break;
			case 5: p->r = 255; p->g = c1; p->b = c2; break;
		}
	}
}

void process() {
	double complex z;
	int i, j, k;
	struct pixel *p;
	for (i = -res_x/2; i < res_x/2; i++) {
		for (j = -res_y/2; j < res_y/2; j++) {
			z = (float)i/res_x*zoom + (float)j/res_y*zoom*I;
			z += d;
			p = tex_ptr + (i+res_x/2) + res_x*(j+res_y/2);
			for (k = 0; k < n_iter; k++) {
				z = z*z + c;
				if (cabs(z) > 2) break;
			}
			if (k == n_iter) {
				p->r = 0;
				p->g = 0;
				p->b = 0;
			}
			else {
				memcpy(p, palette + (int)((float)k/n_iter*n_colors), sizeof(struct pixel));
			}
		}
		
	}
	need = NEED_DRAW;
}

uint32_t timer_func(uint32_t interval, void *param) {
	static unsigned counter = 0;
	if (counter++ == period) {
		c1 = c2;
		do {
			c2 = ((float)rand()/RAND_MAX - 0.5)*step + ((float)rand()/RAND_MAX - 0.5) * step* I;
		} while (cabs(c2 - c1) < 3);
		//printf("c = %f + %fi\n", crealf(c2), cimagf(c2));
		counter = 0;
	}
	if (need == NEED_NOTHING) {
		c = c1+(c2-c1)*((float)counter/period);
		need = NEED_PROCESS;
	}
	
	return interval;
}

int main(int argc, char* argv[])
{

	tex_ptr = calloc(res_x*res_y, sizeof(struct pixel));

	if(!init_opengl(res_x,res_y))
	{
		puts("OpenGL init failed.");
		return -4;
	}

	palette = calloc(n_colors, sizeof(struct pixel));
	fill_palette();
	c1 = -0.74543 + 0.11301*I;
	c2 = ((float)rand()/RAND_MAX - 0.5)*step + (1 + (float)rand()/RAND_MAX - 0.5) * step * I;

	SDL_AddTimer(delay, timer_func, NULL);
	SDL_Event ev;
	while(1) {
		if(SDL_PollEvent(&ev) != 0)	{
			if (ev.type == SDL_QUIT) {
				puts("SDL_QUIT received. Shutting down.");
				break;
			}
		}
		switch (need) {
			case NEED_PROCESS:
				process();
				draw();
				break;
		}
		SDL_Delay(1);
	}

	free(tex_ptr);
	free(palette);
}


