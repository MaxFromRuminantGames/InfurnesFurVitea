#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define PI 3.1415

#define max(a, b)            \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a, b)            \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define arraySize(arr) ( sizeof(arr) / sizeof(__typeof__ (arr)) )

#define clamp(x, min, max) ( ( x < min ? min : x ) > max ? max : ( x < min ? min : x ) )

#define abs(x) (x<0)?-x:x

#define swap(x,y) { x = x + y; y = x - y; x = x - y; }

#define WIDTH 800
#define HEIGHT 600

typedef struct vector3      { int x, y, z; } vec3;
typedef struct vector2      { int x, y; }    vec2;
typedef struct vector2Float { double x, y; } vec2F;

typedef struct color { Uint8 r, g, b; } color;

typedef struct vert3D { color viewcolor; vec3 pos; } vert3D;
typedef struct vert2D { color viewcolor; vec2 pos; } vert2D;

typedef struct triangle3D { vert3D vert1, vert2, vert3; vec2F uv1, uv2, uv3; } triangle3D;
typedef struct mesh { triangle3D *tris; SDL_Texture *texture; } mesh;

typedef struct transform { vec3 position; vec3 rotation; } transform;
typedef struct objData { int faceCount; int vertCount; int uvCount; int texCount;} objData;
typedef struct object { transform root; mesh mesh; objData data; } object;

typedef struct camera
{
    transform root;

    struct screen
    { int width, height; } screen;

    int fov;//in degrees
    int focalLength;
    int nearPlane, farPlane;
} camera;

typedef struct queue { triangle3D* tris; int *texInd; SDL_Texture **textures; int count; int texCount; } queue;
typedef struct Scene3D { queue triQueue; objData data; } Scene3D;

Scene3D CreateScene3D()
{
    Scene3D returnScene;

    returnScene.triQueue.tris = NULL;
    returnScene.triQueue.texInd = NULL;
    returnScene.triQueue.textures = NULL;
    
    returnScene.triQueue.tris = malloc(2 * sizeof(triangle3D));
    returnScene.triQueue.texInd = malloc(2 * sizeof(int));
    returnScene.triQueue.textures = malloc(2 * sizeof(SDL_Texture*));

    returnScene.triQueue.count = 1;
    returnScene.triQueue.texCount = 1;

    return returnScene;
}

