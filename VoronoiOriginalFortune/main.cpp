#include <stdlib.h> // MUST BE BEFORE GLUT ON WINDOWS
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#include <gl/glut.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif
 
#include <vector>
using namespace std;

#include "Vectorf.h"
#include "vdefs.h"
Voronoi *vo ;
 
inline bool GL_OK()
{
  GLenum err = glGetError() ;
  if( err != GL_NO_ERROR )
    printf( "GLERROR %d\n", err ) ;
  return err == GL_NO_ERROR ;
}
 
static float w=512.f,h=512.f,ptSize=1.f ;
Vector2f offset, zoom ;

static Vector2f mousePos ;
 
 
bool showSites=1, showBisectors=0, showTris=1, showVoronoiEdges=1, showVoronoiVertices=0, showText=1;

// the polygon the mouse is currently hovering over
Poly *currentHover = 0 ;
 
void draw()
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
  
  glViewport( 0, 0, w, h ) ;
  glMatrixMode( GL_PROJECTION ) ;
  glLoadIdentity();
  glOrtho( 0 + offset.x + zoom.x, w + offset.x - zoom.x,
           0 + offset.y + zoom.y, h + offset.y - zoom.y, 10, -10 ) ;
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  if( showTris )
  {
    glBegin( GL_TRIANGLES ) ;
    for( Poly* p : vo->outputTris )
    {
      for( int i = 0 ; i < p->pts.size() ; i++ )
      {
        glColor4fv( &p->colors[i].x ) ;
        glVertex2fv( &p->pts[i].x ) ;
      }
    }
    glEnd() ;
  }

  if( showBisectors )
  {
    glBegin( GL_LINES ) ;
    for( Vector2f & v : vo->outputBisectors )
    {
      glColor4f( 1,1,1,1 ) ;
      glVertex2fv( &v.x ) ;
    }
    glEnd() ;
  }
  
  if( showVoronoiEdges )
  {
    glBegin( GL_LINES ) ;
    for( Vector2f & v : vo->outputVoronoiEdges )
    {
      glColor4f( 0,0,1,1 ) ;
      glVertex2fv( &v.x ) ;
    }
    glEnd() ;
  }
  
  if( showVoronoiVertices )
  {
    glPointSize( 8.f ) ;
    glBegin( GL_POINTS ) ;
    for( Vector2f & v : vo->outputVoronoiVertices )
    {
      glColor4f( 1,1,0,1 ) ;
      glVertex2fv( &v.x ) ;
    }
    glEnd() ;
  }


  if( currentHover )
  {
    float pts = 24 ;
    float radius ;
    Vector2f center = currentHover->circumcircle( radius ) ;

    glBegin( GL_LINE_LOOP ) ;
    for( float i = 0 ; i < pts ; i++ )
    {
      glColor4f( 1,1,0,1 ) ;
      float ang = 2.f*M_PI * (i/pts) ;
      Vector2f pt = center + Vector2f( cosf( ang ), sinf( ang ) ) * radius ;
      glVertex2fv( &pt.x ) ;
    }
    glEnd() ;

    glBegin( GL_TRIANGLES ) ;
    for( int i = 0 ; i < currentHover->pts.size() ; i++ )
    {
      glColor4f( 1,0,0,0.5 ) ;
      glVertex2fv( &currentHover->pts[i].x ) ;
    }
    glEnd() ;
  }
  /*
  // Sites are shown by pts of varying size,
  // not circles
  if( showCircles )
  {
    float pts = 16 ;
    for( Circle & c : vo->circles )
    {
      glBegin( GL_LINE_LOOP ) ;
      for( float i = 0 ; i < pts ; i++ )
      {
        glColor4f( 1,1,0,1 ) ;
        float ang = 2.f*M_PI * (i/pts) ;
        Vector2f pt = c.center + Vector2f( cosf( ang ), sinf( ang ) ) * c.radius*20 ;
        glVertex2fv( &pt.x ) ;
      }
      glEnd() ;
    }
  }
  */

  if( showSites )
  {
    glPointSize( 8.f ) ;
    glBegin( GL_POINTS ) ;
    
    //for( Vector2f & v : vo->originalSites )
    for( Vector2f & v : vo->outputSites )
    {
      glColor4f( 1,0,0,1 ) ;
      glVertex2fv( &v.x ) ;
    }
    glEnd() ;
  }
  
 
  //TEXT
  if( showText )
  {
    glMatrixMode( GL_PROJECTION ) ;
    glLoadIdentity();
    glOrtho( 0, w, 0, h, 10, -10 ) ;
    glMatrixMode( GL_MODELVIEW ) ;
    glLoadIdentity() ;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glRasterPos2f( 20, h-20 ) ;
  
    char buf[300];
    sprintf( buf, "Click to move. Also try 2,[,],-,+,b,e,s,t,i,v" ) ;
    const char * p = buf ;
    do glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, *p ); while( *(++p) ) ;

  }
  
  // Shows the mouse cursor
  //glBegin( GL_POINTS ) ;
  //  glColor4f( 1,1,1,1 ) ;
  //  glVertex2fv( &mousePos.x ) ;
  //glEnd() ;
  
  glutSwapBuffers();
}
 
