#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

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

#define swap(x,y) { x = x + y; y = x - y; x = x - y; }

#define WIDTH 800
#define HEIGHT 600

#define DRAWDISTANCE 3200

typedef struct color
{
    Uint8 r, g, b;
} color;

typedef struct vector3
{
    int x, y, z;
} vec3;

typedef struct vector2
{
    int x, y;
} vec2;

typedef struct screenVertex
{
    color pixelColor;
    int x, y;
} screenVertex;

typedef struct vert3d
{
    color pixelColor;
    vec3 position;
} vert3d;

typedef struct triangle
{
    screenVertex vert1, vert2, vert3;
} triangle;

typedef struct triangle3d
{
    vert3d vert1, vert2, vert3;
} triangle3d;

typedef struct transform
{
    vec3 position;
    vec3 rotation;
} transform;

typedef struct camera
{
    transform playerTransform;
    int FOV;
    double focalLength;
    double zNearPlane;
    double zFarPlane;
} camera;
int player_speed;
double sensitivity;

typedef struct mesh
{
    transform root;
    vert3d* vertices;
    vec3* triIndex;
    vec2* textureCoords;
    vec3* textureIndices;
    SDL_Texture *texture;
    int vertCount, faceCount, vtexCount, ftexCount;
} mesh;

void keyboard_inputs(camera *player) 
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_W]) {
        player->playerTransform.position.z += (player_speed * 2 * cos(player->playerTransform.rotation.y));
        player->playerTransform.position.x += (player_speed * 2 * sin(player->playerTransform.rotation.y));
    }
    if (keystate[SDL_SCANCODE_S]) {
    	player->playerTransform.position.z -= (player_speed * 2 * cos(player->playerTransform.rotation.y));
    	player->playerTransform.position.x -= (player_speed * 2 * sin(player->playerTransform.rotation.y));
    }
    if (keystate[SDL_SCANCODE_A]) {
    	player->playerTransform.position.x -= (player_speed * 2 * cos(player->playerTransform.rotation.y));
    	player->playerTransform.position.z -= (player_speed * -2 * sin(player->playerTransform.rotation.y));
    }
    if (keystate[SDL_SCANCODE_D]) {
    	player->playerTransform.position.x += (player_speed * 2 * cos(player->playerTransform.rotation.y));
    	player->playerTransform.position.z += (player_speed * -2 * sin(player->playerTransform.rotation.y));
    }

    if (keystate[SDL_SCANCODE_UP]) {
        player->playerTransform.rotation.x -= sensitivity / (360 * 3.14);
    }
    if (keystate[SDL_SCANCODE_DOWN]) {
    	player->playerTransform.rotation.x += sensitivity / (360 * 3.14);
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
    	player->playerTransform.rotation.y -= sensitivity / (360 * 3.14);
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
    	player->playerTransform.rotation.y += sensitivity / (360 * 3.14);
    }
}

