#pragma once
#include "common.h"
#include "characters.h"  /* drawScene() draws zombies; mouseButton() uses advanceFloor() */

/* ------------------------------------------------------------ */
/* Procedural materials/textures                                */
/* ------------------------------------------------------------ */

static void uploadTex(GLuint *id){
    glGenTextures(1,id);
    glBindTexture(GL_TEXTURE_2D,*id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,TEXSZ,TEXSZ,GL_RGB,
                      GL_UNSIGNED_BYTE,texBuf);
}

/* Supplied floor reference: cool grey, square tiles, subtle mottling. */
static void genTile(void){
    /* Same cool-grey concrete-look material, but with square tiles so that
       the floor has one clean grout grid rather than overlapping/double lines. */
    int tileW=96,tileH=96,m=2;
    int x,y;
    for(y=0;y<TEXSZ;y++)for(x=0;x<TEXSZ;x++){
        int xx=x%tileW, yy=y%tileH;
        int i=(y*TEXSZ+x)*3;
        if(xx<m || yy<m){
            /* thin grey grout line, slightly darker than the tile body */
            texBuf[i]=138; texBuf[i+1]=138; texBuf[i+2]=136;
        }else{
            /* soft cloudy mottling like polished/cement-look porcelain */
            float n=9*sinf(x*.021f+1.3f)+7*sinf(y*.033f)+
                    5*sinf((x+y)*.014f)+3*sinf((x-y)*.045f)+
                    2*sinf(x*.09f+y*.07f);
            int c=(int)(196+n);
            if(c<178)c=178; if(c>218)c=218;
            /* neutral-to-cool grey: R slightly below, B slightly above green */
            texBuf[i]=(GLubyte)(c-6);
            texBuf[i+1]=(GLubyte)c;
            texBuf[i+2]=(GLubyte)(c+2);
        }
    }
    uploadTex(&tileTex);
}

static void genWood(void){
    int x,y;
    for(y=0;y<TEXSZ;y++)for(x=0;x<TEXSZ;x++){
        int i=(y*TEXSZ+x)*3;
        float g=10*sinf(y*.16f+x*.025f)+4*sinf(y*.47f);
        int seam=(x%26<2)?-34:0;
        texBuf[i]=(GLubyte)fmaxf(70,fminf(190,161+g+seam));
        texBuf[i+1]=(GLubyte)fmaxf(42,fminf(148,112+g*.55f+seam*.7f));
        texBuf[i+2]=(GLubyte)fmaxf(20,fminf(96,60+g*.30f+seam*.4f));
    }
    uploadTex(&woodTex);
}

static void genMetal(void){
    int x,y;
    for(y=0;y<TEXSZ;y++)for(x=0;x<TEXSZ;x++){
        int i=(y*TEXSZ+x)*3;
        int c=118+(int)(14*sinf(x*.22f)+7*sinf(y*.08f));
        texBuf[i]=texBuf[i+1]=texBuf[i+2]=(GLubyte)c;
    }
    uploadTex(&metalTex);
}

static void genWall(void){
    int x,y;
    /* Strong warm red/brown brick, matching the supplied corridor reference. */
    const int bw=38,bh=18,m=2;
    for(y=0;y<TEXSZ;y++)for(x=0;x<TEXSZ;x++){
        int row=y/bh;
        int off=(row%2)*(bw/2);
        int xx=(x+off)%bw, yy=y%bh;
        int i=(y*TEXSZ+x)*3;
        int sh=((x*7+y*13)%11)-5;
        if(xx<m || yy<m){
            /* light tan/grey mortar line, as seen between the bricks in the photo */
            texBuf[i]=196; texBuf[i+1]=178; texBuf[i+2]=156;
        }else{
            texBuf[i]=(GLubyte)(182+sh);
            texBuf[i+1]=(GLubyte)(70+sh/2);
            texBuf[i+2]=(GLubyte)(46+sh/2);
        }
    }
    uploadTex(&wallTex);
}

/* ADDED: procedural "outside" scene (sky, grass, a receding road with
   trees along the shoulder, and a parked car) painted onto a texture so
   the new window can show an external environment even though the
   building itself is fully enclosed. */
