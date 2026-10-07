/* SEU Corridor Explorer v10 - main: HUD, render loop, timer, init */
#include "common.h"
#include "characters.h"
#include "environment.h"

/* ------------------------------------------------------------ */
/* Render                                                       */
/* ------------------------------------------------------------ */

static void display(void){
    glClearColor(.42f,.62f,.78f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0,(double)WIN_W/WIN_H,.1,200);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float ry=yaw*PI/180.0f;
    float rp=pitch*PI/180.0f;
    float lx=cosf(rp)*cosf(ry);
    float ly=sinf(rp);
    float lz=cosf(rp)*sinf(ry);

    gluLookAt(camX,camY,camZ,
              camX+lx,camY+ly,camZ+lz,
              0,1,0);

    GLfloat lp[]={7,18,20,1};
    glLightfv(GL_LIGHT0,GL_POSITION,lp);

    /* ADDED: second light source, positioned toward the washroom/lift end
       so that area also gets proper diffuse/specular lighting. */
    GLfloat lp1[]={-4,17,45,1};
    glLightfv(GL_LIGHT1,GL_POSITION,lp1);

    drawScene();
    updateNearestDoor();
    animateDoors();
    updateLiftAnimation();

    /* ---- bullet tracers, drawn in world space ---- */
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    {
        size_t ti;
        for(ti=0;ti<tracers.size();++ti){
            float a=1.0f-(tracers[ti].age/TRACER_LIFE);
            glColor4f(1.0f,0.95f,0.55f,a);
            glVertex3f(tracers[ti].x1,tracers[ti].y1,tracers[ti].z1);
            glVertex3f(tracers[ti].x2,tracers[ti].y2,tracers[ti].z2);
        }
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    /* ---- first-person gun, fixed to the screen ---- */
    glClear(GL_DEPTH_BUFFER_BIT);
    {
        bool moving=keyState['w']||keyState['a']||keyState['s']||keyState['d'];
        float bobX=moving?sinf(gunBobPhase*2.0f)*0.015f:0.0f;
        float bobY=moving?fabsf(sinf(gunBobPhase*2.0f))*0.02f:0.0f;
        glPushMatrix();
        glLoadIdentity();
        if(!gameOver && !gameWon) drawViewmodelGun(bobX,bobY);
        glPopMatrix();
    }

    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,WIN_W,0,WIN_H,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1,1,1);
    char buf[160];
    snprintf(buf,sizeof(buf),
             "SEU Corridor Explorer v10 | WASD/Arrows | Shift Run | Space Shoot | Left Click Door | ESC Quit");
    glRasterPos2f(14,WIN_H-25);
    char *p=buf;
    while(*p)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*p++);

    snprintf(buf,sizeof(buf),"Floor: %d/3   Health: %d   Kills: %d/%d   Shots: %d",
             currentFloor,playerHealth,killCount,
             FLOOR_KILL_REQUIREMENT[currentFloor-1],shotsFired);
    glColor3f(1.0f,0.85f,0.3f);
    glRasterPos2f(14,WIN_H-50);
    p=buf;
    while(*p)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*p++);

    if(nearestDoor>=0){
        if(nearestDoor==LIFT_DOOR_INDEX && currentFloor<3){
            if(liftState==0){
                int left=FLOOR_KILL_REQUIREMENT[currentFloor-1]-killCount;
                snprintf(buf,sizeof(buf),
                         "Elevator - LOCKED (kill %d more zombie%s)",
                         left,(left==1)?"":"s");
            }else if(liftState==1)
                snprintf(buf,sizeof(buf),"Elevator - LEFT CLICK TO OPEN");
            else if(liftState==2)
                snprintf(buf,sizeof(buf),"Elevator - LEFT CLICK TO CLOSE");
            else
                snprintf(buf,sizeof(buf),
                         "Elevator - LEFT CLICK TO GO TO FLOOR %d",currentFloor+1);
        }
        else if(doors[nearestDoor].type==1)
            snprintf(buf,sizeof(buf),"%s - LIFT CLOSED / ENTRY BLOCKED",
                     doors[nearestDoor].label);
        else
            snprintf(buf,sizeof(buf),"%s - LEFT CLICK TO %s",
                     doors[nearestDoor].label,
                     doorTarget[nearestDoor]?"CLOSE":"OPEN");
        glRasterPos2f(WIN_W/2-210,80);
        p=buf;
        while(*p)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*p++);
    }

    if(floorMsgTimer>0.0f){
        snprintf(buf,sizeof(buf),"FLOOR %d",currentFloor);
        glColor3f(.6f,.9f,1.0f);
        glRasterPos2f(WIN_W/2-40,WIN_H/2.0f+90);
        p=buf;
        while(*p)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*p++);
    }

    glColor3f(1,1,1);
    glBegin(GL_LINES);
        glVertex2f(WIN_W/2.0f-8,WIN_H/2.0f);
        glVertex2f(WIN_W/2.0f+8,WIN_H/2.0f);
        glVertex2f(WIN_W/2.0f,WIN_H/2.0f-8);
        glVertex2f(WIN_W/2.0f,WIN_H/2.0f+8);
    glEnd();

    if(gameOver){
        const char *c;
        glColor3f(1.0f,0.2f,0.2f);
        glRasterPos2f(WIN_W/2.0f-55,WIN_H/2.0f+40);
        const char *msg="YOU DIED";
        for(c=msg;*c;++c)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*c);

        glColor3f(1,1,1);
        glRasterPos2f(WIN_W/2.0f-95,WIN_H/2.0f+15);
        const char *msg2="Press R to restart";
        for(c=msg2;*c;++c)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*c);
    }

    if(gameWon){
        const char *c;
        glColor3f(0.25f,1.0f,0.35f);
        glRasterPos2f(WIN_W/2.0f-75,WIN_H/2.0f+40);
        const char *msg="YOU WIN!";
        for(c=msg;*c;++c)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*c);

        glColor3f(1,1,1);
        glRasterPos2f(WIN_W/2.0f-165,WIN_H/2.0f+15);
        const char *msg2="All 3 floors cleared - Press R to play again";
        for(c=msg2;*c;++c)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*c);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

