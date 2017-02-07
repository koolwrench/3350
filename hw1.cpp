//modified by: Andrew Parker
//date: February 6, 2017
//purpose: Homework assignment
//
//cs3350 Spring 2017 Lab-1
//author: Gordon Greisel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 100000
#define CIRCLE 200
#define MAX_BOXES 5
#define GRAVITY 0.1
#define rnd() (float)rand() /(float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box;
	Shape box1;
	Shape box2;
	Shape box3;
	Shape box4;
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int b;
	int bubbler;
	int mouse[2];
	Game() { n=0; b=0; bubbler=0; }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;
	

	game.box.width = 100;
	game.box.height = 15;
	game.box.center.x = 120 + 5*55;
	game.box.center.y = 500 - 5*55;
	game.box1.width = 100;
	game.box1.height = 15;
	game.box1.center.x = 120 + 5*40;
	game.box1.center.y = 500 - 5*40;
	game.box2.width = 100;
	game.box2.height = 15;
	game.box2.center.x = 120 + 5*25;
	game.box2.center.y = 500 - 5*25;
	game.box3.width = 100;
	game.box3.height = 15;
	game.box3.center.x = 120 + 5*10;
	game.box3.center.y = 500 - 5*10;
	game.box4.width = 100;
	game.box4.height = 15;
	game.box4.center.x = 120 + 5*70;
	game.box4.center.y = 500 - 5*70;

	//declare a circle shape
	game.circle.radius = 150;
	game.circle.center.x = 700;
	game.circle.center.y = 0;

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Hw1   LMB for particle");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Initialize fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = rnd() * 0.5 - 0.25;
	p->velocity.x = rnd() * 0.5 - 0.1;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for (int i=0; i<50; i++) {
				makeParticle(game, e->xbutton.x, y);
			}
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}

	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (game->bubbler == 0) {
			game->mouse[0] = savex;
			int y = WINDOW_HEIGHT - e->xbutton.y;
			game->mouse[1] = y;
			//for (int i=0; i<10; i++) {
		    	//	makeParticle(game, e->xbutton.x, y);
			//}
		}
		//if (++n < 10)
		//	return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_b) {
		    game->bubbler ^= 1;
		}
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.



	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;
	float r, cx, cy, d1, d2, distance;

	if (game->n <= 0)
		return;

	if (game->bubbler != 0) {
	    	//the bubbler is on
		for (int i=0; i<10; i++) {
			makeParticle(game, WINDOW_WIDTH/2.0 -200,WINDOW_HEIGHT/2.0+200);
		}
	}
	for (int i=0; i<game->n; i++) {
	    	p = &game->particle[i];
	    	p->velocity.y -= GRAVITY;
	    	p->s.center.x += p->velocity.x;
	    	p->s.center.y += p->velocity.y;

		//check for collision with shapes...
		//collision with box shape
		Shape *s;
		s = &game->box;
		if (p->s.center.y < s->center.y + s->height &&
			p->s.center.y > s->center.y - s->height &&
			p->s.center.x >= s->center.x - s->width &&
			p->s.center.x <= s->center.x + s->width){
	    	p->velocity.y = -p->velocity.y * 0.4f;
		if(p->velocity.x <= 0){
			p->velocity.x = -p->velocity.x;
		}
		p->velocity.x *= 1.01f;
	    	p->s.center.y += 0.5f;
		}
		Shape *t;
		t = &game->box1;
		if (p->s.center.y < t->center.y + t->height &&
			p->s.center.y > t->center.y - t->height &&
			p->s.center.x >= t->center.x - t->width &&
			p->s.center.x <= t->center.x + t->width){
	    	p->velocity.y = -p->velocity.y * 0.4f;
		if(p->velocity.x <= 0){
			p->velocity.x = -p->velocity.x;
		}
		p->velocity.x *= 1.01f;
	    	p->s.center.y += 0.5f;
		}
		Shape *u;
		u = &game->box2;
		if (p->s.center.y < u->center.y + u->height &&
			p->s.center.y > u->center.y - u->height &&
			p->s.center.x >= u->center.x - u->width &&
			p->s.center.x <= u->center.x + u->width){
	    	p->velocity.y = -p->velocity.y * 0.4f;
		if(p->velocity.x <= 0){
			p->velocity.x = -p->velocity.x;
		}
		p->velocity.x *= 1.01f;
	    	p->s.center.y += 0.5f;
		}
		Shape *v;
		v = &game->box3;
		if (p->s.center.y < v->center.y + v->height &&
			p->s.center.y > v->center.y - v->height &&
			p->s.center.x >= v->center.x - v->width &&
			p->s.center.x <= v->center.x + v->width){
	    	p->velocity.y = -p->velocity.y * 0.4f;
		if(p->velocity.x <= 0){
			p->velocity.x = -p->velocity.x;
		}
		p->velocity.x *= 1.01f;
	    	p->s.center.y += 0.5f;
		}
		Shape *x;
		x = &game->box4;
		if (p->s.center.y < x->center.y + x->height &&
			p->s.center.y > x->center.y - x->height &&
			p->s.center.x >= x->center.x - x->width &&
			p->s.center.x <= x->center.x + x->width){
	    	p->velocity.y = -p->velocity.y * 0.4f;
	    	p->s.center.y += 0.5f;
		}

		//collision with circle shape
		Shape *c;
		c = &game->circle;
		r = c->radius;
		cx = c->center.x;
		cy = c->center.y;
		d1 = p->s.center.x - cx;
		d2 = p->s.center.y - cy;
		distance = sqrt((d1*d1 + d2*d2));
		if (distance < r) {
		    p->velocity.x = (p->velocity.x/2) + (d1/distance);
		    p->velocity.y = (p->velocity.y/2) + (d2/distance);
		    p->velocity.y += -p->velocity.y * 0.2f;
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			//std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[--game->n];
		}
	}
}
 
