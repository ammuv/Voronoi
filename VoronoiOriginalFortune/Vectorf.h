#ifndef VECTORF_H
#define VECTORF_H

// 
#define EPS_MIN 1e-6f

inline float& clamp( float& x, float minVal, float maxVal ) {
  if( x < minVal ) x = minVal ;
  else if( x > maxVal ) x = maxVal ;
  return x ;
}

// crummy resolution, but will do
inline float randFloat()
{
  return rand()/(float)RAND_MAX ;
}

// -1,1 => -1 + ( rand between 0 and 2 )
inline float randFloat( float low, float high )
{
  return low + (high-low)*randFloat() ;
}

// between low and (high-1)
inline int randInt( int low, int high )
{
  return low + rand() % (high-low) ;
}

struct Vector2f
{
  float x,y;
  Vector2f():x(0.f),y(0.f){}
  Vector2f( float ix, float iy ):x(ix),y(iy){}
  //Vector2f( const CGPoint& o ):x( o. {  }
  Vector2f( float iv ):x(iv),y(iv){}
  //CONST
  inline void print() const {
    printf( "(%.2f %.2f) ",x,y ) ;
  }
  inline void print( const char* msg ) const {
    printf( "%s (%.2f %.2f)",msg,x,y ) ;
  }
  inline void println() const {
    printf( "(%.2f %.2f)\n",x,y ) ;
  }
  inline void println( const char* msg ) const {
    printf( "%s (%.2f %.2f)\n",msg,x,y ) ;
  }
  inline Vector2f operator+( const Vector2f& o ) const {
    return Vector2f(x+o.x,y+o.y);
  }
  inline Vector2f operator-() const {
    return Vector2f(-x,-y);
  }
  inline Vector2f operator-( const Vector2f& o ) const {
    return Vector2f(x-o.x,y-o.y);
  }
  
  inline Vector2f operator*( const Vector2f& o ) const {
    return Vector2f(x*o.x,y*o.y);
  }
  inline Vector2f operator*( float s ) const {
    return Vector2f(x*s,y*s);
  }
  inline Vector2f operator/( const Vector2f& o ) const {
    return Vector2f(x/o.x,y/o.y);
  }
  inline Vector2f operator/( float s ) const {
    return Vector2f(x/s,y/s);
  }
  inline float cross( const Vector2f& o ) const {
    return x*o.y-y*o.x ;
  }
  inline float dot( const Vector2f& o ) const {
    return x*o.x+y*o.y ;
  }
  inline float len() const {
    return sqrtf( x*x+y*y ) ;
  }
  inline float len2() const {
    return x*x+y*y ;
  }
  inline Vector2f normalizedCopy() const {
    return Vector2f( *this ).normalize() ;
  }
  inline float angleWith( const Vector2f& o ) const {
    return acosf( this->normalizedCopy().dot(o.normalizedCopy()) ) ;
  }
  // Returns + if this leads o.
  // more expensive than unsigned angle,
  // (2x acos)
  inline float signedAngleWith( const Vector2f& o ) const {
    float aThis = atan2f( y, x );
    float aO = atan2f( o.y, o.x ) ;
    
    //info( "lead=%.2f lag=%.2f, ", aThis, aO ) ;
    return aThis - aO ;
    // When 
  }
  // proximity
  inline bool isNear( const Vector2f& o ) const{
    return fabsf(x-o.x)<EPS_MIN && fabsf(y-o.y)<EPS_MIN ;
  }
  inline bool isNear( const Vector2f& o, const Vector2f& maxDiff ) const{
    return fabsf(x-o.x)<maxDiff.x && fabsf(y-o.y)<maxDiff.y ;
  }
  inline bool isCanonical() const {
    return -1.f <= x && x <= 1.f && 
           -1.f <= y && y <= 1.f ;
  }
  #ifndef _WIN32
  inline bool isNaN() const {
    return isnan(x) || isnan(y) ;
  }
  #endif
  // Exact equality
  inline bool operator==( const Vector2f& o ) const{
    return x==o.x && y==o.y ;
  }
  inline bool operator!=( const Vector2f& o ) const{
    return x!=o.x || y!=o.y ;
  }
  
  // Returns TRUE if "this" is closer to A
  // than B
  inline bool isCloserTo( const Vector2f& a, const Vector2f& thanB ){
    return ( *this-a ).len2() < ( *this-thanB ).len2() ;
  }
  
  // gets the tangential and normal components of THIS along fN
  // f MUST BE NORMALIZED.
  // It gives you the component of this along fN.
  // If that's negative, THEN THIS IS MORE THAN 90 DEG FROM FN.
  inline float parallelPerp( const Vector2f& fN, Vector2f &vParallel, Vector2f &vPerp ) const
  {
    // assuming fN is normalized
    float compParallel = fN.dot( *this ) ;
    vParallel = fN * compParallel ;
    vPerp = (*this) - vParallel ;
    return compParallel ;
  }
  
