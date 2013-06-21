#ifndef VDEFS_H
#define VDEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
using namespace std ;

#include "Vectorf.h"

#define le 0
#define re 1
#define DELETED -2

struct Freenode {
  struct Freenode * nextfree;
} ;

struct Freelist
{
  Freenode * head;
  int nodesize;

  Freelist( int size )
  {
    head = NULL ;
    nodesize = size ;
  }
  
  // meaning like "delete", only leaks.
  // this actually puts `curr` at the front of the list
  void makefree( Freenode * curr )
  {
    curr->nextfree = head ; // curr's next is old list head
    head = curr ;           // list head is now `curr`
  }

  // gets you the HEAD of the list.
  // effectively pops the front element too.
  char *getfree( int sqrt_nsites )
  {
    int i ;
    Freenode * t ;

    if( !head )
    {
      // If the list is empty, allocate room for sqrt_nsites nodes,
      // all of which will be Freenode type.
      t = (Freenode *)malloc( sqrt_nsites * nodesize ) ;
      for(i = 0 ; i < sqrt_nsites ; i++)
      {
        // Then he allocates i*nodeSize bytes for each Freenode.
        // This is basically generics in C.
        makefree( (Freenode *)((char *)t+i*nodesize) ) ; // "inserts" each node
        // into the head of the list.  this is a type of push_front().
      }
    }

    t = head ;
    head = head->nextfree ;
    return (char *)t ; // t now points to the front of the list.
    // head is 1 after that.
  }
} ;


// structure used both for sites and for vertices
struct Site
{
  Vector2f coord ;
  int siteNumber ; // this says "siteNumber"
  int refcnt ;

  Site() : siteNumber( 0 ), refcnt( 0 )
  {
  }

  Site( float sx, float sy, int iSiteNo ) : coord( sx, sy ), siteNumber( iSiteNo ), refcnt( 0 )
  {
  }
} ;

struct Edge
{
  float a, b, c ;
  Site * ep[2] ;
  Site * reg[2] ;
  int edgeNumber ;
} ;

struct Halfedge
{
  struct Halfedge * ELleft ;
  struct Halfedge * ELright ;
  Edge * ELedge ;
  int ELrefcnt ;
  char ELpm ;
  Site * vertex ;
  float ystar ;
  struct Halfedge * PQnext ;
} ;

// sort sites on y, then x, coord
// Accepts void* b/c qsort() requires it.
int scomp(const void * vs1, const void * vs2)
{
  Site * s1 = *( (Site**)vs1 ) ;
  Site * s2 = *( (Site**)vs2 ) ;

  if (s1->coord.y < s2->coord.y)
  {
    return -1 ;
  }
  if (s1->coord.y > s2->coord.y)
  {
    return 1 ;
  }
  if (s1->coord.x < s2->coord.x)
  {
    return -1 ;
  }
  if (s1->coord.x > s2->coord.x)
  {
    return 1 ;
  }
  return 0 ;
}



// Voronoi is the main program object.
// It loads sites, solves the Voronoi diagram,
// and stores all global data.
struct Voronoi
{
  vector<Site*> sites ; // the list of sites (generated randomly or clicked by the user)
  int siteidx ;         // the id of the Site we are currently processing
  float xmin, xmax, ymin, ymax ;//maxes/mins of the pts in the scene
  float w,h ;           // user desired window width and height

  //OPTIONS
  int debug, prints ;

  Freelist *siteFreelist ;
  Freelist *edgeFreelist ;

  // "GEOMETRY"
  float deltax, deltay ;
  int sqrt_nsites, nedges, // outputted voronoi edges
    nvertices ; // outputted Voronoi vertices
  

  vector<Vector2f> outputSites, originalSites, outputVoronoiVertices ;
  vector<Vector2f> outputBisectors, outputVoronoiEdges ;
  vector<Poly*> outputTris ;
  vector<Circle> circles ;

