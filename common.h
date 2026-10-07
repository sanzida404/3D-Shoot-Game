#pragma once
/*SEU Corridor Explorer v10 - GLUT/OpenGL
 Environment only: classroom/corridor/washroom/elevator.
 Fixes:
 - Each classroom is a real enclosed room with its own side/back walls.
 - Corridor front walls are split exactly around door openings.
 - Door frames are flush with the wall openings; no visible gaps/crossing.
 - Closed classroom/washroom doors collide; open doors permit entry.
 - Elevators are decorative/closed and physically blocked: they cannot be entered.
 - Grey rectangular tile floor follows the supplied reference photo: large,
   cool-grey, slightly mottled tiles with visible grout and staggered joints.
 - Clean one-floor ceiling/upper-wall geometry; no floating upper-floor slabs.
 - Washroom has a complete enclosing shell and ceiling-height walls.
 Build Linux:
   gcc seu_corridor_v9.c -o seu_corridor_v9 -lglut -lGLU -lGL -lm
*/

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#define WIN_W 1100
#define WIN_H 720
#define PI 3.14159265358979323846f
#define MAX_DOORS 64
#define TEXSZ 256

static float camX=0.0f, camY=1.7f, camZ=10.0f;
static float yaw=-90.0f, pitch=0.0f;
static int keyState[256]={0}, specialState[256]={0};
static int warpingPointer=0;
static const float PLAYER_R=0.34f;

/* ---- combat / gun state (added) ---- */
struct Tracer { float x1,y1,z1,x2,y2,z2; float age; };
static std::vector<Tracer> tracers;
static const float TRACER_LIFE = 0.08f;

static int   playerHealth = 100;
static int   killCount = 0;
static int   shotsFired = 0;
static bool  gameOver = false;
static float playerDamageCooldown = 0.0f;
static float muzzleFlashTimer = 0.0f;
static int   lastShotMs = -10000;
static float gunBobPhase = 0.0f;

/* ---- multi-floor lift progression (added) ----
   Floor 1 needs 3 kills, floor 2 needs 4 kills, floor 3 needs 5 kills.
   The story lift is the door added as "Elevator A (Closed)" in
   setupDoors(); it is always the 13th door added (index 12), since the
   door list order never changes. The second elevator stays permanently
   closed/decorative, exactly as before. */
#define LIFT_DOOR_INDEX 12
static const int FLOOR_KILL_REQUIREMENT[3] = {3,4,5};
static int   currentFloor  = 1;     /* 1, 2 or 3 */
/* liftState: 0=locked (kill quota not met yet), 1=unlocked & closed
   (click to open), 2=open (click to close), 3=closed with the shooter
   inside, ready to travel (click to go to the next floor). */
static int   liftState     = 0;
static float liftDoorOpen  = 0.0f;  /* 0..1 visual slide-open amount */
static bool  gameWon       = false;
static float floorMsgTimer = 0.0f;  /* brief "Floor N" banner after a ride */

typedef struct {
    float x,y,z;
    char label[48];
    float rotY;
    int type;       /* 0 classroom, 1 elevator, 2 washroom, 3 washroom cubicle */
    float angle;
} Door;

static Door doors[MAX_DOORS];
static int doorCount=0, nearestDoor=-1;
static int doorTarget[MAX_DOORS]={0};
static float doorAngle[MAX_DOORS]={0};

typedef struct { float minx,maxx,minz,maxz; } Wall;
static Wall walls[128];
static int wallCount=0;

typedef struct { float minx,maxx,minz,maxz; } Obstacle;
static Obstacle obstacles[256];
static int obstacleCount=0;

static GLubyte texBuf[TEXSZ*TEXSZ*3];
static GLuint tileTex, woodTex, metalTex, wallTex;
static GLuint sceneTex; /* ADDED: outside-view texture for the window (road/trees/car) */
static float  fanAngle = 0.0f; /* ADDED: continuously rotating ceiling fans */

/* ------------------------------------------------------------ */
/* Basic helpers                                                */
/* ------------------------------------------------------------ */

static void addWall(float minx,float maxx,float minz,float maxz){
    if(wallCount>=128)return;
    walls[wallCount].minx=minx;
    walls[wallCount].maxx=maxx;
    walls[wallCount].minz=minz;
    walls[wallCount].maxz=maxz;
    wallCount++;
}

static void addObstacle(float minx,float maxx,float minz,float maxz){
    if(obstacleCount>=256)return;
    obstacles[obstacleCount].minx=minx;
    obstacles[obstacleCount].maxx=maxx;
    obstacles[obstacleCount].minz=minz;
    obstacles[obstacleCount].maxz=maxz;
    obstacleCount++;
}