// Called every time a window is resized to resize the projection matrix
void resize( int newWidth, int newHeight )
{
  w = newWidth ;
  h = newHeight ;
}
int lastButton ;
void mouse( int button, int up, int x, int y )
{
  y = h-y;
  lastButton =button;
  //if( !up )  cp->addPt( Vector2f( x, y ), White ) ;

  if( button == GLUT_LEFT_BUTTON )
  {
    if( !up )
    {
      mousePos = Vector2f( x,y ) ;
    }
  }

  if( button == GLUT_RIGHT_BUTTON )
  {
    if( !up )
    {
      
    }
  }
}

void motion( int x, int y )
{
  if( lastButton == GLUT_LEFT_BUTTON )
  {
    y = h-y;
    Vector2f p(x,y) ;
    Vector2f d = mousePos - p ; // change i last
    offset += d ;
    mousePos = p ;
  }
}

void passiveMotion( int x, int y )
{
  y=h-y;
  Vector2f pt( x, y ) ;
  // left edge now at 0+offset.x+zoom.x, right edge w+offset-zoom.x
  Vector2f o( w,h ) ;
  pt /= o ;
  pt *= (o - zoom*2) ;
  pt += zoom ;
  pt += offset ;
  

  currentHover=0;
  for( Poly *p : vo->outputTris )
  {
    if( p->pnpoly( pt ) )
    {
      currentHover = p ;
      return ;
      // since the tris don't overlap, the mouse can only be
      // over one tri at a time
    }
  }
}
 
void keyboard( unsigned char key, int x, int y )
{
  static float lineWidth = 1.f ;
 
  switch( key )
  {
  case '2':
    {
    int pMode[2];
    glGetIntegerv( GL_POLYGON_MODE, pMode ) ;
    if( pMode[0] == GL_FILL )  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) ;
    else  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) ;
    }
    break ;
 
  case '[':
    zoom -= Vector2f( 10,10 ) ;
    break;  
  
  case ']':
    zoom += Vector2f( 10,10 ) ;
    break;

  case '-':
    lineWidth--;
    ::clamp( lineWidth, 1.f, 16.f ) ;
    glLineWidth( lineWidth ) ;
    break;
    
  case '+':
  case '=':
    lineWidth++;
    ::clamp( lineWidth, 1.f, 16.f ) ;
    glLineWidth( lineWidth ) ;
    break; 
  
  case 'b':
    showBisectors = !showBisectors ;
    break; 

  case 'e':
    showVoronoiEdges = !showVoronoiEdges ;
    break; 

  case 's':
    showSites = !showSites ;
    break ;

  case 't':
    showTris = !showTris ;
    break ;

  case 'i':
    showText = !showText ;
    break ;

  case 'v':
    showVoronoiVertices = !showVoronoiVertices ;
    break ;
  
  case 27:
    exit(0);
    break;
    
  default:
    break;
  }
}
 
void initGL()
{
  glClearColor( 0.1, 0.1, 0.1, 0.1 ) ;
  glEnable( GL_COLOR_MATERIAL ) ;
 
  ptSize=4.f;
  glPointSize( ptSize ) ;
 
  glLineWidth( 2.f ) ;
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) ;
 
  glEnable( GL_BLEND ) ;
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
}
 
int main( int argc, char **argv )
{
  glutInit( &argc, argv ) ; // Initializes glut
 
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA ) ;
  glutInitWindowSize( w, h ) ;
  glutInitWindowPosition( 0, 0 ) ;
  glutCreateWindow( "Fortune's Voronoi sweep algorithm" ) ;
  glutReshapeFunc( resize ) ;
  glutDisplayFunc( draw ) ;
  glutIdleFunc( draw ) ;
  
  glutMouseFunc( mouse ) ;
  glutMotionFunc( motion ) ;
  glutPassiveMotionFunc( passiveMotion ) ;
  
  glutKeyboardFunc( keyboard ) ;
 
  initGL();

  vo = new Voronoi( 18, w, h ) ;
  vo->solve() ;
  
  glutMainLoop();
  return 0;
}