  // read all sites, sort, and compute xmin, xmax, ymin, ymax
  Voronoi( int numSites, float width, float height )
  {
    w=width,h=height;
    debug = 0 ;
    prints = 0 ;
    siteidx = 0 ; //start at site 0

    // 1. initialize Q with all sites

    /*
    FILE *f = fopen( "t", "r" ) ;
    if( !f ){
      printf( "THE VERTEX FILE `t` WAS NOT FOUND." ) ;//, filename ) ;
      exit(1);
    }

    float sx,sy;
    while( fscanf( f, "%f %f", &sx, &sy ) != EOF )
    {
      Site *s = new Site() ;
      s->coord.x = sx*width ;
      s->coord.y = sy*height ;
      s->siteNumber = sites.size() ; // 0 for first site (s hasn't been pushed yet)
      s->refcnt = 0 ;

      sites.push_back( s ) ;
    }
    fclose( f ) ;
    //*/

    ///*
    srand( time(0) ) ;
    for( int i = 0 ; i < numSites ; i++ )
    {
      Site *s = new Site( randFloat( 0, width ), randFloat( 0, height ), sites.size() ) ;
      originalSites.push_back( s->coord ) ;
      s->siteNumber = sites.size() ; // 0 for first site (s hasn't been pushed yet)
      sites.push_back( s ) ;
    }
    //*/
    
    // SORTS
    qsort( (void *)&sites[0], sites.size(), sizeof(Site*), scomp ) ;

    // FIND THE MIN-XY/MAX-XY
    findMaxMin() ;

    // Initialize the sitefreelist and edgefreelist.
    siteFreelist = new Freelist( sizeof(Site) ) ;
    edgeFreelist = new Freelist( sizeof(Edge) ) ;

    // initialize # produced voronoi edges and vertices to 0
    nvertices = nedges = 0 ;

    // he magically adds 4 to nsites for some reason.
    sqrt_nsites = sqrtf( sites.size()+4 ) ;
    
  }

  void findMaxMin()
  {
    // The ymin is always the first one after qsort,
    // ymax always the last one
    ymin = sites[0]->coord.y ;
    ymax = sites[sites.size()-1]->coord.y ;

    // They're not sorted in x necessarily, so,
    // we have to manually find the x min/max
    xmin = sites[0]->coord.x ;
    xmax = sites[0]->coord.x ;
    for( int i = 1 ; i < sites.size() ; ++i )
    {
      if( sites[i]->coord.x < xmin )
        xmin = sites[i]->coord.x ;
      if( sites[i]->coord.x > xmax )
        xmax = sites[i]->coord.x ;
    }
    
    // Now these tell the width and height OF THE SITES in the graph
    deltay = ymax - ymin ;
    deltax = xmax - xmin ;
  }

  // return a single in-storage site
  Site *next()
  {
    Site * s = 0 ;
    if( siteidx < sites.size() )
      s = sites[ siteidx++ ] ;
    return s ; // if 0, it means we're done
  }
  