void renderTri(SDL_Renderer *renderer, SDL_Texture *texture, int *depthBuffer, triangle3d tri, camera cam, vec2 *uv)
{
    bool inVeiwRange = false;

    if(tri.vert1.position.z < cam.zFarPlane || tri.vert2.position.z < cam.zFarPlane || tri.vert3.position.z < cam.zFarPlane)
	{
		if(tri.vert1.position.z > cam.zNearPlane) { inVeiwRange = true; }
		if(tri.vert2.position.z > cam.zNearPlane) { inVeiwRange = true; }
		if(tri.vert3.position.z > cam.zNearPlane) { inVeiwRange = true; }
	}

    if(!inVeiwRange)
    {
        return;
    }

    int vertsInView = 0;

    triangle3d clippedTri1;

    // Calculate direction vector of the line
    int dx1 = tri.vert2.position.x - tri.vert1.position.x, 
        dx2 = tri.vert1.position.x - tri.vert3.position.x, 
        dx3 = tri.vert3.position.x - tri.vert2.position.x;

    int dy1 = tri.vert2.position.y - tri.vert1.position.y, 
        dy2 = tri.vert1.position.y - tri.vert3.position.y, 
        dy3 = tri.vert3.position.y - tri.vert2.position.y;

    int dz1 = tri.vert2.position.z - tri.vert1.position.z, 
        dz2 = tri.vert1.position.z - tri.vert3.position.z, 
        dz3 = tri.vert3.position.z - tri.vert2.position.z;
    
    // Calculate coordinates of the new point
    int clipx1 = ((tri.vert1.position.z-cam.zNearPlane)/dz1)*dx1 + tri.vert1.position.x;
    int clipx2 = ((tri.vert2.position.z-cam.zNearPlane)/dz2)*dx2 + tri.vert2.position.x;
    int clipx3 = ((tri.vert3.position.z-cam.zNearPlane)/dz3)*dx3 + tri.vert3.position.x;

    int clipy1 = ((tri.vert1.position.z-cam.zNearPlane)/dz1)*dx1 + tri.vert1.position.x;
    int clipy2 = ((tri.vert2.position.z-cam.zNearPlane)/dz2)*dx2 + tri.vert2.position.x;
    int clipy3 = ((tri.vert3.position.z-cam.zNearPlane)/dz3)*dx3 + tri.vert3.position.x;

    clippedTri1.vert1.pixelColor = tri.vert1.pixelColor;
    clippedTri1.vert1.position.x = tri.vert1.position.x;
    clippedTri1.vert1.position.y = tri.vert1.position.y;
    clippedTri1.vert1.position.z = max(tri.vert1.position.z, cam.zNearPlane);

    clippedTri1.vert2.pixelColor = tri.vert2.pixelColor;
    clippedTri1.vert2.position.x = tri.vert2.position.x;
    clippedTri1.vert2.position.y = tri.vert2.position.y;
    clippedTri1.vert2.position.z = max(tri.vert2.position.z, cam.zNearPlane);

    clippedTri1.vert3.pixelColor = tri.vert3.pixelColor;
    clippedTri1.vert3.position.x = tri.vert3.position.x;
    clippedTri1.vert3.position.y = tri.vert3.position.y;
    clippedTri1.vert3.position.z = max(tri.vert3.position.z, cam.zNearPlane);

    triangle projTri = {
        .vert1 = {
            .pixelColor = clippedTri1.vert1.pixelColor,
            .x = ( clippedTri1.vert1.position.x * cam.focalLength ) / ( clippedTri1.vert1.position.z + cam.focalLength ),
            .y = ( clippedTri1.vert1.position.y * cam.focalLength ) / ( clippedTri1.vert1.position.z + cam.focalLength )
        },
        .vert2 = {
            .pixelColor = clippedTri1.vert2.pixelColor,
            .x = ( clippedTri1.vert2.position.x * cam.focalLength ) / ( clippedTri1.vert2.position.z + cam.focalLength ),
            .y = ( clippedTri1.vert2.position.y * cam.focalLength ) / ( clippedTri1.vert2.position.z + cam.focalLength )
        },
        .vert3 = {
            .pixelColor = clippedTri1.vert3.pixelColor,
            .x = ( clippedTri1.vert3.position.x * cam.focalLength ) / ( clippedTri1.vert3.position.z + cam.focalLength ),
            .y = ( clippedTri1.vert3.position.y * cam.focalLength ) / ( clippedTri1.vert3.position.z + cam.focalLength )
        }
    };

    int vert1bright = max(cam.zFarPlane - clippedTri1.vert1.position.z, cam.zNearPlane)/cam.zFarPlane;
    SDL_Vertex vert1 = {{projTri.vert1.x + (WIDTH/2), projTri.vert1.y + (HEIGHT/2)},
                        {255, 255, 255, 255},
                        {uv[0].x, uv[0].y}
    };

    int vert2bright = max(cam.zFarPlane - clippedTri1.vert2.position.z, cam.zNearPlane)/cam.zFarPlane;
    SDL_Vertex vert2 = {{projTri.vert2.x + (WIDTH/2), projTri.vert2.y + (HEIGHT/2)},
                        {255, 255, 255, 255},
                        {uv[1].x, uv[1].y}
    };

    int vert3bright = max(cam.zFarPlane - clippedTri1.vert3.position.z, cam.zNearPlane)/cam.zFarPlane;
    SDL_Vertex vert3 = {{projTri.vert3.x + (WIDTH/2), projTri.vert3.y + (HEIGHT/2)},
                        {255, 255, 255, 255},
                        {uv[2].x, uv[2].y}
    };

    SDL_Vertex vertices[] = {
        vert1,
        vert2,
        vert3
    };

    SDL_RenderGeometry(renderer, texture, vertices, 3, NULL, 0);
}