static void genSceneTex(void){
    int x,y;
    for(y=0;y<TEXSZ;y++){
        for(x=0;x<TEXSZ;x++){
            int i=(y*TEXSZ+x)*3;
            if(y<150){
                float t=(float)y/150.0f;
                texBuf[i]  =(GLubyte)(120+70*t);
                texBuf[i+1]=(GLubyte)(170+40*t);
                texBuf[i+2]=(GLubyte)(230+10*t);
            }else{
                float n=6*sinf(x*.09f+y*.05f);
                texBuf[i]  =(GLubyte)fmaxf(40,fminf(90,60+n*.3f));
                texBuf[i+1]=(GLubyte)fmaxf(90,fminf(170,120+n));
                texBuf[i+2]=(GLubyte)fmaxf(35,fminf(85,55+n*.2f));
            }
        }
    }

    for(y=150;y<TEXSZ;y++){
        float t=(float)(y-150)/(float)(TEXSZ-150);
        float halfw=8.0f+t*70.0f;
        int cx=128;
        int xs=(int)(cx-halfw), xe=(int)(cx+halfw);
        if(xs<0)xs=0; if(xe>=TEXSZ)xe=TEXSZ-1;
        for(x=xs;x<=xe;x++){
            int i=(y*TEXSZ+x)*3;
            int n=((x*3+y*5)%9)-4;
            texBuf[i]  =(GLubyte)(96+n);
            texBuf[i+1]=(GLubyte)(96+n);
            texBuf[i+2]=(GLubyte)(100+n);
            if(abs(x-cx)<(int)(1+t*2) && ((y/10)%2==0)){
                texBuf[i]=230; texBuf[i+1]=230; texBuf[i+2]=200;
            }
        }
    }

    {
        int treeX[6]={22,50,206,234,14,244};
        int treeY[6]={168,205,175,215,235,245};
        int t;
        for(t=0;t<6;t++){
            int tx=treeX[t], ty=treeY[t];
            int rad=10+(ty-150)/6;
            for(y=ty-rad*2;y<ty+rad;y++){
                for(x=tx-rad;x<tx+rad;x++){
                    if(x<0||x>=TEXSZ||y<0||y>=TEXSZ)continue;
                    float dx=(float)(x-tx), dy=(float)(y-(ty-rad));
                    if(dx*dx+dy*dy<(float)(rad*rad)){
                        int i=(y*TEXSZ+x)*3;
                        texBuf[i]=40; texBuf[i+1]=(GLubyte)(110+((x+y)%20)); texBuf[i+2]=45;
                    }
                }
            }
            for(y=ty;y<ty+rad;y++){
                for(x=tx-2;x<=tx+2;x++){
                    if(x<0||x>=TEXSZ||y<0||y>=TEXSZ)continue;
                    int i=(y*TEXSZ+x)*3;
                    texBuf[i]=90; texBuf[i+1]=60; texBuf[i+2]=35;
                }
            }
        }
    }

    {
        int cx0=100, cx1=156, cy0=225, cy1=248;
        for(y=cy0;y<cy1;y++){
            for(x=cx0;x<cx1;x++){
                int i=(y*TEXSZ+x)*3;
                if(y<cy0+10){ texBuf[i]=190; texBuf[i+1]=40; texBuf[i+2]=40; }
                else        { texBuf[i]=170; texBuf[i+1]=30; texBuf[i+2]=30; }
            }
        }
        for(y=cy0-6;y<cy0+2;y++){
            if(y<0)continue;
            for(x=cx0+8;x<cx1-8;x++){
                int i=(y*TEXSZ+x)*3;
                texBuf[i]=170; texBuf[i+1]=205; texBuf[i+2]=225;
            }
        }
        for(y=cy1-8;y<cy1+4 && y<TEXSZ;y++){
            for(x=cx0+4;x<cx0+18;x++){int i=(y*TEXSZ+x)*3;texBuf[i]=20;texBuf[i+1]=20;texBuf[i+2]=20;}
            for(x=cx1-18;x<cx1-4;x++){int i=(y*TEXSZ+x)*3;texBuf[i]=20;texBuf[i+1]=20;texBuf[i+2]=20;}
        }
    }

    uploadTex(&sceneTex);
}

/* ------------------------------------------------------------ */
/* Drawing helpers                                               */
/* ------------------------------------------------------------ */

static void setColor(float r,float g,float b){
    GLfloat a[]={r*.35f,g*.35f,b*.35f,1}, d[]={r,g,b,1};
    GLfloat s[]={.18f,.18f,.18f,1}; /* ADDED: modest specular for realistic shading */
    glMaterialfv(GL_FRONT,GL_AMBIENT,a);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,d);
    glMaterialfv(GL_FRONT,GL_SPECULAR,s);   /* ADDED */
    glMaterialf(GL_FRONT,GL_SHININESS,24);  /* ADDED */
    glColor3f(r,g,b);
}

static void box(float x,float y,float z,float w,float h,float d){
    glPushMatrix();
    glTranslatef(x,y,z);
    glScalef(w,h,d);
    glutSolidCube(1);
    glPopMatrix();
}

static void texturedBox(float x,float y,float z,float w,float h,float d,GLuint tex){
    float rx=w/2, ry=h/2, rz=d/2;
    setColor(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tex);
    glPushMatrix();
    glTranslatef(x,y,z);
    glBegin(GL_QUADS);

    glNormal3f(0,0,1);
    glTexCoord2f(0,0);glVertex3f(-rx,-ry,rz);
    glTexCoord2f(w/1.5f,0);glVertex3f(rx,-ry,rz);
    glTexCoord2f(w/1.5f,h/1.5f);glVertex3f(rx,ry,rz);
    glTexCoord2f(0,h/1.5f);glVertex3f(-rx,ry,rz);

    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);glVertex3f(rx,-ry,-rz);
    glTexCoord2f(w/1.5f,0);glVertex3f(-rx,-ry,-rz);
    glTexCoord2f(w/1.5f,h/1.5f);glVertex3f(-rx,ry,-rz);
    glTexCoord2f(0,h/1.5f);glVertex3f(rx,ry,-rz);

    glNormal3f(1,0,0);
    glTexCoord2f(0,0);glVertex3f(rx,-ry,rz);
    glTexCoord2f(d/1.5f,0);glVertex3f(rx,-ry,-rz);
    glTexCoord2f(d/1.5f,h/1.5f);glVertex3f(rx,ry,-rz);
    glTexCoord2f(0,h/1.5f);glVertex3f(rx,ry,rz);

    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);glVertex3f(-rx,-ry,-rz);
    glTexCoord2f(d/1.5f,0);glVertex3f(-rx,-ry,rz);
    glTexCoord2f(d/1.5f,h/1.5f);glVertex3f(-rx,ry,rz);
    glTexCoord2f(0,h/1.5f);glVertex3f(-rx,ry,-rz);

    glNormal3f(0,1,0);
    glTexCoord2f(0,0);glVertex3f(-rx,ry,rz);
    glTexCoord2f(w/1.5f,0);glVertex3f(rx,ry,rz);
    glTexCoord2f(w/1.5f,d/1.5f);glVertex3f(rx,ry,-rz);
    glTexCoord2f(0,d/1.5f);glVertex3f(-rx,ry,-rz);

    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);glVertex3f(-rx,-ry,-rz);
    glTexCoord2f(w/1.5f,0);glVertex3f(rx,-ry,-rz);
    glTexCoord2f(w/1.5f,d/1.5f);glVertex3f(rx,-ry,rz);
    glTexCoord2f(0,d/1.5f);glVertex3f(-rx,-ry,rz);

    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}


