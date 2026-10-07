#pragma once
#include "common.h"


/* ------------------------------------------------------------ */
/* SEU zombie NPCs: supplied character only                     */
/* Human-sized, collision-aware, no zombie ground/pool.         */
/* ------------------------------------------------------------ */

static GLUquadric *zombieQuad=NULL;
static float zombieAnimTime=0.0f;
static float zombieCurrentPhase=0.0f;

typedef struct {
    float x,z;
    int zone;
    int pathIndex;
    float phase;
    int health;       /* hits left before it falls (added) */
    int state;        /* 0 alive, 1 dying, 2 dead/respawning (added) */
    float stateTimer; /* seconds spent in the current state (added) */
} SEUZombieNPC;

/*
   zone:
     0 = Room 101
     1 = Room 105
     2 = Room 108
     3 = left border/corridor
     4 = right border/corridor
     5 = washroom
*/
static SEUZombieNPC seuZombies[6]={
    {-19.20f,16.20f,0,1,0.30f},
    {-19.00f,34.40f,1,0,1.40f},
    { 13.00f,34.40f,2,0,2.60f},
    {-21.50f, 6.00f,3,1,3.70f},
    { 21.50f, 6.00f,4,2,4.80f},
    {  0.00f,42.20f,5,1,5.90f}
};
static const int seuZombieCount=6;

/* Aggro chase (added): once the shooter gets this close, the zombie
   drops its patrol route and sprints straight at the shooter instead. */
static const float ZOMBIE_AGGRO_RADIUS = 6.5f;
static const float ZOMBIE_CHASE_STEP   = 0.030f;

static void drawSEUZombieCharacter(void);

static int zombieRectHit(float x,float z,float radius,
                         float minx,float maxx,float minz,float maxz){
    return (x+radius>minx && x-radius<maxx &&
            z+radius>minz && z-radius<maxz);
}

/*
   Exact furniture blockers for the objects actually drawn in the
   classrooms and washroom. These are zombie-only blockers, so the
   existing player collision behavior is not changed.
*/
static int zombieFurnitureBlocked(float x,float z,float radius){
    int i,r,c;

    /* All classroom furniture + whiteboards. */
    {
        float centers[4]={-16.0f,-6.0f,6.0f,16.0f};
        for(i=0;i<4;i++){
            float cx=centers[i];
            float cz=12.5f;
            float boardZ=-4.35f;

            if(zombieRectHit(x,z,radius,
                             cx-2.95f,cx+2.95f,
                             cz+boardZ-.08f,cz+boardZ+.08f))
                return 1;

            /* Teacher desk. */
            if(zombieRectHit(x,z,radius,
                             cx-3.15f,cx-2.15f,
                             cz-3.60f,cz-2.90f))
                return 1;

            for(r=0;r<3;r++)for(c=0;c<3;c++){
                float fx=cx-1.8f+c*1.8f;
                float fz=cz-2.2f+r*1.85f;

                /* Same footprint for south-room furniture after the
                   180-degree facing rotation. */
                if(zombieRectHit(x,z,radius,
                                 fx-.55f,fx+.55f,
                                 fz-.42f,fz+.42f))
                    return 1;
            }
        }
    }

    /* North classrooms: same furniture layout, opposite teaching wall. */
    {
        float centers[4]={-16.0f,-6.0f,6.0f,16.0f};
        for(i=0;i<4;i++){
            float cx=centers[i];
            float cz=33.0f;
            float boardZ=4.35f;

            if(zombieRectHit(x,z,radius,
                             cx-2.95f,cx+2.95f,
                             cz+boardZ-.08f,cz+boardZ+.08f))
                return 1;

            if(zombieRectHit(x,z,radius,
                             cx-3.15f,cx-2.15f,
                             cz+2.90f,cz+3.60f))
                return 1;

            for(r=0;r<3;r++)for(c=0;c<3;c++){
                float fx=cx-1.8f+c*1.8f;
                float fz=cz+2.2f-r*1.85f;

                if(zombieRectHit(x,z,radius,
                                 fx-.55f,fx+.55f,
                                 fz-.42f,fz+.42f))
                    return 1;
            }
        }
    }

    /* Washroom cubicle partitions and high commodes. */
    {
        float stallX[3]={0.10f,2.50f,4.90f};
        float cz=45.0f;

        for(i=0;i<3;i++){
            float sx=stallX[i];

            if(zombieRectHit(x,z,radius,
                             sx-1.10f,sx-1.00f,
                             cz+1.18f-2.10f,cz+1.18f+2.10f))
                return 1;

            if(zombieRectHit(x,z,radius,
                             sx+1.00f,sx+1.10f,
                             cz+1.18f-2.10f,cz+1.18f+2.10f))
                return 1;

            if(zombieRectHit(x,z,radius,
                             sx-.50f,sx+.50f,
                             cz+1.45f-.62f,cz+1.45f+.62f))
                return 1;
        }

        /* Right-side wash basins/counter. */
        if(zombieRectHit(x,z,radius,
                         6.22f,7.30f,
                         cz-2.10f-.95f,cz-2.10f+.95f))
            return 1;
    }

    return 0;
}