void queueTri(SDL_Texture *texture, camera cam, triangle3D tri, queue *q)
{
    triangle3D translateTri = {
        .vert1 = {
            .viewcolor = tri.vert1.viewcolor,
            .pos = {
                .x = tri.vert1.pos.x - cam.root.position.x,
                .y = tri.vert1.pos.y - cam.root.position.y,
                .z = tri.vert1.pos.z - cam.root.position.z
            }
        },
        .vert2 = {
            .viewcolor = tri.vert2.viewcolor,
            .pos = {
                .x = tri.vert2.pos.x - cam.root.position.x,
                .y = tri.vert2.pos.y - cam.root.position.y,
                .z = tri.vert2.pos.z - cam.root.position.z
            }
        },
        .vert3 = {
            .viewcolor = tri.vert3.viewcolor,
            .pos = {
                .x = tri.vert3.pos.x - cam.root.position.x,
                .y = tri.vert3.pos.y - cam.root.position.y,
                .z = tri.vert3.pos.z - cam.root.position.z
            }
        }
    };

    triangle3D yRotTri = {
        .vert1 = {
            .pos = {
                .x = (translateTri.vert1.pos.x * cos(-cam.root.rotation.y * (PI / 180))) +
                     (translateTri.vert1.pos.z * sin(-cam.root.rotation.y * (PI / 180))),

                .y =  translateTri.vert1.pos.y,

                .z = (translateTri.vert1.pos.z * cos(-cam.root.rotation.y * (PI / 180))) -
                     (translateTri.vert1.pos.x * sin(-cam.root.rotation.y * (PI / 180)))
            }
        },
        .vert2 = {
            .pos = {
                .x = (translateTri.vert2.pos.x * cos(-cam.root.rotation.y * (PI / 180))) +
                     (translateTri.vert2.pos.z * sin(-cam.root.rotation.y * (PI / 180))),

                .y =  translateTri.vert2.pos.y,

                .z = (translateTri.vert2.pos.z * cos(-cam.root.rotation.y * (PI / 180))) -
                     (translateTri.vert2.pos.x * sin(-cam.root.rotation.y * (PI / 180)))
            }
        },
        .vert3 = {
            .pos = {
                .x = (translateTri.vert3.pos.x * cos(-cam.root.rotation.y * (PI / 180))) +
                     (translateTri.vert3.pos.z * sin(-cam.root.rotation.y * (PI / 180))),

                .y =  translateTri.vert3.pos.y,

                .z = (translateTri.vert3.pos.z * cos(-cam.root.rotation.y * (PI / 180))) -
                     (translateTri.vert3.pos.x * sin(-cam.root.rotation.y * (PI / 180)))
            }
        }
    };

    if(yRotTri.vert1.pos.z == 0) {yRotTri.vert1.pos.z = 1;}
    if(yRotTri.vert2.pos.z == 0) {yRotTri.vert2.pos.z = 1;}
    if(yRotTri.vert3.pos.z == 0) {yRotTri.vert3.pos.z = 1;}

    if(min(min(yRotTri.vert1.pos.x/yRotTri.vert1.pos.z, yRotTri.vert2.pos.x/yRotTri.vert2.pos.z), yRotTri.vert3.pos.x/yRotTri.vert3.pos.z) > 
       cam.screen.width/2)  
       { return; }
    if(max(max(yRotTri.vert1.pos.x/yRotTri.vert1.pos.z, yRotTri.vert2.pos.x/yRotTri.vert2.pos.z), yRotTri.vert3.pos.x/yRotTri.vert3.pos.z) < 
       -cam.screen.width/2) 
       { return; }

    triangle3D xRotTri = {
        .vert1 = {
            .pos = {
                .x =  yRotTri.vert1.pos.x,
                .y = (yRotTri.vert1.pos.y * cos(-cam.root.rotation.x * (PI / 180))) - (yRotTri.vert1.pos.z * sin(-cam.root.rotation.x * (PI / 180))),
                .z = (yRotTri.vert1.pos.z * cos(-cam.root.rotation.x * (PI / 180))) + (yRotTri.vert1.pos.y * sin(-cam.root.rotation.x * (PI / 180)))
            }
        },
        .vert2 = {
            .pos = {
                .x =  yRotTri.vert2.pos.x,
                .y = (yRotTri.vert2.pos.y * cos(-cam.root.rotation.x * (PI / 180))) - (yRotTri.vert2.pos.z * sin(-cam.root.rotation.x * (PI / 180))),
                .z = (yRotTri.vert2.pos.z * cos(-cam.root.rotation.x * (PI / 180))) + (yRotTri.vert2.pos.y * sin(-cam.root.rotation.x * (PI / 180)))
            }
        },
        .vert3 = {
            .pos = {
                .x =  yRotTri.vert3.pos.x,
                .y = (yRotTri.vert3.pos.y * cos(-cam.root.rotation.x * (PI / 180))) - (yRotTri.vert3.pos.z * sin(-cam.root.rotation.x * (PI / 180))),
                .z = (yRotTri.vert3.pos.z * cos(-cam.root.rotation.x * (PI / 180))) + (yRotTri.vert3.pos.y * sin(-cam.root.rotation.x * (PI / 180)))
            }
        }
    };

    if(max(max(xRotTri.vert1.pos.z, xRotTri.vert2.pos.z), xRotTri.vert3.pos.z) < cam.nearPlane) { return; }
    if(min(min(xRotTri.vert1.pos.z, xRotTri.vert2.pos.z), xRotTri.vert3.pos.z) > cam.farPlane)  { return; }

    if(xRotTri.vert1.pos.z < cam.nearPlane) { xRotTri.vert1.pos.z = cam.nearPlane; }
    if(xRotTri.vert2.pos.z < cam.nearPlane) { xRotTri.vert2.pos.z = cam.nearPlane; }
    if(xRotTri.vert3.pos.z < cam.nearPlane) { xRotTri.vert3.pos.z = cam.nearPlane; }
    
    triangle3D projTri = {
        .vert1 = {
            .viewcolor = tri.vert1.viewcolor,
            .pos = {
                .x = (xRotTri.vert1.pos.x * cam.focalLength) / (xRotTri.vert1.pos.z),
                .y = (xRotTri.vert1.pos.y * cam.focalLength) / (xRotTri.vert1.pos.z),
                .z =  xRotTri.vert1.pos.z
            }
        },
        .vert2 = {
            .viewcolor = tri.vert2.viewcolor,
            .pos = {
                .x = (xRotTri.vert2.pos.x * cam.focalLength) / (xRotTri.vert2.pos.z),
                .y = (xRotTri.vert2.pos.y * cam.focalLength) / (xRotTri.vert2.pos.z),
                .z =  xRotTri.vert2.pos.z
            }
        },
        .vert3 = {
            .viewcolor = tri.vert3.viewcolor,
            .pos = {
                .x = (xRotTri.vert3.pos.x * cam.focalLength) / (xRotTri.vert3.pos.z),
                .y = (xRotTri.vert3.pos.y * cam.focalLength) / (xRotTri.vert3.pos.z),
                .z =  xRotTri.vert3.pos.z
            }
        },
        .uv1 = tri.uv1,
        .uv2 = tri.uv2,
        .uv3 = tri.uv3
    };

    int smallX = min(projTri.vert1.pos.x, min(projTri.vert2.pos.x, projTri.vert3.pos.x));
    int largeX = max(projTri.vert1.pos.x, max(projTri.vert2.pos.x, projTri.vert3.pos.x));
    int smallY = min(projTri.vert1.pos.y, min(projTri.vert2.pos.y, projTri.vert3.pos.y));
    int largeY = max(projTri.vert1.pos.y, max(projTri.vert2.pos.y, projTri.vert3.pos.y));
    if(smallX > cam.screen.width  || largeX < -cam.screen.width)  { return; }
    if(smallY > cam.screen.height || largeX < -cam.screen.height) { return; }

    int triDist = max(xRotTri.vert1.pos.z, max(xRotTri.vert2.pos.z, xRotTri.vert3.pos.z));
    
    //SDL_RenderGeometry(renderer, texture, vertices, 3, NULL, 0); //to do in the morning - replace w/zBuffered rasterizer
    //Increase the size of the array
    (*q).tris = realloc((*q).tris, ((*q).count + 1) * sizeof(triangle3D));
    (*q).texInd = realloc(q->texInd, ((*q).count + 1) * sizeof(int));
    (*q).textures = realloc((*q).textures, ((*q).count + 1) * sizeof(SDL_Texture*));

    //Find the appropriate position to insert the new triangle based on the z-axis
    int insertIndex = (*q).count;
    while (insertIndex > 0 && triDist > max((*q).tris[insertIndex - 1].vert1.pos.z, 
                                        max((*q).tris[insertIndex - 1].vert2.pos.z, 
                                            (*q).tris[insertIndex - 1].vert3.pos.z)) ) 
    {
        (*q).tris[insertIndex] = (*q).tris[insertIndex - 1];
        (*q).texInd[insertIndex] = (*q).texInd[insertIndex - 1];
        (*q).textures[insertIndex] = (*q).textures[insertIndex - 1];
        insertIndex--;
    }

    bool texIsQueued = false;
    for(int texInsertInd = 0; texInsertInd < (*q).texCount; ++texInsertInd)
    { if((*q).textures[texInsertInd] == texture) { (*q).texInd[insertIndex-1] = texInsertInd; texIsQueued = true; } }

    if(texIsQueued == false)
    {
        (*q).textures = realloc((*q).textures, ((*q).texCount + 1) * sizeof(SDL_Texture*));
        (*q).texCount++;
        (*q).texInd[insertIndex - 1] = (*q).texCount - 1;
        (*q).textures[(*q).texCount - 1] = texture;
    }

    //Insert the triangle and texture
    (*q).tris[insertIndex] = projTri;
    (*q).textures[insertIndex] = texture;
    (*q).count++;
}