  // The perpendicular vector is 
  inline void parallelPerpComponents( const Vector2f& fN, float &compParallel, float &compPerp ) const
  {
    // assuming fN is normalized
    compParallel = fN.dot( *this ) ;
    compPerp = fN.cross( *this ) ;
  }
  
  // This is the CCW 90 deg rotated perp. ( y,-x ) is CW rotated.
  inline Vector2f getPerpendicular() const {
    return Vector2f( -y,x ) ;
  }
  
  
  
  
  
  //NON-CONST
  inline Vector2f& normalize(){
    float length = len() ;
    
    // I added this debug check man, never take it out.
    if( !length ) {
      puts( "Vector2f::normalize() attempt to divide by 0" ) ;
      return *this ;
    }
    
    return (*this)/=length ;
  }
  inline float safeNormalize(){
    float length = len() ;
    if( length )  (*this)/=length ;
    return length ;
  }
  inline Vector2f& operator+=( const Vector2f& o ){
    x+=o.x,y+=o.y;
    return *this ;
  }
  inline Vector2f& operator-=( const Vector2f& o ){
    x-=o.x,y-=o.y;
    return *this ;
  }
  inline Vector2f& operator*=( const Vector2f& o ){
    x*=o.x,y*=o.y;
    return *this ;
  }
  inline Vector2f& operator*=( float s ){
    x*=s,y*=s;
    return *this ;
  }
  inline Vector2f& operator/=( const Vector2f& o ){
    x/=o.x,y/=o.y;
    return *this ;
  }
  inline Vector2f& operator/=( float s ){
    x/=s,y/=s;
    return *this ;
  }
  inline Vector2f& clampLen( float maxLen ){
    float length = len() ;
    if( length > maxLen ) // also means length > 0, hopefully
      return normalize()*=maxLen ;
    
    return *this ;
  }
  inline Vector2f& clampComponent( float minVal, float maxVal )
  {
    ::clamp( x,minVal,maxVal ) ;
    ::clamp( y,minVal,maxVal ) ;
    return *this ;
  }
  inline Vector2f& clampComponentBelow( float below )
  {
    if( x < below ) x=below ;
    if( y < below ) y=below ;
    return *this ;
  }
  inline Vector2f& clampComponentAbove( float above )
  {
    if( x > above ) x=above ;
    if( y > above ) y=above ;
    return *this ;
  }
  inline Vector2f& clampBelow( const Vector2f& below )
  {
    if( x < below.x ) x=below.x ;
    if( y < below.y ) y=below.y ;
    return *this ;
  }
  inline Vector2f& clampAbove( const Vector2f& above )
  {
    if( x > above.x ) x=above.x ;
    if( y > above.y ) y=above.y ;
    return *this ;
  }
  
} ;


inline Vector2f operator-( const Vector2f& v, float s )
{
  return Vector2f(v.x-s,v.y-s);
}

inline Vector2f operator-( float s, const Vector2f& v )
{
  return Vector2f(s-v.x,s-v.y);
}


 
union Vector4f
{
  struct{ float x,y,z,w ; } ;
  struct{ float r,g,b,a ; } ;
  float elts[4];
  
  Vector4f():x(0.f),y(0.f),z(0.f),w(1.f){}
  Vector4f( float ix, float iy, float iz ):x(ix),y(iy),z(iz),w(1.f){}
  Vector4f( float ix, float iy, float iz, float iw ):x(ix),y(iy),z(iz),w(iw){}
  Vector4f( const Vector2f& v2f ):x(v2f.x),y(v2f.y),z(0.0f),w(1.0f){}
  Vector4f( float iv ):x(iv),y(iv),z(iv),w(iv){}
  
  static inline Vector4f random() { return Vector4f( randFloat(), randFloat(), randFloat(), 1.f ) ;  }
  