triangle projectTri(SDL_Renderer *renderer, SDL_Texture *texture, int *depthBuffer, triangle3d tri, camera cam, vec2 *uv)
{
    //translate relative to camera
    triangle3d tranlationTri = {
        .vert1 = {
            .pixelColor = tri.vert1.pixelColor,
            .position.x = tri.vert1.position.x - cam.playerTransform.position.x,
            .position.y = tri.vert1.position.y - cam.playerTransform.position.y,
            .position.z = tri.vert1.position.z - cam.playerTransform.position.z
        },
        .vert2 = {
            .pixelColor = tri.vert2.pixelColor,
            .position.x = tri.vert2.position.x - cam.playerTransform.position.x,
            .position.y = tri.vert2.position.y - cam.playerTransform.position.y,
            .position.z = tri.vert2.position.z - cam.playerTransform.position.z
        },
        .vert3 = {
            .pixelColor = tri.vert3.pixelColor,
            .position.x = tri.vert3.position.x - cam.playerTransform.position.x,
            .position.y = tri.vert3.position.y - cam.playerTransform.position.y,
            .position.z = tri.vert3.position.z - cam.playerTransform.position.z
        }
    };

    triangle3d rotationYCamTri = {
        .vert1 = {
            .pixelColor = tranlationTri.vert1.pixelColor,
            
            .position.x = ( tranlationTri.vert1.position.x * cos(cam.playerTransform.rotation.y) ) -
                          ( tranlationTri.vert1.position.z * sin(cam.playerTransform.rotation.y) ),

            .position.y = tranlationTri.vert1.position.y,

            .position.z = ( tranlationTri.vert1.position.z * cos(cam.playerTransform.rotation.y) ) +
                          ( tranlationTri.vert1.position.x * sin(cam.playerTransform.rotation.y) )
        },

        .vert2 = {
            .pixelColor = tranlationTri.vert2.pixelColor,

            .position.x = ( tranlationTri.vert2.position.x * cos(cam.playerTransform.rotation.y) ) -
                          ( tranlationTri.vert2.position.z * sin(cam.playerTransform.rotation.y) ),

            .position.y = tranlationTri.vert2.position.y,

            .position.z = ( tranlationTri.vert2.position.z * cos(cam.playerTransform.rotation.y) ) +
                          ( tranlationTri.vert2.position.x * sin(cam.playerTransform.rotation.y) )
        },

        .vert3 = {
            .pixelColor = tranlationTri.vert3.pixelColor,

            .position.x = ( tranlationTri.vert3.position.x * cos(cam.playerTransform.rotation.y) ) -
                          ( tranlationTri.vert3.position.z * sin(cam.playerTransform.rotation.y) ),

            .position.y = tranlationTri.vert3.position.y,

            .position.z = ( tranlationTri.vert3.position.z * cos(cam.playerTransform.rotation.y) ) +
                          ( tranlationTri.vert3.position.x * sin(cam.playerTransform.rotation.y) )
        }
    };

    //rotate relative to camera
    triangle3d rotationXCamTri = {
        .vert1 = {
            .pixelColor = rotationYCamTri.vert1.pixelColor,

            .position.x = rotationYCamTri.vert1.position.x,

            .position.y = ( rotationYCamTri.vert1.position.y * cos(cam.playerTransform.rotation.x) ) - 
                          ( rotationYCamTri.vert1.position.z * sin(cam.playerTransform.rotation.x) ),

            .position.z = ( rotationYCamTri.vert1.position.z * cos(cam.playerTransform.rotation.x) ) + 
                          ( rotationYCamTri.vert1.position.y * sin(cam.playerTransform.rotation.x) )
        },

        .vert2 = {
            .pixelColor = rotationYCamTri.vert2.pixelColor,

            .position.x = rotationYCamTri.vert2.position.x,

            .position.y = ( rotationYCamTri.vert2.position.y * cos(cam.playerTransform.rotation.x) ) - 
                          ( rotationYCamTri.vert2.position.z * sin(cam.playerTransform.rotation.x) ),

            .position.z = ( rotationYCamTri.vert2.position.z * cos(cam.playerTransform.rotation.x) ) + 
                          ( rotationYCamTri.vert2.position.y * sin(cam.playerTransform.rotation.x) )
        },

        .vert3 = {
            .pixelColor = rotationYCamTri.vert3.pixelColor,

            .position.x = rotationYCamTri.vert3.position.x,

            .position.y = ( rotationYCamTri.vert3.position.y * cos(cam.playerTransform.rotation.x) ) - 
                          ( rotationYCamTri.vert3.position.z * sin(cam.playerTransform.rotation.x) ),

            .position.z = ( rotationYCamTri.vert3.position.z * cos(cam.playerTransform.rotation.x) ) + 
                          ( rotationYCamTri.vert3.position.y * sin(cam.playerTransform.rotation.x) )
        }
    };

    renderTri(renderer, texture, depthBuffer, rotationXCamTri, cam, uv);
}