void queueObj(camera cam, object obj, queue *triQueue)
{
    for(int triIndex = 0; triIndex < obj.data.faceCount; ++triIndex)
    {
        triangle3D tri = obj.mesh.tris[triIndex];
        
        triangle3D translateTri = {
            .vert1 = {
                .viewcolor = tri.vert1.viewcolor,
                .pos = {
                    .x = tri.vert1.pos.x - obj.root.position.x,
                    .y = tri.vert1.pos.y - obj.root.position.y,
                    .z = tri.vert1.pos.z - obj.root.position.z
                }
            },
            .vert2 = {
                .viewcolor = tri.vert2.viewcolor,
                .pos = {
                    .x = tri.vert2.pos.x - obj.root.position.x,
                    .y = tri.vert2.pos.y - obj.root.position.y,
                    .z = tri.vert2.pos.z - obj.root.position.z
                }
            },
            .vert3 = {
                .viewcolor = tri.vert3.viewcolor,
                .pos = {
                    .x = tri.vert3.pos.x - obj.root.position.x,
                    .y = tri.vert3.pos.y - obj.root.position.y,
                    .z = tri.vert3.pos.z - obj.root.position.z
                }
            },
            .uv1 = tri.uv1,
            .uv2 = tri.uv2,
            .uv3 = tri.uv3
        };

        queueTri(obj.mesh.texture, cam, translateTri, triQueue);
    }
}