static int zombieBlocked(float x,float z){
    const float ZR=.22f; /* human-sized zombie body radius */

    /* Building walls and all existing solid obstacles. */
    if(collidesStatic(x,z))
        return 1;

    if(collidesFurniture(x,z))
        return 1;

    /* Existing animated doors, including the 3 toilet doors. */
    if(collidesDoors(x,z))
        return 1;

    /* Exact classroom/washroom furniture blockers. */
    if(zombieFurnitureBlocked(x,z,ZR))
        return 1;

    /* Hard outer border safety. */
    if(x< -23.20f+ZR || x>23.20f-ZR ||
       z<   4.20f+ZR || z>51.40f-ZR)
        return 1;

    return 0;
}

/* ADDED: hard per-room safety box for the three classroom zombies (Room
   101/105/108). This is an authoritative rectangle that sits safely
   inside each room's own walls, so those zombies can never be seen
   sliding through a classroom wall -- even during the aggro chase,
   which previously used only the general wall/door checks. Zones with
   no room (corridor/washroom zombies) are left completely unaffected. */
static int zombieRoomBounds(int zone,float *minx,float *maxx,float *minz,float *maxz){
    switch(zone){
        case 0: *minx=-19.45f; *maxx=-12.55f; *minz= 8.10f; *maxz=17.60f; return 1; /* Room 101 */
        case 1: *minx=-19.45f; *maxx=-12.55f; *minz=28.40f; *maxz=37.75f; return 1; /* Room 105 */
        case 2: *minx= 12.55f; *maxx= 19.45f; *minz=28.40f; *maxz=37.75f; return 1; /* Room 108 */
        case 5: *minx= -2.45f; *maxx=  6.95f; *minz=41.40f; *maxz=48.45f; return 1; /* Washroom */
        default: return 0;
    }
}

static int zombieBlockedForZone(int zone,float x,float z){
    if(zombieBlocked(x,z)) return 1;
    float minx,maxx,minz,maxz;
    if(zombieRoomBounds(zone,&minx,&maxx,&minz,&maxz)){
        if(x<minx || x>maxx || z<minz || z>maxz) return 1;
    }
    return 0;
}

/* Safe patrol targets. They intentionally run through aisles/open space,
   never through chairs, boards, commodes, partitions, or walls. */
static void zombieTargetFor(const SEUZombieNPC *z,float *tx,float *tz){
    static const float southX[4]={-3.20f,3.20f,3.20f,-3.20f};
    static const float southZ[4]={ 3.70f,3.70f,-1.30f,-1.30f};

    static const float northX[4]={-3.20f,3.20f,3.20f,-3.20f};
    static const float northZ[4]={ 4.00f,4.00f, 1.25f, 1.25f};

    int p=z->pathIndex%4;

    if(z->zone==0){             /* Room 101 */
        *tx=-16.0f+southX[p];
        *tz=12.5f+southZ[p];
    }else if(z->zone==1){       /* Room 105: stay inside the room */
        static const float roomX[4]={-3.00f, 3.00f, 3.00f,-3.00f};
        static const float roomZ[4]={ 1.40f, 1.40f,-2.70f,-2.70f};
        *tx=-16.0f+roomX[p];
        *tz= 33.0f+roomZ[p];
    }else if(z->zone==2){       /* Room 108: stay inside the room */
        static const float roomX[4]={-3.00f, 3.00f, 3.00f,-3.00f};
        static const float roomZ[4]={ 1.40f, 1.40f,-2.70f,-2.70f};
        *tx= 16.0f+roomX[p];
        *tz= 33.0f+roomZ[p];
    }else if(z->zone==3){       /* left border */
        static const float zz[4]={6.0f,17.0f,29.0f,40.5f};
        *tx=-21.50f;
        *tz=zz[p];
    }else if(z->zone==4){       /* right border */
        static const float zz[4]={6.0f,17.0f,29.0f,40.5f};
        *tx=21.50f;
        *tz=zz[p];
    }else{                      /* washroom: open area in front of stalls */
        static const float xx[4]={0.00f,5.35f,5.35f,0.00f};
        static const float zz[4]={42.20f,42.20f,43.35f,43.35f};
        *tx=xx[p];
        *tz=zz[p];
    }
}