void addTextureToObject(mesh *obj, SDL_Texture *texture)
{
	obj->texture = texture;
}

void addVertexToObject(mesh *obj, vec3 vertex) 
{
    int vCount = obj->vertCount;

    obj->vertices = (vert3d*)realloc(obj->vertices, sizeof(vert3d) * (vCount + 1));
    
    obj->vertices[vCount].position.x = vertex.x * 100;
    obj->vertices[vCount].position.y = vertex.y * 100;
    obj->vertices[vCount].position.z = vertex.z * 100;
    obj->vertCount++;
}

void addTexCoordsToObject(mesh *obj, vec2 texcoord) 
{
    int vtexCount = obj->vtexCount;

    obj->textureCoords = (vec2*)realloc(obj->textureCoords, sizeof(vec2) * (vtexCount + 1));
    
    obj->textureCoords[vtexCount] = texcoord;
    obj->vtexCount++;
}

void addFaceToObject(mesh *obj, vec3 face) 
{
    int faceCount = obj->faceCount;

    obj->triIndex = (vec3*)realloc(obj->triIndex, sizeof(vec3) * (faceCount + 1));
    
    obj->triIndex[faceCount].x = face.x;
    obj->triIndex[faceCount].y = face.y;
    obj->triIndex[faceCount].z = face.z;

    obj->faceCount++;
}

void addFaceTextureToObject(mesh *obj, vec3 face) 
{
    int ftexCount = obj->ftexCount;

    obj->textureIndices = (vec3*)realloc(obj->textureIndices, sizeof(vec3) * (ftexCount + 1));
    
    obj->textureIndices[ftexCount].x = face.x;
    obj->textureIndices[ftexCount].y = face.y;
    obj->textureIndices[ftexCount].z = face.z;
}