object importObj(char *filename, SDL_Renderer *renderer) 
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
    char material[8] = "mtllib ";
    char imgmaterial[8] = "map_Kd ";
    char vertex[3] = "v ";
    char texcoord[4] = "vt ";
    char face[3] = "f ";

    object returnObject = {0};
    vert3D *vertices = calloc(1, sizeof(vert3D));
    vec2F *UVs = calloc(1, sizeof(vec2F));
    triangle3D *faces = calloc(1, sizeof(triangle3D));

    int vertexCount = 0;
	int vtextureCount = 0;
    int faceCount = 0;

    for (int vert = 0; fgets(buffer, 255, pF) != NULL; ++vert) 
	{
	    if (strncmp(buffer, material, 7) == 0)
	    {
	    	char texFileName[255];
            char texFileExtension[255];

	    	sscanf(buffer + 7, "%[^\t\n]", texFileName);
			printf("\tassets/%s\n", texFileName);

            strcpy(texFileExtension, "./assets/");
            strcat(texFileExtension, texFileName);

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
					sprintf(texFileExtension, "assets/%s", texFileName);
					printf("\t%s\n", texFileExtension);

					objtexture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(texFileExtension));

					returnObject.mesh.texture = objtexture;
				}
			}
	    }

	    if (strncmp(buffer, vertex, 2) == 0) 
	    {
	        double objvertx, objverty, objvertz;

	        sscanf(buffer + 2, "%lf %lf %lf", &objvertx, &objverty, &objvertz);
	        vertices = (vert3D*)realloc(vertices, sizeof(vert3D) * (vertexCount + 1));

            vertices[vertexCount].pos = (vec3){
                .x = (100 * objvertx),
                .y = (100 * objverty),
                .z = (100 * objvertz)
            };

            vertices[vertexCount].viewcolor = (color){255, 255, 255};

	        vertexCount++;
	    }

	    if (strncmp(buffer, texcoord, 3) == 0) 
	    {
	        vec2F objtexcoord;
	        sscanf(buffer + 3, "%lf %lf", &objtexcoord.x, &objtexcoord.y);

	        UVs = realloc(UVs, sizeof(vec2F) * (vtextureCount + 1));
            UVs[vtextureCount] = (vec2F){
                .x = objtexcoord.x,
                .y = objtexcoord.y
            };

			vtextureCount++;
	    }

	    if (strncmp(buffer, face, 2) == 0) 
	    {
	        int objfacex, objfacey, objfacez;
	        int objfacetexturex, objfacetexturey, objfacetexturez;

	        int trash;
	        sscanf(buffer + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &objfacex, &objfacetexturex, &trash, 
                                                             &objfacey, &objfacetexturey, &trash, 
                                                             &objfacez, &objfacetexturez, &trash);

	        faces = realloc(faces, (faceCount + 1) * sizeof(triangle3D));
            faces[faceCount] = (triangle3D)
            {
                .vert1 = vertices[objfacex - 1],
                .vert2 = vertices[objfacey - 1],
                .vert3 = vertices[objfacez - 1],

                .uv1 = { .x = UVs[objfacetexturex - 1].x, .y = UVs[objfacetexturex - 1].y },
                .uv2 = { .x = UVs[objfacetexturey - 1].x, .y = UVs[objfacetexturey - 1].y },
                .uv3 = { .x = UVs[objfacetexturez - 1].x, .y = UVs[objfacetexturez - 1].y }
            };

	        faceCount++;
	    }
	}

    for(int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
    {
        returnObject.mesh.tris = realloc(returnObject.mesh.tris, (faceIndex + 1) * sizeof(triangle3D));
        returnObject.mesh.tris[faceIndex] = faces[faceIndex];
    }

	returnObject.data.vertCount = vertexCount;
	returnObject.data.uvCount = vtextureCount;
	returnObject.data.faceCount = faceCount;
	
	printf("\tmodel info:\n");
    printf("\t  - vert count %d\n", vertexCount);
	printf("\t  - vert texture count %d\n", vtextureCount);
	printf("\t  - face count %d\n\n", faceCount);

    fclose(pF);
    free(vertices);
    free(UVs);
    free(faces);

    return returnObject;
}