static void updateSEUZombies(void){
    int i;

    for(i=0;i<seuZombieCount;i++){
        SEUZombieNPC *z=&seuZombies[i];

        if(z->state==1){          /* dying: play the fall, then wait dead */
            z->stateTimer+=0.016f;
            if(z->stateTimer>1.0f){ z->state=2; z->stateTimer=0.0f; }
            continue;
        }
        if(z->state==2){          /* dead: wait, then respawn back to life */
            z->stateTimer+=0.016f;
            if(z->stateTimer>6.0f){
                z->state=0;
                z->health=3;
                z->stateTimer=0.0f;
            }
            continue;
        }

        float tx,tz;

        /* Aggro check: the zombie "notices" the shooter and charges in,
           overriding its normal patrol until the shooter is out of range
           again. */
        float pdx=camX-z->x, pdz=camZ-z->z;
        float pdist=sqrtf(pdx*pdx+pdz*pdz);
        if(pdist<ZOMBIE_AGGRO_RADIUS && pdist>0.001f){
            float mx=pdx/pdist*ZOMBIE_CHASE_STEP;
            float mz=pdz/pdist*ZOMBIE_CHASE_STEP;
            if(!zombieBlockedForZone(z->zone,z->x+mx,z->z)) z->x+=mx;
            if(!zombieBlockedForZone(z->zone,z->x,z->z+mz)) z->z+=mz;
            continue;
        }

        zombieTargetFor(z,&tx,&tz);

        float dx=tx-z->x;
        float dz=tz-z->z;
        float dist=sqrtf(dx*dx+dz*dz);

        if(dist<0.18f){
            z->pathIndex=(z->pathIndex+1)%4;
            zombieTargetFor(z,&tx,&tz);
            dx=tx-z->x;
            dz=tz-z->z;
            dist=sqrtf(dx*dx+dz*dz);
        }

        if(dist>.001f){
            const float step=.0125f;

            /* Room 105 and Room 108 use simple axis-by-axis patrols.
               This prevents diagonal corner-cutting and keeps the zombie
               naturally inside its own classroom. */
            if(z->zone==1 || z->zone==2){
                if(fabsf(dx)>0.08f){
                    float mx=(dx>0.0f)?step:-step;
                    if(!zombieBlockedForZone(z->zone,z->x+mx,z->z))
                        z->x+=mx;
                    else
                        z->pathIndex=(z->pathIndex+1)%4;
                }else if(fabsf(dz)>0.08f){
                    float mz=(dz>0.0f)?step:-step;
                    if(!zombieBlockedForZone(z->zone,z->x,z->z+mz))
                        z->z+=mz;
                    else
                        z->pathIndex=(z->pathIndex+1)%4;
                }else{
                    z->pathIndex=(z->pathIndex+1)%4;
                }
                continue;
            }

            /* Original movement behavior for every other zombie remains
               unchanged. */
            {
                float mx=dx/dist*step;
                float mz=dz/dist*step;

                if(!zombieBlockedForZone(z->zone,z->x+mx,z->z))
                    z->x+=mx;
                else
                    z->pathIndex=(z->pathIndex+1)%4;

                if(!zombieBlockedForZone(z->zone,z->x,z->z+mz))
                    z->z+=mz;
            }
        }
    }
}