mesh importObj(const char filename[], SDL_Renderer *renderer, char *args) 
{
    FILE *pF = fopen(filename, "r");
    if (!pF) 
	{
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
	else
	{
		printf("\nSuccessfully opened file: \n\t%s\n", filename);
	}

    char buffer[255];
    char material[7] = "mtllib ";
    char imgmaterial[7] = "map_Kd ";
    char vertex[2] = "v ";
    char texcoord[3] = "vt ";
    char face[2] = "f ";

    mesh returnObject = {0}; // Initialize with zeros

    int vertexCount = 0;
	int vtextureCount = 0;
    int faceCount = 0;
    int ftexCount = 0;

    for (int vert = 0; fgets(buffer, 255, pF) != NULL; ++vert) 
	{
	    if (strncmp(buffer, material, 7) == 0)
	    {
	    	char texFileName[255];
	    	char texFileExtension[255];

	    	sscanf(buffer + 7, "%[^\t\n]", texFileName);
			sprintf(texFileExtension, "assets\\%s", texFileName);

			printf("\t%s\n", texFileExtension);

			SDL_Texture* objtexture = NULL;
			FILE *TexF = fopen(texFileExtension, "r");
			if (TexF == NULL)
			{
				printf("Material not found\n");
			}

			while (fgets(buffer, 255, TexF) != NULL) 
			{
				if(strncmp(buffer, imgmaterial, 7) == 0)
				{
					sscanf(buffer + 7, "%[^\t\n]", texFileName);
					sprintf(texFileExtension, "assets\\%s", texFileName);
					printf("\t%s\n", texFileExtension);

					SDL_Surface* bmptexture = SDL_LoadBMP(texFileExtension);
	    			objtexture = SDL_CreateTextureFromSurface(renderer, bmptexture);

					addTextureToObject(&returnObject, objtexture);
	    			// to do in morning, just fix texturing man i dont fuckking know
				}
			}
	    }
	    if (strncmp(buffer, vertex, 2) == 0) 
	    {
	        vec3 objvertex;
	        sscanf(buffer + 2, "%lf %lf %lf", &objvertex.x, &objvertex.y, &objvertex.z);
	        addVertexToObject(&returnObject, objvertex);

	        vertexCount++;
	    } 
	    if (strncmp(buffer, texcoord, 3) == 0) 
	    {
	        vec2 objtexcoord;
	        sscanf(buffer + 3, "%lf %lf", &objtexcoord.x, &objtexcoord.y);
	        addTexCoordsToObject(&returnObject, objtexcoord);

			vtextureCount++;
	    } 
	    if (strncmp(buffer, face, 2) == 0) 
	    {
	        vec3 objface;
	        int objfacex;
	        int objfacey;
	        int objfacez;

	        vec3 objfacetexture;
	        int objfacetexturex;
	        int objfacetexturey;
	        int objfacetexturez;

	        double trash;
	        sscanf(buffer + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &objfacex, &objfacetexturex, &trash, &objfacey, &objfacetexturey, &trash, &objfacez, &objfacetexturez, &trash);

	        objface.x = (int)objfacex;
	        objface.y = (int)objfacey;
	        objface.z = (int)objfacez;
	        addFaceToObject(&returnObject, objface);

	        objfacetexture.x = (double)objfacetexturex;
	        objfacetexture.y = (double)objfacetexturey;
	        objfacetexture.z = (double)objfacetexturez;
	        addFaceTextureToObject(&returnObject, objfacetexture);

	        faceCount++;
            ftexCount++;
	    }
	}

	returnObject.vertCount = vertexCount;
	returnObject.vtexCount = vtextureCount;
	returnObject.faceCount = faceCount;
    returnObject.ftexCount = ftexCount;
	
	printf("\tmodel info:\n");
    printf("\t  - vert count %d\n", vertexCount);
	printf("\t  - vert texture count %d\n", vtextureCount);
	printf("\t  - face count %d\n", faceCount);
    printf("\t  - face texture count %d\n", ftexCount);

    if(args == "-objdat")
    {
        char *adArgs;
        sscanf((args + 7), "%[^\t\n]", adArgs);

        printf("%s\n", args);
        
        if(adArgs == " -w") 
        {
            char *outFile;
            sscanf(args + 11, "%[^\t\n]", outFile);

            FILE *writeData = fopen(outFile, "w");
            printf("%s\n", outFile);

            fprintf(writeData, "Object Data - %s\n", filename);

            fprintf(writeData, "    Vertices\n");
            for(int vert = 0; vert < returnObject.vertCount; ++vert)
            {
                vec3 objVert = {
                    .x = returnObject.vertices[vert].position.x,
                    .y = returnObject.vertices[vert].position.y,
                    .z = returnObject.vertices[vert].position.z
                };
                
                fprintf(writeData, "\tvertex %d : x-%lf y-%lf z-%lf\n", vert, objVert.x, objVert.y, objVert.z);
            }

            fprintf(writeData, "    Faces\n");
            for(int face = 0; face < returnObject.vertCount; ++face)
            {
                int objVertx = returnObject.triIndex[face].x;
                int objVerty = returnObject.triIndex[face].y;
                int objVertz = returnObject.triIndex[face].z;
                
                fprintf(writeData, "\tface %d : point1-%d point2-%d point3-%d\n", face, objVertx, objVerty, objVertz);
            }

            fclose(writeData);
        } else
        {
            printf("Object Data - %s\n", filename);

            printf("    Vertices\n");
            for(int vert = 0; vert < returnObject.vertCount; ++vert)
            {
                vec3 objVert = {
                    .x = returnObject.vertices[vert].position.x,
                    .y = returnObject.vertices[vert].position.y,
                    .z = returnObject.vertices[vert].position.z
                };
                
                printf("\tvertex %d : x-%lf y-%lf z-%lf\n", vert, objVert.x, objVert.y, objVert.z);
            }

            printf("    Faces\n");
            for(int face = 0; face < returnObject.vertCount; ++face)
            {
                int objVertx = returnObject.triIndex[face].x;
                int objVerty = returnObject.triIndex[face].y;
                int objVertz = returnObject.triIndex[face].z;
                
                printf("\tface %d : point1-%d point2-%d point3-%d\n", face, objVertx, objVerty, objVertz);
            }
        }
    }

    fclose(pF);
    return returnObject;
}

void objRender(SDL_Renderer *renderer, int *depthBuffer, camera cam, mesh objMesh)
{
    int vertCount = objMesh.vertCount;//printf("debugA");
    int faceCount = objMesh.faceCount;//printf("tex faces count - %d", objMesh.faceCount);

    for (int face_index = 0; face_index < faceCount; ++face_index)
    {
        triangle3d currentTri = {
            .vert1 = objMesh.vertices[ objMesh.triIndex[ face_index ].x ],
            .vert2 = objMesh.vertices[ objMesh.triIndex[ face_index ].y ],
            .vert3 = objMesh.vertices[ objMesh.triIndex[ face_index ].z ]
        };//printf("tex faces count - %d", objMesh.faceCount);

        vec2 vert1 = {
            .x = objMesh.textureCoords[ 11 ].x,
            .y = objMesh.textureCoords[ 11 ].y
        };
        vec2 vert2 = {
            .x = objMesh.textureCoords[ 13 ].x,
            .y = objMesh.textureCoords[ 13 ].y
        };
        vec2 vert3 = {
            .x = objMesh.textureCoords[ 17 ].x,
            .y = objMesh.textureCoords[ 17 ].y
        };
        vec2 triUV[3] = {vert1, vert2, vert3};
        
        projectTri(renderer, objMesh.texture, depthBuffer, currentTri, cam, triUV);
    }
}

int main(int argc, char* argv[])
{
    //todo, make the screen draw triangles, give the triangles color, make the triangles 3d, make cube from tris, obj importer
    SDL_Window  *window = NULL;
    SDL_Renderer *renderer = NULL;
    
    window = SDL_CreateWindow("Curse Engine, Software Rasterizer", /* Title of the SDL window */
			  SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
			  SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
			  WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS); /* Additional flag(s) */

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    camera player = {
        .playerTransform = {
            .position = {0, 0, 0},
            .rotation = {0, 0, 0}
        },
        .FOV = 90,
        .focalLength = 0,
        .zNearPlane = 2,
        .zFarPlane = DRAWDISTANCE
    };
    player_speed = 20;
    sensitivity = 20;
    player.focalLength = sin((180 - player.FOV/2)*3.14 / 180) * WIDTH/2 * sin(player.FOV*3.14/180);

    mesh sproRoom = importObj("assets\\spyro.obj", renderer, "0");
    mesh monke = importObj("assets\\monke.obj", renderer, "-objdat -w monkedebug.txt");

    int *depthBuffer = (int *)malloc((WIDTH * HEIGHT) * sizeof(int));

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

        keyboard_inputs(&player);

        objRender(renderer, depthBuffer, player, sproRoom);
        //objRender(renderer, depthBuffer, player, monke);
        
        SDL_RenderPresent(renderer);//printf(", or here");
        SDL_UpdateWindowSurface(window);//printf(", or here?");
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    free(sproRoom.vertices);
    free(sproRoom.triIndex);
    free(sproRoom.textureCoords);
    free(sproRoom.textureIndices);

    free(monke.vertices);
    free(monke.triIndex);
    free(monke.textureCoords);
    free(monke.textureIndices);

    return 0;
}