void renderSkyBox(SDL_Renderer *renderer, SDL_Texture *texture, object skyBox, camera cam)
{
    skyBox.mesh.texture = texture;
    object scaledBox;

    scaledBox.mesh.texture = texture;
    scaledBox.mesh.tris = malloc(sizeof(triangle3D) * skyBox.data.faceCount);

    int edge = cam.farPlane;
    color skyHue = {255, 255, 255};

    for(int triInd = 0; triInd < skyBox.data.faceCount; ++triInd)
    {triangle3D tri = skyBox.mesh.tris[triInd];
        triangle3D scaledTri = {
            .vert1 = {
                .viewcolor = skyHue,
                .pos = {skyBox.mesh.tris[triInd].vert1.pos.x*edge, 
                        skyBox.mesh.tris[triInd].vert1.pos.y*edge, 
                        skyBox.mesh.tris[triInd].vert1.pos.z*edge}
            },
            .vert2 = {
                .viewcolor = skyHue,
                .pos = {skyBox.mesh.tris[triInd].vert2.pos.x*edge, 
                        skyBox.mesh.tris[triInd].vert2.pos.y*edge, 
                        skyBox.mesh.tris[triInd].vert2.pos.z*edge}
            },
            .vert3 = {
                .viewcolor = skyHue,
                .pos = {skyBox.mesh.tris[triInd].vert3.pos.x*edge, 
                        skyBox.mesh.tris[triInd].vert3.pos.y*edge, 
                        skyBox.mesh.tris[triInd].vert3.pos.z*edge}
            },
            .uv1 = { tri.uv1.x, tri.uv1.y },
            .uv2 = { tri.uv2.x, tri.uv2.y },
            .uv3 = { tri.uv3.x, tri.uv3.y }
        };

        scaledBox.mesh.tris[triInd].vert1.pos.x = scaledTri.vert1.pos.x;
        scaledBox.mesh.tris[triInd].vert1.pos.y = scaledTri.vert1.pos.y;
        scaledBox.mesh.tris[triInd].vert1.pos.z = scaledTri.vert1.pos.z;
        scaledBox.mesh.tris[triInd].uv1.x = scaledTri.uv1.x;
        scaledBox.mesh.tris[triInd].uv1.y = scaledTri.uv1.y;

        scaledBox.mesh.tris[triInd].vert2.pos.x = scaledTri.vert2.pos.x;
        scaledBox.mesh.tris[triInd].vert2.pos.y = scaledTri.vert2.pos.y;
        scaledBox.mesh.tris[triInd].vert2.pos.z = scaledTri.vert2.pos.z;
        scaledBox.mesh.tris[triInd].uv2.x = scaledTri.uv2.x;
        scaledBox.mesh.tris[triInd].uv2.y = scaledTri.uv2.y;
        
        scaledBox.mesh.tris[triInd].vert3.pos.x = scaledTri.vert3.pos.x;
        scaledBox.mesh.tris[triInd].vert3.pos.y = scaledTri.vert3.pos.y;
        scaledBox.mesh.tris[triInd].vert3.pos.z = scaledTri.vert3.pos.z;
        scaledBox.mesh.tris[triInd].uv3.x = scaledTri.uv3.x;
        scaledBox.mesh.tris[triInd].uv3.y = scaledTri.uv3.y;
    }

    for(int triInd = 0; triInd < skyBox.data.faceCount; ++triInd)
    {triangle3D tri = scaledBox.mesh.tris[triInd];
        triangle3D yRotTri = {
            .vert1 = {
                .pos = {
                    .x = (tri.vert1.pos.x * cos(-cam.root.rotation.y * (PI / 180))) +
                         (tri.vert1.pos.z * sin(-cam.root.rotation.y * (PI / 180))),

                    .y =  tri.vert1.pos.y,

                    .z = (tri.vert1.pos.z * cos(-cam.root.rotation.y * (PI / 180))) -
                         (tri.vert1.pos.x * sin(-cam.root.rotation.y * (PI / 180)))
                }
            },
            .vert2 = {
                .pos = {
                    .x = (tri.vert2.pos.x * cos(-cam.root.rotation.y * (PI / 180))) +
                         (tri.vert2.pos.z * sin(-cam.root.rotation.y * (PI / 180))),

                    .y =  tri.vert2.pos.y,

                    .z = (tri.vert2.pos.z * cos(-cam.root.rotation.y * (PI / 180))) -
                         (tri.vert2.pos.x * sin(-cam.root.rotation.y * (PI / 180)))
                }
            },
            .vert3 = {
                .pos = {
                    .x = (tri.vert3.pos.x * cos(-cam.root.rotation.y * (PI / 180))) +
                         (tri.vert3.pos.z * sin(-cam.root.rotation.y * (PI / 180))),

                    .y =  tri.vert3.pos.y,

                    .z = (tri.vert3.pos.z * cos(-cam.root.rotation.y * (PI / 180))) -
                         (tri.vert3.pos.x * sin(-cam.root.rotation.y * (PI / 180)))
                }
            }
        };

        if(yRotTri.vert1.pos.z == 0) {yRotTri.vert1.pos.z = 1;}
        if(yRotTri.vert2.pos.z == 0) {yRotTri.vert2.pos.z = 1;}
        if(yRotTri.vert3.pos.z == 0) {yRotTri.vert3.pos.z = 1;}

        if(min(min(yRotTri.vert1.pos.x/yRotTri.vert1.pos.z, yRotTri.vert2.pos.x/yRotTri.vert2.pos.z), yRotTri.vert3.pos.x/yRotTri.vert3.pos.z) > 
           cam.screen.width/2)  
           { return; }
        if(max(max(yRotTri.vert1.pos.x/yRotTri.vert1.pos.z, yRotTri.vert2.pos.x/yRotTri.vert2.pos.z), yRotTri.vert3.pos.x/yRotTri.vert3.pos.z) < 
           -cam.screen.width/2) 
           { return; }

        triangle3D xRotTri = {
            .vert1 = {
                .pos = {
                    .x =  yRotTri.vert1.pos.x,
                    .y = (yRotTri.vert1.pos.y * cos(-cam.root.rotation.x * (PI / 180))) - (yRotTri.vert1.pos.z * sin(-cam.root.rotation.x * (PI / 180))),
                    .z = (yRotTri.vert1.pos.z * cos(-cam.root.rotation.x * (PI / 180))) + (yRotTri.vert1.pos.y * sin(-cam.root.rotation.x * (PI / 180)))
                }
            },
            .vert2 = {
                .pos = {
                    .x =  yRotTri.vert2.pos.x,
                    .y = (yRotTri.vert2.pos.y * cos(-cam.root.rotation.x * (PI / 180))) - (yRotTri.vert2.pos.z * sin(-cam.root.rotation.x * (PI / 180))),
                    .z = (yRotTri.vert2.pos.z * cos(-cam.root.rotation.x * (PI / 180))) + (yRotTri.vert2.pos.y * sin(-cam.root.rotation.x * (PI / 180)))
                }
            },
            .vert3 = {
                .pos = {
                    .x =  yRotTri.vert3.pos.x,
                    .y = (yRotTri.vert3.pos.y * cos(-cam.root.rotation.x * (PI / 180))) - (yRotTri.vert3.pos.z * sin(-cam.root.rotation.x * (PI / 180))),
                    .z = (yRotTri.vert3.pos.z * cos(-cam.root.rotation.x * (PI / 180))) + (yRotTri.vert3.pos.y * sin(-cam.root.rotation.x * (PI / 180)))
                }
            }
        };

        if(xRotTri.vert1.pos.z == -cam.focalLength) { xRotTri.vert1.pos.z = -cam.focalLength + 1; }
        if(xRotTri.vert2.pos.z == -cam.focalLength) { xRotTri.vert2.pos.z = -cam.focalLength + 1; }
        if(xRotTri.vert3.pos.z == -cam.focalLength) { xRotTri.vert3.pos.z = -cam.focalLength + 1; }

        xRotTri.vert1.pos.z = max(xRotTri.vert1.pos.z, cam.nearPlane);
        xRotTri.vert2.pos.z = max(xRotTri.vert2.pos.z, cam.nearPlane);
        xRotTri.vert3.pos.z = max(xRotTri.vert3.pos.z, cam.nearPlane);

        if(max(max(xRotTri.vert1.pos.z, xRotTri.vert2.pos.z), xRotTri.vert3.pos.z) < cam.nearPlane)
        { return; }

        triangle3D projTri = {
            .vert1 = {
                .viewcolor = skyHue,
                .pos = {
                    .x = (xRotTri.vert1.pos.x * cam.focalLength) / (xRotTri.vert1.pos.z),
                    .y = (xRotTri.vert1.pos.y * cam.focalLength) / (xRotTri.vert1.pos.z),
                    .z =  xRotTri.vert1.pos.z
                }
            },
            .vert2 = {
                .viewcolor = skyHue,
                .pos = {
                    .x = (xRotTri.vert2.pos.x * cam.focalLength) / (xRotTri.vert2.pos.z),
                    .y = (xRotTri.vert2.pos.y * cam.focalLength) / (xRotTri.vert2.pos.z),
                    .z =  xRotTri.vert2.pos.z
                }
            },
            .vert3 = {
                .viewcolor = skyHue,
                .pos = {
                    .x = (xRotTri.vert3.pos.x * cam.focalLength) / (xRotTri.vert3.pos.z),
                    .y = (xRotTri.vert3.pos.y * cam.focalLength) / (xRotTri.vert3.pos.z),
                    .z =  xRotTri.vert3.pos.z
                }
            },
            .uv1 = tri.uv1,
            .uv2 = tri.uv2,
            .uv3 = tri.uv3
        };

        SDL_Vertex vert1 = {{projTri.vert1.pos.x + (WIDTH/2), -projTri.vert1.pos.y + (HEIGHT/2)},
                        {255, 255, 255, 255},
                        {tri.uv1.x, 1 - tri.uv1.y} };

        SDL_Vertex vert2 = {{projTri.vert2.pos.x + (WIDTH/2), -projTri.vert2.pos.y + (HEIGHT/2)},
                            {255, 255, 255, 255},
                            {tri.uv2.x, 1 - tri.uv2.y} };

        SDL_Vertex vert3 = {{projTri.vert3.pos.x + (WIDTH/2), -projTri.vert3.pos.y + (HEIGHT/2)},
                            {255, 255, 255, 255},
                            {tri.uv3.x, 1 - tri.uv3.y} };

        SDL_Vertex vertices[] = { vert1, vert2, vert3 };

        SDL_RenderGeometry(renderer, texture, vertices, 3, NULL, 0);
    }
}

