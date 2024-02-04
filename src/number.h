#ifndef NUMBER_H_
#define NUMBER_H_

int number__init(void);
void number__del(void);
void number__render(SDL_Renderer* render, int x, int y, unsigned char num);
void number__render_full(SDL_Renderer* render, int x, int y, unsigned int num, int pad);

#endif // NUMBER_H_