  static inline Vector4f random(float min, float max) {
    return Vector4f( randFloat(min,max), randFloat(min,max), randFloat(min,max), 1.f ) ;
  }
  static Vector4f lerp( const Vector4f& A, const Vector4f& B, float t ){
    return A*(1.f-t) + B*t ;
  }
  //CONST
  inline void print() const {
    printf( "(%.2f %.2f %.2f %.2f) ",x,y,z,w ) ;
  }
  inline void println( const char* msg ) const {
    printf( "%s (%.2f %.2f %.2f %.2f)\n",msg,x,y,z,w ) ;
  }
  inline void println() const {
    printf( "(%.2f %.2f %.2f %.2f)\n",x,y,z,w ) ;
  }
  // Good for checking if all 0
  inline bool all( float val ){
    return x==val && y==val && z==val && w==val ;
  }
  inline Vector4f operator+( const Vector4f& o ) const {
    return Vector4f(x+o.x,y+o.y,z+o.z,w+o.w);
  }
  inline Vector4f operator-() const{
    return Vector4f(-x,-y,-z,-w);
  }
  inline Vector4f operator-( const Vector4f& o ) const {
    return Vector4f(x-o.x,y-o.y,z-o.z,w-o.w);
  }
  inline Vector4f operator*( const Vector4f& o ) const {
    return Vector4f(x*o.x,y*o.y,z*o.z,w*o.w);
  }
  inline Vector4f operator*( float s ) const {
    return Vector4f(x*s,y*s,z*s,w*s);
  }
  inline Vector4f operator/( const Vector4f& o ) const {
    return Vector4f(x/o.x,y/o.y,z/o.z,w/o.w);
  }
  inline Vector4f operator/( float s ) const {
    return Vector4f(x/s,y/s,z/s,w/s);
  }
  // 5 op
  inline float dot( const Vector4f& o ) const{
    return x*o.x+y*o.y+z*o.z+w*o.w ;
  }
  
  // proximity
  inline bool isNear( const Vector4f& o ) const{
    return fabsf(x-o.x)<EPS_MIN && fabsf(y-o.y)<EPS_MIN && fabsf(z-o.z)<EPS_MIN && fabsf(w-o.w)<EPS_MIN ;
  }
  inline bool isCanonical() const {
    return -1.f <= x && x <= 1.f && 
           -1.f <= y && y <= 1.f && 
           -1.f <= z && z <= 1.f && 
           -1.f <= w && w <= 1.f ;
  }
  // Exact equality
  inline bool operator==( const Vector4f& o ) const{
    return x==o.x && y==o.y && z==o.z && w==o.w;
  }
  
  //Vector4f toHSV() const {
	//  return Vector4f( Vector3f::RGBtoHSV( x, y, z ), w ) ;
  //}
  //Vector4f toRGB() const {
	//  return Vector4f( Vector3f::HSVtoRGB( x, y, z ), w ) ;
  //}
  
  inline Vector2f& xy(){
    return (Vector2f&)x ;
  }
  
  inline Vector4f& operator+=( const Vector4f& o ){
    x+=o.x,y+=o.y,z+=o.z,w+=o.w;
    return *this ;
  }
  inline Vector4f& operator-=( const Vector4f& o ){
    x-=o.x,y-=o.y,z-=o.z,w-=o.w;
    return *this ;
  }
  inline Vector4f& operator*=( const Vector4f& o ){
    x*=o.x,y*=o.y,z*=o.z,w*=o.w;
    return *this ;
  }
  inline Vector4f& operator*=( float s ){
    x*=s,y*=s,z*=s,w*=s;
    return *this ;
  }
  inline Vector4f& operator/=( const Vector4f& o ){
    x/=o.x,y/=o.y,z/=o.z,w/=o.w;
    return *this ;
  }
  inline Vector4f& operator/=( float s ){
    x/=s,y/=s,z/=s,w/=s;
    return *this ;
  }
  inline Vector4f& clampComponent( float minVal, float maxVal )
  {
    ::clamp( x,minVal,maxVal ) ;
    ::clamp( y,minVal,maxVal ) ;
    ::clamp( z,minVal,maxVal ) ;
    ::clamp( w,minVal,maxVal ) ;
    return *this ;
  }
  
  // You only need __4__ components to do a regular perspective projection, not 16
  static Vector4f persp(float fovyRadians, float aspect, float nearZ, float farZ)
  {
    float yScale = 1.0 / tanf( fovyRadians / 2.0 ) ;
    float xScale = yScale / aspect ;
    
    // This is D3DXMatrixPerspectiveFovRH: http://msdn.microsoft.com/en-us/library/bb205351(VS.85).aspx
    //((TRANSPOSED)):  THE FOLLOWING MATRIX IS COLUMN MAJOR:
    //xScale     0          0              0
    //0        yScale       0              0
    //0        0        zf/(zn-zf)        zn*zf/(zn-zf)
    //0        0           -1              0
    //where:
    //yScale = cot(fovY/2)
    //xScale = yScale / aspect ratio

    // You don't actually need a whole matrix to do
    // perspective projection.  You only need a vec4.
    return Vector4f( xScale,
                     yScale,
                     (farZ) / (nearZ - farZ),
                     (farZ * nearZ) / (nearZ - farZ)
                   ) ;
                   
    // So:
    // xT = v.x * x ;
    // yT = v.y * y ;
    // zT = v.z * z + v.w ;
    // wT = -z ;
    //gl_Position.x = eyeSpace.x * perspProj.x ;
    //gl_Position.y = eyeSpace.y * perspProj.y ;
    //gl_Position.z = eyeSpace.z * perspProj.z + perspProj.w ;
    //gl_Position.w = -eyeSpace.z ;
  }
} ;
 