void renderQueue(SDL_Renderer *renderer, camera cam, queue *q)
{
    for(int queueIndex = 0; queueIndex < (*q).count; ++queueIndex)
    {
        triangle3D tri = (*q).tris[queueIndex];
        double v1zBuff = 255 * max(0, cam.farPlane - tri.vert1.pos.z)/cam.farPlane;
        double v2zBuff = 255 * max(0, cam.farPlane - tri.vert2.pos.z)/cam.farPlane;
        double v3zBuff = 255 * max(0, cam.farPlane - tri.vert3.pos.z)/cam.farPlane;

        SDL_Vertex vert1 = {{tri.vert1.pos.x + (WIDTH/2), -tri.vert1.pos.y + (HEIGHT/2)},
                            {v1zBuff, v1zBuff, v1zBuff, v1zBuff},
                            {tri.uv1.x, 1 - tri.uv1.y} };

        SDL_Vertex vert2 = {{tri.vert2.pos.x + (WIDTH/2), -tri.vert2.pos.y + (HEIGHT/2)},
                            {v2zBuff, v2zBuff, v2zBuff, v2zBuff},
                            {tri.uv2.x, 1 - tri.uv2.y} };

        SDL_Vertex vert3 = {{tri.vert3.pos.x + (WIDTH/2), -tri.vert3.pos.y + (HEIGHT/2)},
                            {v3zBuff, v3zBuff, v3zBuff, v3zBuff},
                            {tri.uv3.x, 1 - tri.uv3.y} };

        const SDL_Vertex vertices[] = { vert1, vert2, vert3 };

        //int indOfTexture = (*q).texInd[queueIndex];
        //SDL_Texture *texture = q.textures[q.texInd[queueIndex]];

        SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);
    }

    //free((*q).tris);
    //free((*q).texInd);
    //free((*q).textures);

    (*q).tris = realloc((*q).tris, 2 * sizeof(triangle3D));
    (*q).texInd = realloc((*q).texInd, 2 * sizeof(int));
    (*q).textures = realloc((*q).textures, 2 * sizeof(SDL_Texture*));

    (*q).count = 1;
}