static void drawSEUZombies(void){
    int i;

    for(i=0;i<seuZombieCount;i++){
        SEUZombieNPC *z=&seuZombies[i];

        if(z->state==2) continue;   /* dead, waiting to respawn: hidden */

        float tx,tz;
        float pdx=camX-z->x, pdz=camZ-z->z;
        float pdist=sqrtf(pdx*pdx+pdz*pdz);
        if(z->state==0 && pdist<ZOMBIE_AGGRO_RADIUS && pdist>0.001f){
            tx=camX; tz=camZ;   /* chasing: face straight at the shooter */
        }else{
            zombieTargetFor(z,&tx,&tz);
        }

        float dx=tx-z->x;
        float dz=tz-z->z;
        float heading=atan2f(dx,dz)*180.0f/PI;

        zombieCurrentPhase=z->phase;

        glPushMatrix();

        /* ~1.8m human scale from the supplied zombie model. */
        if(z->state==1){
            /* dying: topple backward and sink slightly */
            glTranslatef(z->x,.58f-z->stateTimer*0.3f,z->z);
            glRotatef(heading,0,1,0);
            glRotatef(z->stateTimer*90.0f,1,0,0);
        }else{
            glTranslatef(z->x,.58f,z->z);
            glRotatef(heading,0,1,0);
        }
        glScalef(.32f,.32f,.32f);

        drawSEUZombieCharacter();

        glPopMatrix();
    }
}


//  Colors  (used with GL_COLOR_MATERIAL, so glColor3f drives lighting)
// --------------------------------------------------------------------
struct ZombieColor { float r, g, b; };

static const ZombieColor SKIN      = {0.33f, 0.42f, 0.20f};
static const ZombieColor SKIN_DARK = {0.20f, 0.28f, 0.12f};
static const ZombieColor SHIRT     = {0.17f, 0.33f, 0.33f};
static const ZombieColor PANTS     = {0.13f, 0.13f, 0.19f};
static const ZombieColor BOOT      = {0.07f, 0.06f, 0.05f};
static const ZombieColor BLOOD     = {0.40f, 0.02f, 0.02f};
static const ZombieColor BLOOD_DK  = {0.22f, 0.01f, 0.01f};
static const ZombieColor EYE_GLOW  = {1.00f, 0.75f, 0.10f};
static const ZombieColor TEETH     = {0.82f, 0.78f, 0.62f};
static const ZombieColor CLAW      = {0.07f, 0.07f, 0.05f};

static void zombieSetColor(const ZombieColor &c) { glColor3f(c.r, c.g, c.b); }

// --------------------------------------------------------------------
//  Hardcoded blood-blob positions (deterministic � no rand() jitter)
//  Each entry: x, y, z, radius   (local space of the body part)
// --------------------------------------------------------------------

static const float headBlood[][4] = {
    { 0.35f,  0.10f,  0.55f, 0.07f},
    {-0.20f, -0.25f,  0.58f, 0.06f},
    { 0.05f, -0.40f,  0.55f, 0.05f},
    {-0.45f,  0.30f,  0.40f, 0.05f},
    { 0.50f, -0.10f,  0.30f, 0.06f},
    {-0.10f,  0.55f,  0.35f, 0.04f}
};
static const int NUM_HEAD_BLOOD = 6;

static const float torsoBlood[][4] = {
    {-0.45f,  0.55f,  0.44f, 0.10f},
    { 0.30f,  0.35f,  0.45f, 0.08f},
    { 0.10f, -0.10f,  0.46f, 0.11f},
    {-0.20f, -0.35f,  0.44f, 0.09f},
    { 0.40f, -0.55f,  0.42f, 0.12f},
    {-0.10f, -0.70f,  0.40f, 0.10f},
    { 0.15f, -0.85f,  0.38f, 0.13f},
    {-0.35f, -0.80f,  0.36f, 0.10f},
    { 0.00f,  0.10f, -0.44f, 0.08f},
    {-0.30f, -0.40f, -0.42f, 0.09f}
};
static const int NUM_TORSO_BLOOD = 10;

static const float limbBlood[][4] = {
    { 0.05f, -0.20f,  0.15f, 0.06f},
    {-0.08f, -0.55f,  0.14f, 0.05f},
    { 0.10f, -0.85f,  0.13f, 0.05f}
};
static const int NUM_LIMB_BLOOD = 3;

static void zombieDrawBloodBlobs(const float arr[][4], int count) {
    zombieSetColor(BLOOD);
    for (int i = 0; i < count; ++i) {
        glPushMatrix();
        glTranslatef(arr[i][0], arr[i][1], arr[i][2]);
        glutSolidSphere(arr[i][3], 8, 6);
        glPopMatrix();
    }
}

// --------------------------------------------------------------------
//  Small geometry helpers
// --------------------------------------------------------------------

