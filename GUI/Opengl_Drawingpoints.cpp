#include<glut.h>
#include<windows.h>
#include "math.h"

#include "stdlib.h"

void init(void)
{
	glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_SMOOTH);
}

void quadra(void)
{
	
	
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glBegin(GL_TRIANGLES) ;
		glColor3f(0.0,1.0,0.0) ;
		glVertex2f(-0.8,-0.8) ;
		glColor3f(1.0,0.0,0.0) ;
		glVertex2f(-0.8,0.75) ;
		glColor3f(1.0,1.0,0.0) ;
		glVertex2f(0.75,0.75) ;
		glColor3f(0.0,0.0,1.0) ;
		glVertex2f(0.75,-0.8) ;
   glEnd() ;

	glPopMatrix();

	glutSwapBuffers();
}

 int main(int argc,char** argv)
 {
	 glutInit(&argc,argv);
	 glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH);
	 
	 glutInitWindowSize(300,300);
	 glutInitWindowPosition(100,100);

	 glutCreateWindow("Example for Points");

	 init();
	 glutDisplayFunc(quadra);
	 glutMainLoop();

	 return 0;

 }