void keyboard_inputs(camera *player) 
{
    int player_speed = 20;
    int sensitivity = 2;

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_LSHIFT]) { player_speed = 40; }

    if (keystate[SDL_SCANCODE_W]) 
    {
        player->root.position.z += (player_speed *  2 * cos(player->root.rotation.y * (PI / 180)));
        player->root.position.x += (player_speed *  2 * sin(player->root.rotation.y * (PI / 180)));
    }
    if (keystate[SDL_SCANCODE_S]) 
    {
    	player->root.position.z -= (player_speed *  2 * cos(player->root.rotation.y * (PI / 180)));
    	player->root.position.x -= (player_speed *  2 * sin(player->root.rotation.y * (PI / 180)));
    }
    if (keystate[SDL_SCANCODE_A]) 
    {
    	player->root.position.x -= (player_speed *  2 * cos(player->root.rotation.y * (PI / 180)));
    	player->root.position.z -= (player_speed * -2 * sin(player->root.rotation.y * (PI / 180)));
    }
    if (keystate[SDL_SCANCODE_D]) 
    {
    	player->root.position.x += (player_speed *  2 * cos(player->root.rotation.y * (PI / 180)));
    	player->root.position.z += (player_speed * -2 * sin(player->root.rotation.y * (PI / 180)));
    }

    if (keystate[SDL_SCANCODE_SPACE]) 
    {
    	player->root.position.y += player_speed;
    }
    if (keystate[SDL_SCANCODE_LCTRL]) 
    {
    	player->root.position.y -= player_speed;
    }

    if (keystate[SDL_SCANCODE_UP]) 
    {
        player->root.rotation.x -= sensitivity;
    }
    if (keystate[SDL_SCANCODE_DOWN]) 
    {
    	player->root.rotation.x += sensitivity;
    }
    if (keystate[SDL_SCANCODE_LEFT]) 
    {
    	player->root.rotation.y -= sensitivity;
    }
    if (keystate[SDL_SCANCODE_RIGHT]) 
    {
    	player->root.rotation.y += sensitivity;
    }
}