static void timer(int v){
    (void)v;
    int mod=glutGetModifiers();
    keyState['j']=(mod&GLUT_ACTIVE_SHIFT)?2:0;

    /* ADDED: keep the classroom ceiling fans spinning continuously,
       regardless of game state (requirement 5). */
    fanAngle += 9.0f;
    if(fanAngle>360.0f) fanAngle-=360.0f;

    if(!gameOver && !gameWon){
        movePlayer();
        updateNearestDoor();

        zombieAnimTime += 0.016f;
        updateSEUZombies();
        updateZombieContact();

        if(keyState[' ']) tryShoot();

        bool moving=keyState['w']||keyState['a']||keyState['s']||keyState['d'];
        if(moving) gunBobPhase+=0.09f; else gunBobPhase*=0.85f;

        if(muzzleFlashTimer>0.0f) muzzleFlashTimer-=0.016f;

        {
            size_t i=0;
            while(i<tracers.size()){
                tracers[i].age+=0.016f;
                if(tracers[i].age>TRACER_LIFE) tracers.erase(tracers.begin()+i);
                else ++i;
            }
        }

        if(playerHealth<=0){
            playerHealth=0;
            gameOver=true;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16,timer,0);
}

/* ------------------------------------------------------------ */
/* Init / main                                                   */
/* ------------------------------------------------------------ */

static void initGL(void){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

    GLfloat diff[]={1.0f,.94f,.86f,1};
    GLfloat amb[]={.48f,.48f,.50f,1};
    GLfloat globalAmb[]={.24f,.24f,.26f,1};

    glLightfv(GL_LIGHT0,GL_DIFFUSE,diff);
    glLightfv(GL_LIGHT0,GL_AMBIENT,amb);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,globalAmb);

    /* ADDED: specular for the existing main light + a second light source,
       so at least two lights with ambient/diffuse/specular are active
       (requirement 7). */
    GLfloat spec0[]={.5f,.5f,.5f,1};
    glLightfv(GL_LIGHT0,GL_SPECULAR,spec0);

    glEnable(GL_LIGHT1);
    GLfloat diff1[]={.55f,.60f,.70f,1};
    GLfloat amb1[]={.10f,.10f,.12f,1};
    GLfloat spec1[]={.35f,.35f,.35f,1};
    glLightfv(GL_LIGHT1,GL_DIFFUSE,diff1);
    glLightfv(GL_LIGHT1,GL_AMBIENT,amb1);
    glLightfv(GL_LIGHT1,GL_SPECULAR,spec1);

    genTile();
    genWood();
    genMetal();
    genWall();
    genSceneTex();

    zombieQuad=gluNewQuadric();
    gluQuadricNormals(zombieQuad,GLU_SMOOTH);

    /* combat defaults (added) */
    int zi;
    for(zi=0; zi<seuZombieCount; zi++){
        seuZombies[zi].health=3;
        seuZombies[zi].state=0;
        seuZombies[zi].stateTimer=0.0f;
    }
}

int main(int argc,char **argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow("SEU Corridor Explorer v10 - Environment Only");

    initGL();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouseButton);
    glutPassiveMotionFunc(mouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(WIN_W/2,WIN_H/2);
    glutTimerFunc(16,timer,0);

    glutMainLoop();
    return 0;
}