void render(Game *game)
{
	float w, h, r, x, y, cy, cx;
	Rect re;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	Shape *s;
	glColor3ub(90,140,90);
	s = &game->box;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();


        Shape *t;
        glColor3ub(90,140,90);
        t = &game->box1;
        glPushMatrix();
        glTranslatef(t->center.x, t->center.y, t->center.z);
        w = t->width;
        h = t->height;
        glBegin(GL_QUADS);
                glVertex2i(-w,-h);
                glVertex2i(-w, h);
                glVertex2i( w, h);
                glVertex2i( w,-h);
        glEnd();
        glPopMatrix();

	Shape *u;
        glColor3ub(90,140,90);
        u = &game->box2;
        glPushMatrix();
        glTranslatef(u->center.x, u->center.y, u->center.z);
        w = u->width;
        h = u->height;
        glBegin(GL_QUADS);
                glVertex2i(-w,-h);
                glVertex2i(-w, h);
                glVertex2i( w, h);
                glVertex2i( w,-h);
        glEnd();
        glPopMatrix();

	Shape *z;
        glColor3ub(90,140,90);
        z = &game->box3;
        glPushMatrix();
        glTranslatef(z->center.x, z->center.y, z->center.z);
        w = z->width;
        h = z->height;
        glBegin(GL_QUADS);
                glVertex2i(-w,-h);
                glVertex2i(-w, h);
                glVertex2i( w, h);
                glVertex2i( w,-h);
        glEnd();
        glPopMatrix();

	Shape *b;
        glColor3ub(90,140,90);
        b = &game->box4;
        glPushMatrix();
        glTranslatef(b->center.x, b->center.y, b->center.z);
        w = b->width;
        h = b->height;
        glBegin(GL_QUADS);
                glVertex2i(-w,-h);
                glVertex2i(-w, h);
                glVertex2i( w, h);
                glVertex2i( w,-h);
        glEnd();
        glPopMatrix();


	Shape *c;
	glColor3ub(90,140,90);
	c = &game->circle;
	r = c->radius;
	cx = c->center.x;
	cy = c->center.y;
	glPushMatrix();
	glTranslatef(c->center.x, c->center.y, c->center.z);
	glBegin(GL_TRIANGLE_FAN);
	for(int i=0; i<CIRCLE; i++) {
	    x = r*cos(i) - cx;
	    y = r*sin(i) + cy;
	    glVertex3f(x+cx, y-cy, 0);
	    x = r*cos(i+0.1) - cx;
	    y = r*sin(i+0.1) + cy;
	    glVertex3f(x+cx, y-cy, 0);
	}
	glEnd();
	glPopMatrix();
	
	
	//draw all particles here
	for (int i=0; i<game->n; i++) {
		glPushMatrix();
		if(rand() % 3 < 1)
		{
		glColor3ub(87,120,215);
		}else{
		glColor3ub(120,200,220);
		}
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}

	//draw text
	re.bot = WINDOW_HEIGHT - 585;
	re.left = 630;
	re.center = 0;
	ggprint16(&re, 0, 0x00ddaa00, "Waterfall Model");
	re.bot = game->box.center.y - 10;
	re.left = game->box.center.x - 50;
	ggprint16(&re, 0, 0x00ddaa00, "Testing");
	re.bot = game->box1.center.y - 10;
	re.left = game->box1.center.x - 50;
	ggprint16(&re, 0, 0x00ddaa00, "Coding");
	re.bot = game->box2.center.y - 10;
	re.left = game->box2.center.x - 50;
	ggprint16(&re, 0, 0x00ddaa00, "Design");
	re.bot = game->box3.center.y - 10;
	re.left = game->box3.center.x - 50;
	ggprint16(&re, 0, 0x00ddaa00, "Requirements");
	re.bot = game->box4.center.y - 10;
	re.left = game->box4.center.x - 50;
	ggprint16(&re, 0, 0x00ddaa00, "Maintenance");
	
}
