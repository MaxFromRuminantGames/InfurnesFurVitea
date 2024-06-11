#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define PI 3.1415

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define abs(x) (x<0)?-x:x

#define swap(x,y) { x = x + y; y = x - y; x = x - y; }

#define WIDTH 800
#define HEIGHT 600

typedef struct vector3      { int x, y, z; }    vec3;
typedef struct vector3Float { double x, y, z; } vec3F;
typedef struct vector2      { int x, y; }       vec2;
typedef struct vector2Float { double x, y; }    vec2F;

typedef struct color { Uint8 r, g, b; } color;

typedef struct vert3D { color viewcolor; vec3 pos; } vert3D;
typedef struct vert2D { color viewcolor; vec2 pos; } vert2D;

typedef struct triangle3D { vert3D vert1, vert2, vert3; vec2F uv1, uv2, uv3; } triangle3D;

typedef struct transform { vec3 position; vec3 rotation; } transform;

typedef struct object
{
    transform root;

    struct mesh { triangle3D *tris; SDL_Texture *texture; } mesh;
    struct objData { int faceCount; int vertCount; int uvCount; } data;
} object;

typedef struct camera
{
    transform root;

    struct screen
    { int width, height; } screen;

    int fov;//in degrees
    int focalLength;
    int nearPlane, farPlane;
} camera;

void drawTri(SDL_Renderer *renderer, int p1x, int p1y, int p1z,
                                     int p2x, int p2y, int p2z,
                                     int p3x, int p3y, int p3z)
{
    int minY1 = min(p1y, p2y); int minY2 = min(p2y, p3y); int minY3 = min(p3y, p1y);
    int maxY1 = max(p1y, p2y); int maxY2 = max(p2y, p3y); int maxY3 = max(p3y, p1y);

    int triMax = max(p1y, max(p2y, p3y));
    int triMin = min(p1y, min(p2y, p3y));

    int minX1, maxX1, minX2, maxX2, minX3, maxX3;
    if(minY1 == p1y) { minX1 = p1x; maxX1 = p2x; } else { minX1 = p2x; maxX1 = p1x; }
    if(minY2 == p2y) { minX2 = p2x; maxX2 = p3x; } else { minX2 = p3x; maxX2 = p2x; }
    if(minY3 == p3y) { minX3 = p3x; maxX3 = p1x; } else { minX3 = p1x; maxX3 = p3x; }

    double slope1 = (maxY1 - minY1) / (maxX1 - minX1);
    double slope2 = (maxY2 - minY2) / (maxX2 - minX2);
    double slope3 = (maxY3 - minY3) / (maxX3 - minX3);
    int intercept1 = -slope1*minX1 + minY1;
    int intercept2 = -slope2*minX2 + minY2;
    int intercept3 = -slope3*minX3 + minY3;

    for(int scanY = triMin; scanY < triMax; ++scanY)
    {int outX; //y=mx+b //x=(-y+b)/m //b=-mx+y
        if(scanY < maxY1 && scanY > minY1) 
        { 
            outX = (-scanY + intercept1)/slope1;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawPoint(renderer, -outX, scanY);
        }
        if(scanY < maxY2 && scanY > minY2) 
        { 
            outX = (-scanY + intercept2)/slope2;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawPoint(renderer, -outX, scanY);
        }
        if(scanY < maxY3 && scanY > minY3) 
        { 
            outX = (-scanY + intercept3)/slope3;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawPoint(renderer, -outX, scanY);
        }
    }
}

int main()
{
    //todo, make the screen draw triangles, give the triangles color, make the triangles 3d, make cube from tris, obj importer
    SDL_Window  *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *test = NULL;
    
    window = SDL_CreateWindow("Curse Engine, Software Rasterizer", /* Title of the SDL window */
			  SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
			  SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
			  WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS); /* Additional flag(s) */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    test = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("assets\\missing texture.bmp"));

    camera player = {
        .root = {{-10, 0, 0}, {0, 0, 0}},
        .fov = 110,
        .screen = {800, 600},//tan(fov)=WIDTH/2focalLength//focalLength=WIDTH/2tan(fov)
        .focalLength = player.screen.width/(2*tan((player.fov/2)*(PI/180))),
        .nearPlane = 2,
        .farPlane = 20000
    };

    SDL_Event e;

    bool keep_window_open = true;
	while (keep_window_open)
	{
		while(SDL_PollEvent(&e) > 0)
    	{
    		switch(e.type)
    		{
    	    	case SDL_KEYDOWN:

                    break;
                
                case SDL_QUIT:
    	        	keep_window_open = false;
    	        	break;
    		}
    	}
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);//printf("lemme guess");

        drawTri(renderer, 200, 200, 0, 400, 400, 0, 300, 250, 0);
        
        SDL_RenderPresent(renderer);//printf(", or here");
        SDL_UpdateWindowSurface(window);//printf(", or here?");
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(test);

    return 0;
}