/* Dedicated floor renderer: exact square 2x2 world-unit tiles.
   This is used only for the floor so no other textured geometry changes. */
static void squareTiledFloor(void){
    float x0=-23.0f, x1=23.0f;
    float z0=4.0f, z1=52.0f;
    float y=0.06f;

    setColor(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tileTex);

    /* Top surface: 23 x 24 identical 2x2 square tiles. */
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);
    glTexCoord2f(0,0);   glVertex3f(x0,y,z0);
    glTexCoord2f(23,0);  glVertex3f(x1,y,z0);
    glTexCoord2f(23,24); glVertex3f(x1,y,z1);
    glTexCoord2f(0,24);  glVertex3f(x0,y,z1);
    glEnd();

    /* Thin floor sides, kept simple and unchanged in overall size. */
    glDisable(GL_TEXTURE_2D);
    setColor(.72f,.72f,.72f);
    box(0,-.03f,28,46,.18f,.04f);
    box(0,-.03f,52,46,.18f,.04f);
    box(-23,-.03f,28,.04f,.18f,48);
    box(23,-.03f,28,.04f,.18f,48);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tileTex);
    glDisable(GL_TEXTURE_2D);
}

static void shadowBlob(float x,float y,float z,float rx,float rz){
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glColor4f(0,0,0,.20f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x,y,z);
    int i;
    for(i=0;i<=24;i++){
        float a=i*2*PI/24;
        glVertex3f(x+cosf(a)*rx,y,z+sinf(a)*rz);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

/* ------------------------------------------------------------ */
/* Floor and building shell                                      */
/* ------------------------------------------------------------ */

static void drawFloor(void){
    GLfloat spec[]={.42f,.42f,.42f,1};
    GLfloat zero[]={0,0,0,1};
    glMaterialfv(GL_FRONT,GL_SPECULAR,spec);
    glMaterialf(GL_FRONT,GL_SHININESS,30);

    /* One clean square-tile grid comes directly from tileTex.
       Do not add a second geometric grout grid here; that was causing
       the visible double-line floor in the previous version. */
    squareTiledFloor();

    glMaterialfv(GL_FRONT,GL_SPECULAR,zero);
}

/* Main corridor is z=4..18. Classrooms occupy z=18..30.
   Each classroom has a front wall with one exact 2.2m door opening. */
static void buildBuildingGeometry(void){
    wallCount=0;
    obstacleCount=0;

    /* Building boundary: a long central corridor with classrooms on both sides. */
    addWall(-24,-23.6f,4,52);
    addWall(23.6f,24,4,52);
    addWall(-24,24,3.6f,4);
    addWall(-24,24,51.6f,52);

    /* Freeze the four outer borders: the player cannot pass through
       any boundary of the floor, including the previous top-side gap. */
    addWall(-24,-23.6f,4,52);
    addWall(23.6f,24,4,52);
    addWall(-24,24,3.6f,4);
    addWall(-24,24,51.6f,52);

    /* NORTH SIDE: four classrooms opening onto the central corridor at z=28. */
    {
        float centers[4]={-16,-6,6,16};
        int i;
        for(i=0;i<4;i++){
            float c=centers[i], left=c-4.0f, right=c+4.0f, dw=2.20f;
            addWall(left,right,38.0f,38.35f);
            addWall(left,left+.30f,28.0f,38.0f);
            addWall(right-.30f,right,28.0f,38.0f);
            addWall(left,c-dw/2,27.85f,28.15f);
            addWall(c+dw/2,right,27.85f,28.15f);
            addObstacle(c-3.20f,c-2.10f,35.7f,36.8f);
            addObstacle(c-1.70f,c+1.70f,37.25f,37.75f);
            /* Whiteboard collision: prevents walking through the board. */
            addObstacle(c-2.95f,c+2.95f,37.15f,37.48f);
            int r,col;
            for(r=0;r<3;r++)for(col=0;col<2;col++){
                float sx=c-.95f+col*1.9f, sz=34.2f-r*1.8f;
                addObstacle(sx-.50f,sx+.50f,sz-.55f,sz+.55f);
            }
        }
    }

    /* SOUTH SIDE: four additional classrooms opening onto the same corridor at z=18. */
    {
        float centers[4]={-16,-6,6,16};
        int i;
        for(i=0;i<4;i++){
            float c=centers[i], left=c-4.0f, right=c+4.0f, dw=2.20f;
            addWall(left,right,7.5f,7.85f);
            addWall(left,left+.30f,7.85f,18.0f);
            addWall(right-.30f,right,7.85f,18.0f);
            addWall(left,c-dw/2,17.85f,18.15f);
            addWall(c+dw/2,right,17.85f,18.15f);
            addObstacle(c-3.20f,c-2.10f,9.0f,10.1f);
            addObstacle(c-1.70f,c+1.70f,7.95f,8.35f);
            /* Whiteboard collision: prevents walking through the board. */
            addObstacle(c-2.95f,c+2.95f,7.98f,8.30f);
            int r,col;
            for(r=0;r<3;r++)for(col=0;col<2;col++){
                float sx=c-.95f+col*1.9f, sz=10.5f+r*1.8f;
                addObstacle(sx-.50f,sx+.50f,sz-.55f,sz+.55f);
            }
        }
    }

    /* End utility block: elevators + washroom, reached from the corridor end.
       The old blanket obstacle that sealed this whole nook off (so neither
       elevator could ever be entered) has been removed: the story lift
       (Elevator A) now has real floor space to walk into once it is
       unlocked and open. Elevator B stays impassable on its own, since its
       door-plane collision in collidesDoors() never opens. */
    addWall(-7.8f,-7.5f,41.0f,49.0f);
    addWall(-4.0f,-3.7f,41.0f,49.0f);
    addWall(-7.8f,-3.7f,48.70f,49.0f);
    addWall(-7.80f,-7.30f,41.00f,41.30f);
    addWall(-6.00f,-5.50f,41.00f,41.30f);
    addWall(-4.20f,-3.70f,41.00f,41.30f);

    addWall(-3.0f,-2.70f,41.0f,49.0f);
    addWall(7.20f,7.50f,41.0f,49.0f);
    addWall(-3.0f,7.50f,48.70f,49.0f);
    addWall(-3.0f,1.40f,40.85f,41.15f);
    addWall(3.60f,7.50f,40.85f,41.15f);

    /* WASHROOM ONLY: three cubicles, high commodes, and the wash-basin
       area are physically blocked.  These coordinates exactly match the
       washroom drawing below (world X coordinates). */
    {
        float stallX[3]={0.10f,2.50f,4.90f};
        int s;
        for(s=0;s<3;s++){
            float x=stallX[s];

            /* Cubicle side partitions: front starts just behind each door. */
            addObstacle(x-1.10f,x-1.00f,44.08f,48.28f);
            addObstacle(x+1.00f,x+1.10f,44.08f,48.28f);

            /* High commode + tank. */
            addObstacle(x-.50f,x+.50f,45.72f,46.95f);
        }

        /* Wash-basin counter and two basins at the open front-right area. */
        addObstacle(6.22f,7.30f,41.98f,43.83f);
    }

    drawFloor();

    setColor(.94f,.94f,.92f);
    box(0,3.16f,28,46,.10f,48);

    setColor(1.0f,.97f,.88f);
    int i;
    for(i=-2;i<=2;i++) box(i*8,3.08f,14,1.5f,.06f,.55f);
    for(i=-2;i<=2;i++) box(i*8,3.08f,23,1.5f,.06f,.55f);
    for(i=-2;i<=2;i++) box(i*8,3.08f,32,1.5f,.06f,.55f);
    box(-5.8f,3.08f,45.0f,1.5f,.06f,.55f);
    box(4.8f,3.08f,45.0f,1.5f,.06f,.55f);
}
/* Draw the physical wall pieces. */
static void drawWallList(void){
    int i;
    for(i=0;i<wallCount;i++){
        float cx=(walls[i].minx+walls[i].maxx)/2;
        float cz=(walls[i].minz+walls[i].maxz)/2;
        float w=walls[i].maxx-walls[i].minx;
        float d=walls[i].maxz-walls[i].minz;
        if(w<0.5f) w=0.32f;
        if(d<0.5f) d=0.32f;
        texturedBox(cx,1.5f,cz,w,3.0f,d,wallTex);
        setColor(.55f,.52f,.48f);
        box(cx,.075f,cz,w+.04f,.12f,d+.04f);
    }
}

/* ------------------------------------------------------------ */
/* Doors                                                        */
/* ------------------------------------------------------------ */

static void drawDoorFace(float faceSign){
    /* The same attractive door treatment is deliberately rendered on BOTH
       faces of every classroom/washroom door.  faceSign = +1 is the corridor
       face and -1 is the room/interior face. */
    float z=faceSign;
    float baseZ=0.095f*faceSign;
    float glassZ=0.105f*faceSign;
    float glassHiZ=0.126f*faceSign;
    float plateZ=0.12f*faceSign;
    float paperZ=0.14f*faceSign;
    float knobZ=0.15f*faceSign;
    float knobTipZ=0.19f*faceSign;

    setColor(.43f,.26f,.14f);
    box(-.70f,.55f,baseZ,.05f,1.12f,.025f);
    box(.70f,.55f,baseZ,.05f,1.12f,.025f);
    box(0,1.10f,baseZ,1.40f,.05f,.025f);
    box(0,-.02f,baseZ,1.40f,.05f,.025f);
    box(0,-1.02f,baseZ,1.40f,.05f,.025f);

    /* Narrow tall dark glass slit. */
    setColor(.10f,.12f,.12f);
    box(.34f,.28f,glassZ,.26f,1.20f,.028f);
    setColor(.22f,.30f,.31f);
    box(.34f,.28f,.12f*faceSign,.19f,1.10f,.012f);
    setColor(.42f,.50f,.50f);
    box(.30f,.62f,glassHiZ,.05f,.42f,.006f);

    /* Small black nameplate and light notice. */
    setColor(.07f,.07f,.07f);
    box(.34f,1.14f,plateZ,.46f,.17f,.025f);
    setColor(.86f,.84f,.78f);
    box(.34f,1.14f,paperZ,.36f,.10f,.008f);
    setColor(.90f,.89f,.85f);
    box(.34f,.86f,.13f*faceSign,.34f,.30f,.006f);

    /* Round door knob on both sides, like a real turning handle. */
    setColor(.80f,.80f,.82f);
    glPushMatrix();
    glTranslatef(-.18f,-.05f,knobTipZ);
    glutSolidSphere(.075f,16,12);
    glPopMatrix();
    setColor(.55f,.55f,.58f);
    glPushMatrix();
    glTranslatef(-.18f,-.05f,knobZ);
    glutSolidSphere(.045f,12,8);
    glPopMatrix();
}

static void drawWashroomCubicleDoor(const Door *d){
    glPushMatrix();
    glTranslatef(d->x,d->y,d->z);

    /* Left hinge; the door swings inward into its cubicle. */
    glTranslatef(-1.02f,0,0);
    glRotatef(d->angle,0,1,0);
    glTranslatef(1.02f,0,0);

    /* Door frame stays fixed while the leaf swings on its left hinge. */
    setColor(.36f,.23f,.12f);
    box(-1.06f,1.20f,0,.08f,2.50f,.16f);
    box( 1.06f,1.20f,0,.08f,2.50f,.16f);
    box(0,2.45f,0,2.20f,.10f,.16f);

    /* Door slab. */
    setColor(.52f,.34f,.18f);
    box(0,0,0,2.04f,2.35f,.08f);

    /* Simple upper ventilation gap. */
    setColor(.24f,.24f,.24f);
    box(.20f,.82f,.045f,.82f,.38f,.012f);

    /* 3D round knob on the opening side. */
    setColor(.48f,.48f,.50f);
    glPushMatrix();
    glTranslatef(.72f,-.05f,-.065f);
    glScalef(1,1,.45f);
    glutSolidSphere(.085f,18,14);
    glPopMatrix();

    setColor(.68f,.68f,.70f);
    glPushMatrix();
    glTranslatef(.72f,-.05f,-.045f);
    glScalef(1,1,.35f);
    glutSolidSphere(.045f,14,10);
    glPopMatrix();

    glPopMatrix();
}

static void drawWoodDoor(const Door *d){
    glPushMatrix();
    glTranslatef(d->x,d->y,d->z);
    glRotatef(d->rotY,0,1,0);

    /* Same structural frame on both sides of every doorway. */
    setColor(.52f,.36f,.19f);
    box(-1.16f,0,0,.14f,3.08f,.28f);
    box( 1.16f,0,0,.14f,3.08f,.28f);
    box(0,1.51f,0,2.46f,.18f,.28f);
    setColor(.40f,.27f,.14f);
    box(-1.06f,.78f,.15f,.06f,.18f,.06f);
    box(-1.06f,-.78f,.15f,.06f,.18f,.06f);
    box( 1.06f,.78f,.15f,.06f,.18f,.06f);
    box( 1.06f,-.78f,.15f,.06f,.18f,.06f);
    box(0,1.60f,.02f,2.56f,.10f,.34f);

    /* Moving leaf. The complete decorative design is applied to BOTH faces. */
    glPushMatrix();
    glTranslatef(-1.08f,0,.03f);
    glRotatef(d->angle,0,1,0);
    glTranslatef(1.08f,0,0);

    texturedBox(0,-.02f,.03f,2.16f,2.76f,.11f,woodTex);
    drawDoorFace(+1.0f);
    drawDoorFace(-1.0f);

    glPopMatrix();
    glPopMatrix();
}

/* Elevators are intentionally NOT interactive. Their doors never slide open.
   A solid collision box and a closed metal door prevent entering the lift. */
static void drawElevator(float x,float z,const char *label,float openAmount,int isStoryLift){
    (void)label;
    setColor(.22f,.23f,.24f);
    box(x,1.55f,z,1.55f,3.15f,.36f);

    setColor(.08f,.09f,.10f);
    box(x,1.45f,z-.20f,1.30f,2.86f,.08f);

    /* ADDED: only while the story lift's doors are actually sliding open
       (mouse-click) or standing open does the interior read as a dark
       shaft; while closed it looks exactly as before. Nothing about the
       lift's collision, doors, or the decorative Elevator B is changed. */
    if(isStoryLift && openAmount>0.02f){
        glDisable(GL_LIGHTING);
        glColor3f(.02f,.02f,.03f);
        box(x,1.45f,z-.30f,1.22f,2.80f,.05f);
        box(x-.62f,1.45f,z-.20f,.05f,2.80f,.36f);
        box(x+.62f,1.45f,z-.20f,.05f,2.80f,.36f);
        box(x,2.83f,z-.20f,1.22f,.05f,.36f);
        glEnable(GL_LIGHTING);
    }

    /* The story lift's two door panels slide apart as it opens; the
       decorative elevator always passes openAmount==0 and stays shut. */
    float slide=openAmount*0.30f;
    texturedBox(x-.30f-slide,1.45f,z-.24f,.58f,2.78f,.035f,metalTex);
    texturedBox(x+.30f+slide,1.45f,z-.24f,.58f,2.78f,.035f,metalTex);

    setColor(.10f,.10f,.11f);
    box(x,2.96f,z-.24f,1.42f,.10f,.06f);

    /* Status light: red while locked, green once the kill quota is met
       (unlocked/open/ready-to-ride). The decorative elevator stays green
       as before since it was never meant to signal anything. */
    if(isStoryLift && liftState==0) setColor(.68f,.16f,.13f);
    else setColor(.25f,.60f,.38f);
    box(x,2.75f,z-.27f,.25f,.16f,.04f);

    /* call panel is outside, but there is no interaction/opening */
    setColor(.08f,.08f,.08f);
    box(x+.92f,1.55f,z-.20f,.30f,1.05f,.08f);
    setColor(.78f,.78f,.72f);
    box(x+.92f,2.06f,z-.25f,.18f,.20f,.03f);
    setColor(.78f,.62f,.20f);
    box(x+.92f,1.60f,z-.25f,.13f,.16f,.03f);
}

/* ADDED: small continuously-rotating ceiling fan (requirement 5). */
static void drawCeilingFan(float x,float y,float z){
    glPushMatrix();
    glTranslatef(x,y,z);

    /* short mounting rod down from the ceiling */
    setColor(.15f,.15f,.16f);
    box(0,.10f,0,.05f,.20f,.05f);

    /* motor hub */
    setColor(.20f,.20f,.22f);
    box(0,0,0,.22f,.10f,.22f);

    /* spinning blades */
    glPushMatrix();
    glRotatef(fanAngle,0,1,0);
    setColor(.80f,.80f,.78f);
    int b;
    for(b=0;b<4;b++){
        glPushMatrix();
        glRotatef(b*90.0f,0,1,0);
        box(.32f,0,0,.58f,.02f,.11f);
        glPopMatrix();
    }
    glPopMatrix();

    glPopMatrix();
}

/* ADDED: a large "window" showing an outdoor environment (road, trees,
   a car) painted onto sceneTex, mounted flush against a border wall
   (requirement 6). It is purely visual and does not alter any wall,
   door, or collision geometry already in the level. */
static void drawWindowView(float x,float y,float z,float rotY){
    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(rotY,0,1,0);

    setColor(.40f,.27f,.14f);
    box(0,0,0,3.6f,2.6f,.12f);

    setColor(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,sceneTex);
    glBegin(GL_QUADS);
    glNormal3f(0,0,1);
    /* v=0 -> sky row, v=1 -> road/ground row, so the sky must land on the
       TOP edge of the window and the road/car on the BOTTOM edge. */
    glTexCoord2f(0,1); glVertex3f(-1.65f,-1.15f,.07f);
    glTexCoord2f(1,1); glVertex3f( 1.65f,-1.15f,.07f);
    glTexCoord2f(1,0); glVertex3f( 1.65f, 1.15f,.07f);
    glTexCoord2f(0,0); glVertex3f(-1.65f, 1.15f,.07f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    /* window cross-bar mullions */
    setColor(.30f,.20f,.11f);
    box(0,0,.09f,.08f,2.3f,.05f);
    box(0,0,.09f,3.3f,.08f,.05f);

    glPopMatrix();
}

/* ------------------------------------------------------------ */
/* Classroom and washroom interiors                             */
/* ------------------------------------------------------------ */

static void drawClassroom(float cx,float cz){
    glPushMatrix();
    glTranslatef(cx,0,cz);

    /* Teaching wall / whiteboard is always at the BACK of the room,
       away from the doorway. */
    float boardZ = (cz < 20.0f) ? -4.35f : 4.35f;
    setColor(.88f,.88f,.85f);
    box(0,1.72f,boardZ,5.9f,1.55f,.08f);
    setColor(.16f,.16f,.16f);
    box(0,1.00f,boardZ + ((cz < 20.0f) ? .07f : -.07f),5.65f,.08f,.05f);

    /* Teacher desk: keep it at the teaching/whiteboard side. */
    float southRoom = (cz < 20.0f);
    float teacherZ = southRoom ? -3.25f : 3.25f;
    setColor(.18f,.18f,.19f);
    box(-2.65f,.55f,teacherZ,.95f,1.05f,.65f);
    setColor(.66f,.50f,.32f);
    box(-2.65f,1.10f,teacherZ,1.05f,.10f,.70f);

    /* Three rows x three columns of lecture chair/desk combos.
       South rooms (101-104) are mirrored so the students face the
       whiteboard at the back, while all furniture stays well inside
       the classroom. North rooms keep the original arrangement. */
    int r,c;
    for(r=0;r<3;r++)for(c=0;c<3;c++){
        float x=-1.8f+c*1.8f;
        float z = southRoom ? (-2.2f+r*1.85f) : (2.2f-r*1.85f);

        glPushMatrix();
        if(southRoom){
            glTranslatef(x,0,z);
            glRotatef(180.0f,0,1,0);
            glTranslatef(-x,0,-z);
        }

        /* black tubular 4-leg frame */
        setColor(.07f,.07f,.075f);
        box(x-.32f,.23f,z-.20f,.055f,.46f,.055f);
        box(x+.32f,.23f,z-.20f,.055f,.46f,.055f);
        box(x-.32f,.23f,z+.20f,.055f,.46f,.055f);
        box(x+.32f,.23f,z+.20f,.055f,.46f,.055f);
        box(x,.05f,z,.70f,.05f,.46f);

        /* solid wine/maroon moulded seat pan */
        setColor(.34f,.08f,.07f);
        box(x,.47f,z,.76f,.09f,.56f);

        /* slatted maroon backrest, angled slightly back */
        box(x,.68f,z-.24f,.72f,.10f,.06f);
        box(x,.83f,z-.27f,.72f,.10f,.06f);
        box(x,.98f,z-.30f,.72f,.10f,.06f);
        box(x,1.11f,z-.32f,.72f,.09f,.06f);
        setColor(.07f,.07f,.075f);
        box(x-.30f,.85f,z-.29f,.05f,.55f,.05f);
        box(x+.30f,.85f,z-.29f,.05f,.55f,.05f);

        /* black tablet-arm writing desk on the right, with its own support leg */
        setColor(.10f,.10f,.11f);
        box(x+.44f,.80f,z-.10f,.62f,.05f,.56f);
        box(x+.68f,.42f,z+.10f,.05f,.80f,.05f);
        box(x+.68f,.045f,z+.10f,.34f,.05f,.34f);

        glPopMatrix();
    }

    /* ceiling light visible in classroom */
    setColor(1.0f,.98f,.90f);
    box(0,2.98f,0,1.4f,.05f,.65f);

    /* ADDED: 4 small continuously-rotating ceiling fans per classroom
       (requirement 5), kept clear of the ceiling light box above. */
    drawCeilingFan(-1.5f,2.85f,-1.3f);
    drawCeilingFan( 1.5f,2.85f,-1.3f);
    drawCeilingFan(-1.5f,2.85f, 1.3f);
    drawCeilingFan( 1.5f,2.85f, 1.3f);

    /* soft contact shadows under the furniture */
    shadowBlob(0,.03f,0,3.1f,3.8f);
    glPopMatrix();
}
static void drawWashroom(float cx,float cz){
    /* Washroom interior: three separate private toilet cubicles, high
       commodes, wash basins, faucets, and a large framed mirror. */
    int i;
    float stallX[3]={-2.40f,0.00f,2.40f};

    /* Rear wall tile finish. */
    setColor(.90f,.90f,.88f);
    texturedBox(cx,1.50f,cz+3.05f,7.0f,2.95f,.08f,tileTex);

    /* Three cubicles. */
    for(i=0;i<3;i++){
        float sx=cx + stallX[i];

        /* Side partitions stay inside the washroom shell. */
        setColor(.82f,.82f,.80f);
        box(sx-1.05f,1.35f,cz+1.18f,.10f,2.60f,4.20f);
        box(sx+1.05f,1.35f,cz+1.18f,.10f,2.60f,4.20f);

        /* Top privacy panel. */
        setColor(.72f,.72f,.70f);
        box(sx,2.60f,cz+1.18f,2.10f,.10f,4.20f);

        /* High commode pedestal. */
        setColor(.93f,.93f,.91f);
        box(sx,.42f,cz+1.45f,.72f,.70f,.80f);

        /* Raised bowl. */
        glPushMatrix();
        glTranslatef(sx,.83f,cz+1.18f);
        glScalef(.56f,.25f,.62f);
        glutSolidSphere(1.0,24,16);
        glPopMatrix();

        /* Seat/rim. */
        setColor(.82f,.82f,.80f);
        glPushMatrix();
        glTranslatef(sx,1.02f,cz+1.18f);
        glRotatef(90,1,0,0);
        glutSolidTorus(.055f,.38f,12,24);
        glPopMatrix();

        /* High flush tank. */
        setColor(.90f,.90f,.88f);
        box(sx,1.15f,cz+1.72f,.62f,1.10f,.28f);

        /* Flush button. */
        setColor(.55f,.55f,.57f);
        glPushMatrix();
        glTranslatef(sx,1.48f,cz+1.56f);
        glutSolidSphere(.055f,12,8);
        glPopMatrix();
    }

    /* Animated cubicle doors are drawn separately in drawScene().
       Do NOT draw static slabs here, otherwise each door would be drawn twice. */

    /* Wash-basin counter fixed firmly on the washroom's RIGHT side. */
    {
        const float basinX = 6.75f;
        const float mirrorX = 7.10f;

        setColor(.72f,.72f,.70f);
        box(basinX,.82f,cz-2.10f,1.05f,.16f,1.85f);

        /* Two separate ceramic basins. */
        for(i=0;i<2;i++){
            float bz=cz-2.55f+i*.90f;

            setColor(.94f,.94f,.92f);
            glPushMatrix();
            glTranslatef(basinX,1.00f,bz);
            glScalef(.42f,.18f,.34f);
            glutSolidSphere(1.0,24,16);
            glPopMatrix();

            /* Drain. */
            setColor(.48f,.49f,.50f);
            glPushMatrix();
            glTranslatef(basinX,1.08f,bz);
            glutSolidSphere(.055f,12,8);
            glPopMatrix();

            /* Faucet stem and spout. */
            setColor(.62f,.63f,.65f);
            box(basinX,1.28f,bz+.22f,.08f,.48f,.08f);
            box(basinX,1.50f,bz+.12f,.08f,.08f,.26f);
        }

        /* Large framed mirror above the basins. */
        setColor(.28f,.28f,.29f);
        box(mirrorX,2.05f,cz-2.10f,.08f,1.55f,1.85f);
        setColor(.55f,.70f,.74f);
        box(mirrorX-.05f,2.05f,cz-2.10f,.025f,1.35f,1.65f);
    }

    /* Small ceiling light inside the washroom. */
    setColor(1.0f,.98f,.90f);
    box(cx,3.00f,cz+0.20f,1.30f,.05f,.55f);

    shadowBlob(cx,.03f,cz,3.2f,2.8f);

    /* Clean ceiling panel for the room. */
    setColor(.94f,.94f,.92f);
    box(cx,3.05f,cz,7.0f,.08f,6.9f);
}


/* ------------------------------------------------------------ */
/* Scene                                                        */
/* ------------------------------------------------------------ */

static void setupDoors(void){
    doorCount=0;
    /* South row */
    addDoor(-16,1.55f,18.0f,0,"Room 101",0);
    addDoor(-6, 1.55f,18.0f,0,"Room 102",0);
    addDoor( 6, 1.55f,18.0f,0,"Room 103",0);
    addDoor(16, 1.55f,18.0f,0,"Room 104",0);
    /* North row */
    addDoor(-16,1.55f,28.0f,0,"Room 105",0);
    addDoor(-6, 1.55f,28.0f,0,"Room 106",0);
    addDoor( 6, 1.55f,28.0f,0,"Room 107",0);
    addDoor(16, 1.55f,28.0f,0,"Room 108",0);
    addDoor(2.50f,1.55f,41.0f,0,"Washroom",2);
    addDoor(0.10f,1.20f,44.02f,0,"Toilet 1",3);
    addDoor(2.50f,1.20f,44.02f,0,"Toilet 2",3);
    addDoor(4.90f,1.20f,44.02f,0,"Toilet 3",3);
    addDoor(-6.65f,1.55f,41.0f,0,"Elevator A (Closed)",1);
    addDoor(-4.85f,1.55f,41.0f,0,"Elevator B (Closed)",1);
}

static void drawScene(void){
    buildBuildingGeometry();
    setupDoors();
    drawWallList();

    /* South row classrooms */
    drawClassroom(-16,12.5f);
    drawClassroom(-6,12.5f);
    drawClassroom(6,12.5f);
    drawClassroom(16,12.5f);
    /* North row classrooms */
    drawClassroom(-16,33.0f);
    drawClassroom(-6,33.0f);
    drawClassroom(6,33.0f);
    drawClassroom(16,33.0f);

    drawWashroom(2.5f,45.0f);

    int i;
    for(i=0;i<doorCount;i++){
        doors[i].angle=doorAngle[i];
        if(doors[i].type==0 || doors[i].type==2)
            drawWoodDoor(&doors[i]);
        else if(doors[i].type==3)
            drawWashroomCubicleDoor(&doors[i]);
    }
    drawElevator(-6.65f,41.0f,"Elevator A",liftDoorOpen,1);
    drawElevator(-4.85f,41.0f,"Elevator B",0.0f,0);

    /* ADDED: window onto the outside environment (requirement 6), mounted
       flush on the east border wall in the open corridor stretch between
       the two classroom rows. Purely decorative; no wall/collision data
       is touched. */
    drawWindowView(23.55f,1.9f,23.0f,-90.0f);

    /* Supplied zombie character only: six human-sized NPCs. */
    drawSEUZombies();
}

/* ------------------------------------------------------------ */
/* Door interaction                                              */
/* ------------------------------------------------------------ */

static void updateNearestDoor(void){
    float best=3.4f;
    nearestDoor=-1;
    int i;
    for(i=0;i<doorCount;i++){
        float dx=camX-doors[i].x;
        float dz=camZ-doors[i].z;
        float d=sqrtf(dx*dx+dz*dz);
        if(d<best){
            best=d;
            nearestDoor=i;
        }
    }
}

static void mouseButton(int button,int state,int x,int y){
    (void)x;(void)y;
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        /* Recalculate at the exact click moment.  When inside the washroom,
           a nearby cubicle door gets priority over the main washroom door. */
        updateNearestDoor();

        int hit=nearestDoor;
        float best=3.0f;
        int i;
        for(i=0;i<doorCount;i++){
            if(doors[i].type!=3)continue;
            float dx=camX-doors[i].x;
            float dz=camZ-doors[i].z;
            float d=sqrtf(dx*dx+dz*dz);
            if(d<best){
                best=d;
                hit=i;
            }
        }

        if(hit<0)return;

        if(hit==LIFT_DOOR_INDEX){
            if(currentFloor<3){
                if(liftState==1)      liftState=2;      /* open the doors   */
                else if(liftState==2) liftState=3;      /* close, rider in */
                else if(liftState==3) advanceFloor();   /* ride to next floor */
                /* liftState==0: still locked, the click does nothing */
            }
            return;
        }
        if(doors[hit].type==1)return;

        nearestDoor=hit;
        doorTarget[hit]=!doorTarget[hit];
    }
}

/* Slides the story lift's doors toward open (liftState==2) or closed
   (every other state). */
static void updateLiftAnimation(void){
    float target=(liftState==2)?1.0f:0.0f;
    float delta=target-liftDoorOpen;
    if(fabsf(delta)<.02f) liftDoorOpen=target;
    else liftDoorOpen+=delta*.12f;

    if(floorMsgTimer>0.0f) floorMsgTimer-=0.016f;
}

static void animateDoors(void){
    int i;
    for(i=0;i<doorCount;i++){
        if(doors[i].type==1){
            /* Elevator doors never use the hinge animation; the story
               lift's sliding panels are driven by liftDoorOpen instead
               (see updateLiftAnimation). */
            doorAngle[i]=0;
            doorTarget[i]=0;
            continue;
        }
        float target;
        if(doors[i].type==3)
            target=doorTarget[i]?-78.0f:0.0f;
        else
            target=doorTarget[i]?78.0f:0.0f;
        float delta=target-doorAngle[i];
        if(fabsf(delta)<.7f)doorAngle[i]=target;
        else doorAngle[i]+=delta*.18f;
    }
}