Vector4f Red( 1,0,0,1 ), DarkRed( 0.35,0,0,1 ),
  Green( 0,1,0,1 ), DarkGreen( 0,0.35,0,1 ),
  Blue( 0,0,1,1 ), DarkBlue( 0,0,0.35,1 ),
  White(1,1,1,1), Gray(0.5,0.5,0.65,1), DarkGray(0.21,0.21,0.21,1), Black(0,0,0,1), 
  Magenta(1,0,1,1), Teal(0,1,1,1), Yellow(1,1,0,1), 
 
  TWhite(1,1,1,0), TBlack(0,0,0,0)
;



struct Circle
{
  Vector2f center ;
  float radius ;

  Circle( Vector2f iCenter, float iRadius ) :
    center( iCenter ), radius( iRadius )
  {
    
  }
} ;



struct VertexPC
{
  Vector2f pos ;
  Vector4f color ;
  
  VertexPC(){}
  VertexPC( float x, float y, float r, float g, float b, float a ):
    pos(x,y), color(r,g,b,a) { }
    
  VertexPC( const Vector2f& iPos, const Vector4f& iColor ) :
    pos( iPos ), color( iColor )
  {
    
  }
  
} ;



struct Poly
{
  vector<Vector2f> pts ;
  vector<Vector4f> colors ;
  
  Poly(){}
  Poly( const Vector2f& a, const Vector2f& b, const Vector2f& c, const Vector4f& color )
  {
    pts.push_back( a ) ;
    pts.push_back( b ) ;
    pts.push_back( c ) ;

    colors.push_back( color ) ;
    colors.push_back( color ) ;
    colors.push_back( color ) ;
  }
 
  void setColor( const Vector4f& color )
  {
    for( int i = 0 ; i < colors.size() ; i++ )
      colors[i] = color ;
  }
 
  void draw()
  {
    if( colors.size() != pts.size() ) {
      printf( "ERROR colors.size( %d ) != pts.size( %d )\n", colors.size(), pts.size() ) ;
      colors.resize( pts.size() ) ; // shouldn't happen, but could.
    }
    //if( !pts.size() ) {
    //  puts( "ERROR NO PTS INSIDE POLYGON" ) ;
    //  return ;
    //}
    else if( pts.size() == 1 )
      glBegin( GL_POINTS ) ;
    else if( pts.size() == 2 )
      glBegin( GL_LINES ) ;
    else
      glBegin( GL_POLYGON ) ;
    
    for( int i = 0 ; i < pts.size() ; i++ )
    {
      glColor4fv( &colors[i].x ) ;
      glVertex2fv( &pts[i].x ) ;
    }
    glEnd() ;
  }
 
  void addPt( const Vector2f &pt, const Vector4f &color )
  {
    pts.push_back( pt ) ;
    colors.push_back( color ) ;
  }
  
  Vector2f getCentroid()
  {
    if( !pts.size() )  {  puts("NO CENTROID"); return 0 ; }

    Vector2f c = pts[0];
    for( int i = 1 ; i < pts.size() ; i++ )
      c+=pts[i] ;
    return c / pts.size() ;
  }

  Vector2f circumcircle( float &r )
  {
    const Vector2f &a=pts[0],&b=pts[1],&c=pts[2];
    float d = 2*(a.x*(b.y-c.y)+b.x*(c.y-a.y)+c.x*(a.y-b.y));

    Vector2f u;
    u.x = a.len2()*(b.y-c.y)+b.len2()*(c.y-a.y)+c.len2()*(a.y-b.y);
    u.y = a.len2()*(c.x-b.x)+b.len2()*(a.x-c.x)+c.len2()*(b.x-a.x);
    u/=d;

    r = (a-u).len() ;
    return u ;
  }

  int pnpoly( const Vector2f& test )
  {
    int i, j, c = 0;
    for (i = 0, j = pts.size()-1; i < pts.size(); j = i++) {
      if ( ((pts[i].y>test.y) != (pts[j].y>test.y)) &&
        (test.x < (pts[j].x-pts[i].x) * (test.y-pts[i].y) / (pts[j].y-pts[i].y) + pts[i].x) )
        c = !c;
    }
    return c;
  } 
} ;


#endif