static int pointInRect(float x,float z,float minx,float maxx,float minz,float maxz){
    return x>minx && x<maxx && z>minz && z<maxz;
}

static int collidesStatic(float x,float z){
    int i;
    for(i=0;i<wallCount;i++){
        if(x+PLAYER_R>walls[i].minx && x-PLAYER_R<walls[i].maxx &&
           z+PLAYER_R>walls[i].minz && z-PLAYER_R<walls[i].maxz)
            return 1;
    }
    return 0;
}

/* Door collision follows the actual hinged leaf.  A closed door blocks the
   doorway, while an open door occupies its real swung position.  This keeps
   the player from walking through the physical door leaf instead of passing
   through the doorway opening. */
static int collidesDoors(float x,float z){
    int i;
    for(i=0;i<doorCount;i++){
        Door *d=&doors[i];

        if(d->type==1){
            /* The story lift stops blocking movement only while its doors
               are open, so the shooter can actually step inside/out. Every
               other state (locked, unlocked-but-closed, closed-with-rider)
               keeps it sealed, same as the decorative second elevator. */
            if(i==LIFT_DOOR_INDEX && liftState==2)
                continue;
            if(pointInRect(x,z,d->x-1.18f,d->x+1.18f,d->z-.34f,d->z+.34f))
                return 1;
            continue;
        }

        /* Washroom cubicle doors use the same real hinged-leaf idea as the
           main doors, but with a 2.04m leaf and a left-side hinge. */
        if(d->type==3){
            float a=d->angle*PI/180.0f;
            float ca=cosf(a), sa=sinf(a);
            float hx=d->x-1.02f, hz=d->z;
            float ux=ca, uz=-sa;
            float vx=sa, vz=ca;
            float px=x-hx, pz=z-hz;
            float along=px*ux+pz*uz;
            float across=px*vx+pz*vz;
            float halfW=.04f+PLAYER_R;

            if(along>-PLAYER_R && along<2.04f+PLAYER_R && fabsf(across)<halfW)
                return 1;

            /* Keep the doorway completely sealed while the door is closed. */
            if(fabsf(d->angle)<8.0f &&
               pointInRect(x,z,d->x-1.02f,d->x+1.02f,d->z-.10f,d->z+.10f))
                return 1;
            continue;
        }

        /* Door leaf in its own local coordinates:
           hinge = (-1.08, 0.03), leaf length = 2.16, thickness = .11. */
        float a=(d->rotY + d->angle)*PI/180.0f;
        float ca=cosf(a), sa=sinf(a);

        /* Transform the hinge point by the fixed doorway rotation. */
        float br=(d->rotY)*PI/180.0f;
        float cbr=cosf(br), sbr=sinf(br);
        float hx=d->x + cbr*(-1.08f) + sbr*(.03f);
        float hz=d->z - sbr*(-1.08f) + cbr*(.03f);

        /* Player-circle vs. door rectangle: use a small set of segments
           along the leaf, including its two long edges and end edge. */
        float ux=cosf(a), uz=-sinf(a);
        float vx=sinf(a), vz=cosf(a);
        float halfW=.055f + PLAYER_R;
        float halfL=1.08f;
        float px=x-hx, pz=z-hz;
        float along=px*ux+pz*uz;
        float across=px*vx+pz*vz;

        if(along>-PLAYER_R && along<2.16f+PLAYER_R && fabsf(across)<halfW)
            return 1;

        /* Closed-door collision remains slightly wider so the doorway is
           completely sealed at small animation angles. */
        if(fabsf(d->angle)<8.0f &&
           pointInRect(x,z,d->x-1.02f,d->x+1.02f,d->z-.34f,d->z+.34f))
            return 1;
    }
    return 0;
}

static int collidesFurniture(float x,float z){
    int i;
    for(i=0;i<obstacleCount;i++){
        if(x+PLAYER_R>obstacles[i].minx && x-PLAYER_R<obstacles[i].maxx &&
           z+PLAYER_R>obstacles[i].minz && z-PLAYER_R<obstacles[i].maxz)
            return 1;
    }
    return 0;
}

static int collides(float x,float z){
    return collidesStatic(x,z) || collidesDoors(x,z) || collidesFurniture(x,z);
}

static void addDoor(float x,float y,float z,float rot,const char *label,int type){
    if(doorCount>=MAX_DOORS)return;
    Door *d=&doors[doorCount++];
    d->x=x; d->y=y; d->z=z; d->rotY=rot; d->type=type; d->angle=0;
    strncpy(d->label,label,47); d->label[47]='\0';
}