// A cylinder-based "limb segment" running along -Y, from y=0 to y=-length
static void zombieDrawLimbSegment(float radiusTop, float radiusBottom, float length) {
    glPushMatrix();
    glRotatef(-90.0f, 1, 0, 0);      // gluCylinder extends along +Z by default
    gluCylinder(zombieQuad, radiusTop, radiusBottom, length, 10, 4);
    // cap the top so it doesn't look hollow
    gluDisk(zombieQuad, 0, radiusTop, 10, 2);
    glPopMatrix();
}

static void zombieDrawClawFinger(float spreadX, float droop) {
    glPushMatrix();
    glTranslatef(spreadX, -0.05f, 0.02f);
    glRotatef(droop, 1, 0, 0);

    zombieSetColor(SKIN);
    glPushMatrix();
    glScalef(0.05f, 0.28f, 0.05f);
    glTranslatef(0, -0.5f, 0);
    glutSolidCube(1.0f);
    glPopMatrix();

    // claw tip
    glTranslatef(0, -0.30f, 0);
    zombieSetColor(CLAW);
    glRotatef(90, 1, 0, 0);
    glutSolidCone(0.035f, 0.12f, 8, 2);
    glPopMatrix();
}

static void zombieDrawHand() {
    // palm
    zombieSetColor(SKIN);
    glPushMatrix();
    glScalef(0.26f, 0.16f, 0.14f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // four clawed fingers, spread out
    zombieDrawClawFinger(-0.16f, 60.0f);
    zombieDrawClawFinger(-0.05f, 68.0f);
    zombieDrawClawFinger( 0.06f, 68.0f);
    zombieDrawClawFinger( 0.17f, 60.0f);

    // thumb
    glPushMatrix();
    glTranslatef(-0.18f, 0.02f, 0.05f);
    glRotatef(80.0f, 0, 0, 1);
    zombieSetColor(SKIN);
    glScalef(0.045f, 0.20f, 0.045f);
    glTranslatef(0, -0.5f, 0);
    glutSolidCube(1.0f);
    glPopMatrix();

    zombieDrawBloodBlobs(limbBlood, NUM_LIMB_BLOOD > 1 ? 1 : 0); // tiny fleck on hand
}

// --------------------------------------------------------------------
//  Body parts
// --------------------------------------------------------------------

static void zombieDrawHead() {
    glPushMatrix();

    // main blocky skull
    zombieSetColor(SKIN);
    glPushMatrix();
    glScalef(1.5f, 1.55f, 1.25f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // deep brow ridge (darker box across the top-front)
    zombieSetColor(SKIN_DARK);
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.60f);
    glScalef(1.3f, 0.18f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // eye sockets (dark hollows)
    zombieSetColor(SKIN_DARK);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(0.33f * s, 0.08f, 0.62f);
        glScalef(0.34f, 0.24f, 0.10f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // glowing eyes (emissive so they read as "dangerous" even lit dim)
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(0.33f * s, 0.08f, 0.68f);
        GLfloat emission[] = { EYE_GLOW.r, EYE_GLOW.g * 0.4f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, emission);
        zombieSetColor(EYE_GLOW);
        glutSolidSphere(0.09f, 10, 8);
        GLfloat noEmission[] = { 0,0,0,1 };
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
        glPopMatrix();
    }

    // snarling mouth (dark cavity)
    zombieSetColor(BLOOD_DK);
    glPushMatrix();
    glTranslatef(0.0f, -0.35f, 0.60f);
    glScalef(0.75f, 0.22f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // jagged teeth
    zombieSetColor(TEETH);
    for (int i = -3; i <= 3; ++i) {
        glPushMatrix();
        glTranslatef(i * 0.10f, -0.27f, 0.66f);
        glScalef(0.06f, 0.10f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // blood dripping from mouth
    zombieSetColor(BLOOD);
    for (int i = -1; i <= 1; ++i) {
        glPushMatrix();
        glTranslatef(i * 0.18f, -0.55f - 0.05f * i, 0.60f);
        glScalef(0.05f, 0.35f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    zombieDrawBloodBlobs(headBlood, NUM_HEAD_BLOOD);

    glPopMatrix();
}

static void zombieDrawTorso() {
    glPushMatrix();
    zombieSetColor(SHIRT);
    glPushMatrix();
    glScalef(1.55f, 1.9f, 0.85f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // torn jagged hem strips at the bottom
    zombieSetColor(SHIRT);
    for (int i = -2; i <= 2; ++i) {
        glPushMatrix();
        glTranslatef(i * 0.3f, -0.95f - (i % 2 == 0 ? 0.10f : 0.0f), 0.40f);
        glScalef(0.22f, 0.35f, 0.15f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    zombieDrawBloodBlobs(torsoBlood, NUM_TORSO_BLOOD);
    glPopMatrix();
}

static void zombieDrawHips() {
    zombieSetColor(PANTS);
    glPushMatrix();
    glScalef(1.45f, 0.5f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// isLeft flips claw spread slightly; pose (rotation) is applied by caller
static void zombieDrawArm(bool isLeft) {
    glPushMatrix();

    // upper arm
    zombieSetColor(SKIN);
    zombieDrawLimbSegment(0.20f, 0.17f, 0.85f);

    // forearm, attached at end of upper arm
    glTranslatef(0.0f, -0.85f, 0.0f);
    zombieSetColor(SKIN);
    zombieDrawLimbSegment(0.15f, 0.12f, 0.75f);

    // hand at end of forearm
    glTranslatef(0.0f, -0.78f, 0.0f);
    zombieDrawHand();

    zombieDrawBloodBlobs(limbBlood, NUM_LIMB_BLOOD);

    glPopMatrix();
}

static void zombieDrawLeg(bool isLeft) {
    glPushMatrix();

    // thigh
    zombieSetColor(PANTS);
    zombieDrawLimbSegment(0.26f, 0.22f, 1.0f);

    // shin
    glTranslatef(0.0f, -1.0f, 0.0f);
    zombieSetColor(PANTS);
    zombieDrawLimbSegment(0.20f, 0.16f, 0.9f);

    // boot
    glTranslatef(0.0f, -0.95f, 0.10f);
    zombieSetColor(BOOT);
    glPushMatrix();
    glScalef(0.32f, 0.22f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    zombieDrawBloodBlobs(limbBlood, NUM_LIMB_BLOOD > 2 ? 2 : NUM_LIMB_BLOOD);

    glPopMatrix();
}

// --------------------------------------------------------------------
//  Full zombie, posed like a lurching, reaching attacker
// --------------------------------------------------------------------
static void drawSEUZombieCharacter() {
    glPushMatrix();

    float bob = sinf(zombieAnimTime * 1.3f) * 0.03f;   // idle breathing/bob
    glTranslatef(0.0f, bob, 0.0f);

    // --- hips / pelvis (root of the hierarchy) ---
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.0f);
    zombieDrawHips();

    // left leg (bent forward, lurching stride)
    glPushMatrix();
    glTranslatef(-0.35f, -0.25f, 0.0f);
    glRotatef(22.0f + sinf(zombieAnimTime*5.0f + zombieCurrentPhase)*10.0f, 1, 0, 0);
    zombieDrawLeg(true);
    glPopMatrix();

    // right leg (planted back)
    glPushMatrix();
    glTranslatef(0.35f, -0.25f, 0.0f);
    glRotatef(-15.0f - sinf(zombieAnimTime*5.0f + zombieCurrentPhase)*10.0f, 1, 0, 0);
    zombieDrawLeg(false);
    glPopMatrix();
    glPopMatrix(); // end hips

    // --- torso ---
    glPushMatrix();
    glTranslatef(0.0f, 1.55f, 0.0f);
    glRotatef(5.0f, 0, 1, 0);
    zombieDrawTorso();

    // neck
    zombieSetColor(SKIN);
    glPushMatrix();
    glTranslatef(0.0f, 1.15f, 0.0f);
    zombieDrawLimbSegment(0.28f, 0.32f, 0.35f);
    glPopMatrix();

    // head (tilted forward, snarling toward viewer)
    glPushMatrix();
    glTranslatef(0.0f, 1.55f, 0.05f);
    glRotatef(-6.0f, 0, 1, 0);
    glRotatef(10.0f, 1, 0, 0);
    zombieDrawHead();
    glPopMatrix();

    // left arm: raised high and clawing outward (matches reference pose)
    glPushMatrix();
    glTranslatef(-1.02f, 0.60f, 0.0f);
    glRotatef(-95.0f + sinf(zombieAnimTime * 0.9f) * 4.0f, 0, 0, 1);
    glRotatef(-20.0f, 1, 0, 0);
    zombieDrawArm(true);
    glPopMatrix();

    // right arm: bent, reaching forward toward the viewer
    glPushMatrix();
    glTranslatef(1.02f, 0.60f, 0.0f);
    glRotatef(60.0f + sinf(zombieAnimTime * 1.1f + 1.0f) * 4.0f, 0, 0, 1);
    glRotatef(30.0f, 1, 0, 0);
    zombieDrawArm(false);
    glPopMatrix();

    glPopMatrix(); // end torso

    glPopMatrix(); // end whole zombie
}

// --------------------------------------------------------------------

/* ------------------------------------------------------------ */
/* Gun viewmodel + shooting (added)                             */
/* ------------------------------------------------------------ */

static void gunBox(float w,float h,float d,float r,float g,float b){
    glColor3f(r,g,b);
    glPushMatrix();
    glScalef(w,h,d);
    glutSolidCube(1.0f);
    glPopMatrix();
}

/* Drawn in camera-local space (right after glLoadIdentity, still inside
   the gluLookAt matrix stack): -Z is forward, +X is right, +Y is up. */
static void drawViewmodelGun(float bobX,float bobY){
    glPushMatrix();
    glTranslatef(0.32f+bobX, -0.28f+bobY, -0.55f);
    glRotatef(-8.0f,0,1,0);

    glPushMatrix();
    glTranslatef(0.0f,0.02f,-0.22f);
    gunBox(0.05f,0.05f,0.28f, 0.85f,0.68f,0.15f);   /* barrel */
    glPopMatrix();

    gunBox(0.09f,0.11f,0.24f, 0.08f,0.08f,0.08f);   /* receiver */

    glPushMatrix();
    glTranslatef(0.0f,-0.12f,0.07f);
    glRotatef(20.0f,1,0,0);
    gunBox(0.06f,0.14f,0.07f, 0.05f,0.05f,0.05f);   /* grip */
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f,-0.01f,0.16f);
    gunBox(0.07f,0.08f,0.16f, 0.15f,0.15f,0.15f);   /* stock */
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f,0.075f,-0.05f);
    gunBox(0.03f,0.05f,0.05f, 0.03f,0.03f,0.03f);   /* sight */
    glPopMatrix();

    if(muzzleFlashTimer>0.0f){
        glPushMatrix();
        glTranslatef(0.0f,0.02f,-0.38f);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f,0.9f,0.5f);
        glutSolidSphere(0.05f*(muzzleFlashTimer/0.08f),8,8);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    glPopMatrix();
}

/* Hitscan shot: a ray from the camera along the view direction, tested
   against a bounding sphere around each living zombie. */
static void tryShoot(void){
    if(gameOver||gameWon)return;

    int now=glutGet(GLUT_ELAPSED_TIME);
    if(now-lastShotMs<150)return;
    lastShotMs=now;
    shotsFired++;
    muzzleFlashTimer=0.08f;

    float ry=yaw*PI/180.0f;
    float rp=pitch*PI/180.0f;
    float dx=cosf(rp)*cosf(ry);
    float dy=sinf(rp);
    float dz=cosf(rp)*sinf(ry);

    float bestT=60.0f;
    int hitIdx=-1;
    int i;

    for(i=0;i<seuZombieCount;i++){
        SEUZombieNPC *z=&seuZombies[i];
        if(z->state!=0)continue;

        float cx=z->x, cy=1.0f, cz=z->z, radius=0.55f;
        float ocx=camX-cx, ocy=camY-cy, ocz=camZ-cz;
        float b=ocx*dx+ocy*dy+ocz*dz;
        float c=ocx*ocx+ocy*ocy+ocz*ocz-radius*radius;
        float disc=b*b-c;
        if(disc<0.0f)continue;

        float t=-b-sqrtf(disc);
        if(t>0.0f && t<bestT){
            bestT=t;
            hitIdx=i;
        }
    }

    Tracer tr;
    tr.x1=camX; tr.y1=camY; tr.z1=camZ;
    tr.x2=camX+dx*bestT; tr.y2=camY+dy*bestT; tr.z2=camZ+dz*bestT;
    tr.age=0.0f;
    tracers.push_back(tr);

    if(hitIdx>=0){
        SEUZombieNPC *z=&seuZombies[hitIdx];
        z->health--;
        if(z->health<=0){
            z->state=1;
            z->stateTimer=0.0f;
            killCount++;

            int req=FLOOR_KILL_REQUIREMENT[currentFloor-1];
            if(currentFloor<3){
                /* Quota met: unlock the lift (it still starts closed). */
                if(killCount>=req && liftState==0)
                    liftState=1;
            }else{
                /* Final floor: no lift needed, hitting the quota wins it. */
                if(killCount>=req && !gameWon)
                    gameWon=true;
            }
        }
    }
}

/* A zombie reaching arm's reach of the shooter is instant death. */
static void updateZombieContact(void){
    if(gameOver||gameWon)return;

    int i;
    for(i=0;i<seuZombieCount;i++){
        SEUZombieNPC *z=&seuZombies[i];
        if(z->state!=0)continue;

        float dx=camX-z->x, dz=camZ-z->z;
        float d=sqrtf(dx*dx+dz*dz);
        if(d<0.9f){
            playerHealth=0;
            gameOver=true;
            return;
        }
    }
}

static void resetGame(void){
    playerHealth=100;
    killCount=0;
    shotsFired=0;
    gameOver=false;
    gameWon=false;
    currentFloor=1;
    liftState=0;
    liftDoorOpen=0.0f;
    doorTarget[LIFT_DOOR_INDEX]=0;
    floorMsgTimer=0.0f;
    playerDamageCooldown=0.0f;
    muzzleFlashTimer=0.0f;
    tracers.clear();

    camX=0.0f; camY=1.7f; camZ=10.0f;
    yaw=-90.0f; pitch=0.0f;

    int i;
    for(i=0;i<seuZombieCount;i++){
        seuZombies[i].health=3;
        seuZombies[i].state=0;
        seuZombies[i].stateTimer=0.0f;
    }
}

/* Ride the story lift up to the next floor: reset the kill quota, the
   zombies, and put the shooter back at the same start point (the next
   floor is laid out identically to this one). */
static void advanceFloor(void){
    if(currentFloor>=3)return;
    currentFloor++;
    killCount=0;
    liftState=0;
    liftDoorOpen=0.0f;
    doorTarget[LIFT_DOOR_INDEX]=0;
    floorMsgTimer=2.5f;

    int i;
    for(i=0;i<seuZombieCount;i++){
        seuZombies[i].health=3;
        seuZombies[i].state=0;
        seuZombies[i].stateTimer=0.0f;
    }

    camX=0.0f; camY=1.7f; camZ=10.0f;
    yaw=-90.0f; pitch=0.0f;
}

/* ------------------------------------------------------------ */
/* Camera / movement                                             */
/* ------------------------------------------------------------ */

static void movePlayer(void){
    float s=(keyState['j']==2)?0.13f:0.06f;
    float r=yaw*PI/180.0f;
    float fx=cosf(r),fz=sinf(r);
    float rx=cosf(r+PI/2),rz=sinf(r+PI/2);
    float mx=0,mz=0;

    if(keyState['w']||specialState[GLUT_KEY_UP]){mx+=fx;mz+=fz;}
    if(keyState['s']||specialState[GLUT_KEY_DOWN]){mx-=fx;mz-=fz;}
    if(keyState['a']||specialState[GLUT_KEY_LEFT]){mx-=rx;mz-=rz;}
    if(keyState['d']||specialState[GLUT_KEY_RIGHT]){mx+=rx;mz+=rz;}

    float len=sqrtf(mx*mx+mz*mz);
    if(len>.0001f){
        float nx=camX+mx/len*s;
        float nz=camZ+mz/len*s;
        if(!collides(nx,camZ))camX=nx;
        if(!collides(camX,nz))camZ=nz;
    }
    camY=1.7f;
}

static void keyboard(unsigned char key,int x,int y){
    (void)x;(void)y;
    if(key==27)exit(0);
    if((gameOver||gameWon) && (key=='r'||key=='R')){
        resetGame();
        return;
    }
    keyState[(unsigned char)tolower(key)]=1;
}

static void keyboardUp(unsigned char key,int x,int y){
    (void)x;(void)y;
    keyState[(unsigned char)tolower(key)]=0;
}

static void specialDown(int key,int x,int y){
    (void)x;(void)y;
    if(key<256)specialState[key]=1;
}

static void specialUp(int key,int x,int y){
    (void)x;(void)y;
    if(key<256)specialState[key]=0;
}

static void mouseMotion(int x,int y){
    if(warpingPointer){warpingPointer=0;return;}
    int cx=WIN_W/2,cy=WIN_H/2;
    yaw+=(x-cx)*.12f;
    pitch-=(y-cy)*.12f;
    if(pitch>89)pitch=89;
    if(pitch<-89)pitch=-89;
    warpingPointer=1;
    glutWarpPointer(cx,cy);
}