  Site * bottomsite ; // this is the lowest, unprocessed site so far.
  Halfedge * ELleftend, * ELrightend ;
  void solve()
  {

    Site * newsite, * bot, * top, * temp, * p, * v ;
    Vector2f newintstar ;
    int pm ;
    Halfedge * lbnd, * rbnd, * llbnd, * rrbnd, * bisector ;
    Edge * e ;

    PQinitialize() ;  
    bottomsite = next() ;    // 2. p <- extract_min( Q )
    out_site( bottomsite ) ; // This just displays each site as we process it.

    ELinitialize() ;
    newsite = next() ; // This is `p` from the paper.
    while( 1 )
    {
      if( !PQempty() ) // 
      {
        newintstar = PQ_min() ;
      }
      if( newsite &&
          ( PQempty() || newsite -> coord.y < newintstar.y || 
          ( newsite->coord.y == newintstar.y && newsite->coord.x < newintstar.x ) ) 
        )
      {// new site is smallest
        out_site( newsite ) ;

        lbnd = ELleftbnd(&(newsite->coord)) ;
        rbnd = ELright(lbnd) ;
        bot = rightreg(lbnd) ;
        e = bisect(bot, newsite) ;
        bisector = HEcreate(e, le) ;
        ELinsert(lbnd, bisector) ;
        p = intersect(lbnd, bisector) ;
        if( p )
        {
          PQdelete(lbnd) ;
          PQinsert(lbnd, p, dist(p,newsite)) ;
        }
        lbnd = bisector ;
        bisector = HEcreate(e, re) ;
        ELinsert(lbnd, bisector) ;
        p = intersect(bisector, rbnd) ;
        if( p )
        {
          PQinsert( bisector, p, dist(p,newsite) ) ;
        }
        newsite = next() ;
      }
      else if( !PQempty() )   // intersection is smallest
      {
        lbnd = PQextractmin() ;
        llbnd = ELleft(lbnd) ;
        rbnd = ELright(lbnd) ;
        rrbnd = ELright(rbnd) ;
        bot = leftreg(lbnd) ;
        top = rightreg(rbnd) ;
        out_triple(bot, top, rightreg(lbnd)) ;
        v = lbnd->vertex ;
        makevertex(v) ;
        endpoint(lbnd->ELedge, lbnd->ELpm, v);
        endpoint(rbnd->ELedge, rbnd->ELpm, v) ;
        ELdelete(lbnd) ;
        PQdelete(rbnd) ;
        ELdelete(rbnd) ;
        pm = le ;
        if (bot->coord.y > top->coord.y)
        {
          temp = bot ;
          bot = top ;
          top = temp ;
          pm = re ;
        }
        e = bisect(bot, top) ;
        bisector = HEcreate(e, pm) ;
        ELinsert(llbnd, bisector) ;
        endpoint(e, re-pm, v) ;
        deref(v) ;
        p = intersect(llbnd, bisector) ;
        if( p )
        {
          PQdelete(llbnd) ;
          PQinsert(llbnd, p, dist(p,bot)) ;
        }
        p = intersect(bisector, rrbnd) ;
        if( p )
        {
          PQinsert(bisector, p, dist(p,bot)) ;
        }
      }
      else
      {
        break ;
      }
    }

    for( lbnd = ELright(ELleftend) ; lbnd != ELrightend ; lbnd = ELright(lbnd) )
    {
      e = lbnd->ELedge ;
      out_endpoint(e) ;
    }
  }









// HALFEDGE CLASS
// This delete routine can't reclaim node, since pointers from hash
// table may be present.
void ELdelete(Halfedge * he)
{
  he->ELleft->ELright = he->ELright ;
  he->ELright->ELleft = he->ELleft ;
  he->ELedge = (Edge *)DELETED ;
}

Halfedge *ELright(Halfedge * he)
{
  return he->ELright ;
}

Halfedge *ELleft(Halfedge * he)
{
  return he->ELleft ;
}

Site *leftreg(Halfedge * he)
{
  if( !he->ELedge )
  {
    return bottomsite ;
  }
  return he->ELpm == le ? he->ELedge->reg[le] : he->ELedge->reg[re] ;
}

Site *rightreg(Halfedge * he)
{
  if( !he->ELedge )
  {
    return(bottomsite) ;
  }
  return (he->ELpm == le ? he->ELedge->reg[re] :
    he->ELedge->reg[le]) ;
}





// "HEAP"
int PQmin, PQcount, PQhashsize ;
Halfedge * PQhash ;

void PQinsert(Halfedge * he, Site * v, float offset)
{
  Halfedge * last, * next ;

  he->vertex = v ;
  ref(v) ;
  he->ystar = v->coord.y + offset ;
  last = &PQhash[ PQbucket(he)] ;
  while( (next = last->PQnext) != NULL &&
    (he->ystar  > next->ystar  ||
    (he->ystar == next->ystar &&
    v->coord.x > next->vertex->coord.x)))
  {
    last = next ;
  }
  he->PQnext = last->PQnext ;
  last->PQnext = he ;
  PQcount++ ;
}

void PQdelete(Halfedge * he)
{
  Halfedge * last;

  if( he->vertex != NULL )
  {
    last = &PQhash[PQbucket(he)] ;
    while (last -> PQnext != he)
    {
      last = last->PQnext ;
    }
    last->PQnext = he->PQnext;
    PQcount-- ;
    deref(he->vertex) ;
    he->vertex = NULL ;
  }
}

int PQbucket(Halfedge * he)
{
  int bucket ;
  if( he->ystar < ymin )  bucket = 0;
  else if( he->ystar >= ymax ) bucket = PQhashsize-1;
  else
    bucket = (he->ystar - ymin)/deltay * PQhashsize;
  
  if( bucket < 0 )  bucket = 0 ;
  if( bucket >= PQhashsize )
    bucket = PQhashsize-1 ;
  if (bucket < PQmin)
    PQmin = bucket ;
  return bucket ;
}

int PQempty()
{
  return PQcount == 0 ;
}


Vector2f PQ_min()
{
  Vector2f answer ;

  // 
  while( !PQhash[PQmin].PQnext )
    ++PQmin ;
  
  answer.x = PQhash[PQmin].PQnext->vertex->coord.x ;
  answer.y = PQhash[PQmin].PQnext->ystar ;
  return answer ;
}

Halfedge *PQextractmin()
{
  Halfedge * curr ;
  curr = PQhash[PQmin].PQnext ;
  PQhash[PQmin].PQnext = curr->PQnext ;
  PQcount-- ;
  return curr ;
}

void PQinitialize()
{
  PQcount = PQmin = 0 ;
  PQhashsize = 4 * sqrt_nsites ;
  PQhash = (Halfedge *)malloc(PQhashsize * sizeof *PQhash) ;
  for( int i = 0 ; i < PQhashsize; i++ )
  {
    PQhash[i].PQnext = NULL ;
  }
}










///"OUTPUT"
void circle(float ax, float ay, float radius)
{
  circles.push_back( Circle( Vector2f( ax, ay ), radius ) ) ;
}

void out_bisector( Edge * e )
{
  if( debug )
    printf("line (%d) %gx+%gy=%g, bisecting %d %d\n",
      e->edgeNumber, e->a, e->b, e->c, e->reg[le]->siteNumber, e->reg[re]->siteNumber) ;
  else if( prints )
    printf("l %f %f %f\n", e->a, e->b, e->c) ;
  
  // equation of line is given as ax + by = c, 
  // get the xy intercepts
  float xint = ( e->c - e->b*0.f ) / e->a ;
  float yint = ( e->c - e->a*0.f ) / e->b ;
  if( xint > 0 ) // can use
    outputBisectors.push_back( Vector2f( xint, 0 ) ) ;
  else
  {
    // use a pt at top
    xint = ( e->c - e->b*h ) / e->a ;
    outputBisectors.push_back( Vector2f( xint, h ) ) ;
  }
  if( yint > 0 )
    outputBisectors.push_back( Vector2f( 0, yint ) ) ;
  else
  {
    yint = ( e->c - e->a*w ) / e->b ;
    outputBisectors.push_back( Vector2f( w, yint ) ) ;
  }
  
}

void out_endpoint(Edge * e)
{
  if( prints )
    printf("e %d %d %d\n", e->edgeNumber,
      e->ep[le] ? e->ep[le]->siteNumber : -1,
      e->ep[re] ? e->ep[re]->siteNumber : -1 );

  // We're going to put out a voronoi edge.
  clip_line( e ) ;
}

void out_vertex( Site * v )
{
  if( debug )
    printf("vertex(%d) at %f %f\n", v->siteNumber, v->coord.x, v->coord.y) ;
  else if( prints )
    printf( "v %f %f\n", v->coord.x, v->coord.y ) ;
  
  // This outputs a VORONOI vertex
  outputVoronoiVertices.push_back( v->coord ) ;
}

void out_site(Site * s)
{
  if( debug )
    printf("site (%d) at %f %f\n", s->siteNumber, s->coord.x, s->coord.y) ;
  else if( prints )
    printf("s %f %f\n", s->coord.x, s->coord.y) ;

  // The site must not be mangled by now
  outputSites.push_back( Vector2f(s->coord.x, s->coord.y) ) ;
  
  // He used circles to show the sites.  but pts will suffice here.
  //circle( s->coord.x, s->coord.y, cradius ) ;
  
}

void out_triple( Site * s1, Site * s2, Site * s3 )
{
  if( debug )
    printf("circle through left=%d right=%d bottom=%d\n", s1->siteNumber, s2->siteNumber, s3->siteNumber) ;
  else if( prints )
    printf("%d %d %d\n", s1->siteNumber, s2->siteNumber, s3->siteNumber) ;
  
  // the sites are not mangled here. I checked.
  outputTris.push_back( new Poly( s1->coord, s2->coord, s3->coord, Vector4f::random() ) ) ;
}

// This clips a little too tightly.
void clip_line( Edge * e )
{
  Site *s1, *s2 ;
  float x1, x2, y1, y2 ;

  if( e->a == 1.0f && e->b >= 0.0f )
  {
    s1 = e->ep[1] ;
    s2 = e->ep[0] ;
  }
  else
  {
    s1 = e->ep[0] ;
    s2 = e->ep[1] ;
  }
  if( e->a == 1.0f )
  {
    y1 = ymin ;
    if( s1 && s1->coord.y > ymin )
    {
      y1 = s1->coord.y ;
    }
    if( y1 > ymax )
    {
      return ;
    }
    x1 = e->c - e->b * y1 ;
    y2 = ymax ;
    if( s2 && s2->coord.y < ymax )
    {
      y2 = s2->coord.y ;
    }
    if (y2 < ymin)
    {
      return ;
    }
    x2 = e->c - e->b * y2 ;
    if(((x1 > xmax) && (x2 > xmax)) || ((x1 < xmin) && (x2 < xmin)))
    {
      return ;
    }
    if( x1 > xmax )
    {
      x1 = xmax ;
      y1 = (e->c - x1) / e->b ;
    }
    if( x1 < xmin )
    {
      x1 = xmin ;
      y1 = (e->c - x1) / e->b ;
    }
    if( x2 > xmax )
    {
      x2 = xmax ;
      y2 = (e->c - x2) / e->b ;
    }
    if( x2 < xmin )
    {
      x2 = xmin ;
      y2 = (e->c - x2) / e->b ;
    }
  }
  else
  {
    x1 = xmin ;
    if( s1 && s1->coord.x > xmin )
    {
      x1 = s1->coord.x ;
    }
    if( x1 > xmax )
    {
      return ;
    }
    y1 = e->c - e->a * x1 ;
    x2 = xmax ;
    if( s2 != NULL && s2->coord.x < xmax )
    {
      x2 = s2->coord.x ;
    }
    if( x2 < xmin )
    {
      return ;
    }
    y2 = e->c - e->a * x2 ;
    if (((y1 > ymax) && (y2 > ymax)) || ((y1 < ymin) && (y2 <ymin)))
    {
      return ;
    }
    if( y1 > ymax )
    {
      y1 = ymax ;
      x1 = (e->c - y1) / e->a ;
    }
    if (y1 < ymin)
    {
      y1 = ymin ;
      x1 = (e->c - y1) / e->a ;
    }
    if (y2 > ymax)
    {
      y2 = ymax ;
      x2 = (e->c - y2) / e->a ;
    }
    if (y2 < ymin)
    {
      y2 = ymin ;
      x2 = (e->c - y2) / e->a ;
    }
  }

  outputVoronoiEdges.push_back( Vector2f( x1, y1 ) ) ;
  outputVoronoiEdges.push_back( Vector2f( x2, y2 ) ) ;
}












// "GEOMETRY"
Edge *bisect(Site * s1, Site * s2)
{
  float dx, dy, adx, ady ;
  Edge * newedge = (Edge *)edgeFreelist->getfree( sqrt_nsites ) ;
  newedge->reg[0] = s1 ;
  newedge->reg[1] = s2 ;
  ref(s1) ;
  ref(s2) ;
  newedge->ep[0] = newedge->ep[1] = NULL ;
  dx = s2->coord.x - s1->coord.x ;
  dy = s2->coord.y - s1->coord.y ;
  adx = dx>0 ? dx : -dx ;
  ady = dy>0 ? dy : -dy ;
  newedge->c = s1->coord.x * dx + s1->coord.y * dy + (dx*dx + dy*dy) * 0.5 ;
  if( adx > ady )
  {
    newedge->a = 1.0 ;
    newedge->b = dy/dx ;
    newedge->c /= dx ;
  }
  else
  {
    newedge->b = 1.0 ;
    newedge->a = dx/dy ;
    newedge->c /= dy ;
  }
  newedge->edgeNumber = nedges ;
  out_bisector( newedge ) ; // draw the raw bisector while we're here.
  // voronoi edges fall along bisectors, but they must be clipped.
  nedges++ ;
  return newedge ;
}

Site *intersect(Halfedge * el1, Halfedge * el2)
{
  Edge * e1, * e2, * e ;
  Halfedge * el ;
  float d, xint, yint ;
  int right_of_site ;
  Site * v ;

  e1 = el1->ELedge ;
  e2 = el2->ELedge ;
  if( !e1 || !e2 )
  {
    return NULL ;
  }
  if( e1->reg[1] == e2->reg[1] )
  {
    return NULL ;
  }
  d = (e1->a * e2->b) - (e1->b * e2->a) ;
  if( -1.0e-10f < d && d < 1.0e-10f )
  {
    return NULL ;
  }
  xint = (e1->c * e2->b - e2->c * e1->b) / d ;
  yint = (e2->c * e1->a - e1->c * e2->a) / d ;
  if ((e1->reg[1]->coord.y < e2->reg[1]->coord.y) ||
    (e1->reg[1]->coord.y == e2->reg[1]->coord.y &&
    e1->reg[1]->coord.x < e2->reg[1]->coord.x))
  {
    el = el1 ;
    e = e1 ;
  }
  else
  {
    el = el2 ;
    e = e2 ;
  }
  right_of_site = (xint >= e->reg[1]->coord.x) ;
  if ((right_of_site && (el->ELpm == le)) ||
    (!right_of_site && (el->ELpm == re)))
  {
    return NULL ;
  }
  v = (Site *)siteFreelist->getfree( sqrt_nsites ) ;
  v->refcnt = 0 ;
  v->coord.x = xint ;
  v->coord.y = yint ;
  return (v) ;
}

// returns 1 if p is to right of halfedge e
int right_of(Halfedge * el, Vector2f * p)
{
  Edge * e ;
  Site * topsite ;
  int right_of_site, above, fast ;
  float dxp, dyp, dxs, t1, t2, t3, yl ;

  e = el->ELedge ;
  topsite = e->reg[1] ;
  right_of_site = (p->x > topsite->coord.x) ;
  if( right_of_site && el->ELpm == le )
  {
    return 1 ;
  }
  if( !right_of_site && el->ELpm == re )
  {
    return 0 ;
  }
  if( e->a == 1.0f )
  {
    dyp = p->y - topsite->coord.y ;
    dxp = p->x - topsite->coord.x ;
    fast = 0 ;
    if ((!right_of_site & (e->b < 0.0)) ||
      (right_of_site & (e->b >= 0.0)))
    {
      fast = above = (dyp >= e->b*dxp) ;
    }
    else
    {
      above = ((p->x + p->y * e->b) > (e->c)) ;
      if (e->b < 0.0)
      {
        above = !above ;
      }
      if (!above)
      {
        fast = 1 ;
      }
    }
    if (!fast)
    {
      dxs = topsite->coord.x - (e->reg[0])->coord.x ;
      above = (e->b * (dxp*dxp - dyp*dyp))
        <
        (dxs * dyp * (1.0 + 2.0 * dxp /
        dxs + e->b * e->b)) ;
      if (e->b < 0.0)
      {
        above = !above ;
      }
    }
  }
  else  // e->b == 1.0f
  {
    yl = e->c - e->a * p->x ;
    t1 = p->y - yl ;
    t2 = p->x - topsite->coord.x ;
    t3 = yl - topsite->coord.y ;
    above = ((t1*t1) > ((t2 * t2) + (t3 * t3))) ;
  }
  return (el->ELpm == le ? above : !above) ;
}

void endpoint( Edge * e, int lr, Site * s )
{
  e->ep[lr] = s ;
  ref(s) ;
  if( ! e->ep[re-lr] )
  {
    return ;
  }
  out_endpoint(e) ;
  deref(e->reg[le]) ;
  deref(e->reg[re]) ;
  edgeFreelist->makefree( (Freenode *)e ) ;
}

float dist(Site * s, Site * t)
{
  float dx,dy ;

  dx = s->coord.x - t->coord.x ;
  dy = s->coord.y - t->coord.y ;
  return sqrtf(dx*dx + dy*dy) ;
}

// This outputs a VORONOI vertex
void makevertex(Site * v)
{
  v->siteNumber = nvertices++ ;
  out_vertex(v) ;
}

void deref(Site * v)
{
  if( --(v->refcnt) == 0 )
  {
    siteFreelist->makefree( (Freenode *)v ) ;
  }
}

void ref(Site * v)
{
  ++(v->refcnt) ;
}







// "EDGELIST"
int ELhashsize ;
Freelist *halfedgeFreelist ;
Halfedge **ELhash ;
int ntry, totalsearch ;

void ELinitialize()
{
  halfedgeFreelist = new Freelist( sizeof(Halfedge) ) ;
  ELhashsize = 2 * sqrt_nsites ;
  ELhash = (Halfedge **)malloc( sizeof(*ELhash) * ELhashsize) ;
  for( int i = 0  ; i < ELhashsize ; i++ )
  {
    ELhash[i] = NULL ;
  }
  ELleftend = HEcreate(NULL, 0) ;
  ELrightend = HEcreate(NULL, 0) ;
  ELleftend->ELleft = NULL ;
  ELleftend->ELright = ELrightend ;
  ELrightend->ELleft = ELleftend ;
  ELrightend->ELright = NULL ;
  ELhash[0] = ELleftend ;
  ELhash[ELhashsize-1] = ELrightend ;
}

Halfedge *HEcreate(Edge * e, int pm)
{
  Halfedge * answer ;

  answer = (Halfedge *)halfedgeFreelist->getfree( sqrt_nsites ) ;
  answer->ELedge = e ;
  answer->ELpm = pm ;
  answer->PQnext = NULL ;
  answer->vertex = NULL ;
  answer->ELrefcnt = 0 ;
  return answer ;
}

void ELinsert(Halfedge * lb, Halfedge * newEdge)
{
  newEdge->ELleft = lb ;
  newEdge->ELright = lb->ELright ;
  lb->ELright->ELleft = newEdge ;
  lb->ELright = newEdge ;
}

// Get entry from hash table, pruning any deleted nodes
Halfedge *ELgethash( int b )
{
  Halfedge * he ;

  if( b < 0 || b >= ELhashsize )
  {
    printf( "ERROR: ELgethash. %d out of bounds", b ) ;
    return NULL ;
  }

  he = ELhash[b] ;
  if( !he || he->ELedge != (Edge *)DELETED )
  {
    // If he doesn't exist, or his edge has not been deleted yet, you get HE back.
    return he ;
  }

  // Hash table points to deleted half edge.  Patch as necessary.
  ELhash[b] = NULL ;
  if( --(he->ELrefcnt) == 0 )
  {
    halfedgeFreelist->makefree( (Freenode *)he ) ;
  }
  return NULL ;
}

Halfedge *ELleftbnd( Vector2f * p )
{
  int i, bucket ;
  Halfedge * he ;

  // Use hash table to get close to desired halfedge
  bucket = ( p->x - xmin ) / deltax * ELhashsize ;
  if (bucket < 0)
  {
    bucket = 0 ;
  }
  if (bucket >= ELhashsize)
  {
    bucket = ELhashsize - 1 ;
  }
  he = ELgethash(bucket) ;
  if  (he == (Halfedge *)NULL)
  {
    for (i = 1 ; 1 ; i++)
    {
      if( he = ELgethash( bucket-i ) )
      {
        break ;
      }
      if( he = ELgethash( bucket+i ) )
      {
        break ;
      }
    }
    totalsearch += i ;
  }
  ntry++ ;
  /* Now search linear list of halfedges for the corect one */
  if( he == ELleftend || (he != ELrightend && right_of(he,p)) )
  {
    do  {
      he = he->ELright ;
    } while( he != ELrightend && right_of(he,p) ) ;
    he = he->ELleft ;
  }
  else
  {
    do  {
      he = he->ELleft ;
    } while( he != ELleftend && !right_of(he,p) ) ;
  }
  
  // Update hash table and reference counts
  if( bucket > 0 && (bucket < ELhashsize-1) )
  {
    if( ELhash[bucket] )
    {
      (ELhash[bucket]->ELrefcnt)-- ;
    }
    ELhash[bucket] = he ;
    (ELhash[bucket]->ELrefcnt)++ ;
  }
  return he ;
}




} ;



#endif  