int main() 
{
    //todo, make the screen draw triangles, give the triangles color, make the triangles 3d, make cube from tris, obj importer
    SDL_Window  *window = NULL;
    SDL_Renderer *renderer = NULL;
    
    window = SDL_CreateWindow("Curse Engine, Software Rasterizer", /* Title of the SDL window */
			  SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
			  SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
			  WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS); /* Additional flag(s) */

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *sky = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./assets/nelunnnh.bmp"));
    object skyBox = importObj("./assets/skyball.obj", renderer);
    skyBox.root = (transform){{0, 0, 0}, {0, 0, 0}};

    camera player = {
        .root = {{-10, 0, 0}, {0, 0, 0}},
        .screen = {800, 600},
        .fov = 110, //tan(fov)=WIDTH/2focalLength//focalLength=WIDTH/2tan(fov)
        .focalLength = player.screen.width/(2*tan((player.fov/2)*(PI/180))),
        .nearPlane = 2,
        .farPlane = 10000
    };

    Scene3D mainScene = CreateScene3D();

    object monke = importObj("./assets/ArtisansHub.obj", renderer);
    monke.root = (transform){{0, 0, 0}, {0, 0, 0}};

    object testCube = {
        .root = {
            {0, 0, 0}, {0, 0, 0}
        },
        .mesh = {
            .tris = calloc(12, sizeof(triangle3D)),
            .texture = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./assets/missing texture.bmp"))
        },
        .data = {
            .faceCount = 12,
            .vertCount = 8,
            .uvCount = 8
        }
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
        keyboard_inputs(&player);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);//printf("lemme guess");

        renderSkyBox(renderer, sky, skyBox, player);
        
        queueObj(player, monke, &mainScene.triQueue);
        //queueObj(player, testCube, &mainScene.triQueue);

        renderQueue(renderer, player, &mainScene.triQueue);
        
        SDL_RenderPresent(renderer);//printf(", or here");
        SDL_UpdateWindowSurface(window);//printf(", or here?");
    }
    free(skyBox.mesh.tris);
    SDL_DestroyTexture(skyBox.mesh.texture);
    free(testCube.mesh.tris);
    SDL_DestroyTexture(testCube.mesh.texture);
    free(monke.mesh.tris);
    SDL_DestroyTexture(monke.mesh.texture);

    free(mainScene.triQueue.tris);
    free(mainScene.triQueue.texInd);
    free(mainScene.triQueue.textures);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
