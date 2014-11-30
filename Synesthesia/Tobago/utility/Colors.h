#ifndef COLORS_H
#define COLORS_H

//http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
#include <iostream>

typedef struct {
    float r;       // percent
    float g;       // percent
    float b;       // percent
} rgb;

typedef struct {
    float h;       // angle in degrees
    float s;       // percent
    float v;       // percent
} hsv;

typedef struct {
    double L;
    double a;
    double b;
} Lab;


#define CMIN(a,b) (((a)<(b))?(a):(b))
#define CMAX(a,b) (((a)>(b))?(a):(b))

static Lab      rgb2Lab(rgb in);
static hsv      rgb2hsl(rgb in);
static hsv      rgb2hsv(rgb in);
static rgb      hsv2rgb(hsv in);

Lab rgb2Lab(rgb in) {
    Lab out;
    double x,y,z;
    x = 0.412453*in.r+0.357580*in.g+0.180423*in.b;
    y = 0.212671*in.r+0.715160*in.g+0.072169*in.b;
    z = 0.019334*in.r+0.119193*in.g+0.950227*in.b;

    double ref_x = 95.047;
    double ref_y = 100.000;
    double ref_z = 108.883;

    double var_x = x/ref_x;
    double var_y = y/ref_y;
    double var_z = z/ref_z;

    if(var_x > 0.008856) var_x = pow(var_x, 1.0/3.0);
    else var_x = (7.787*var_x)+(16.0/116.0);
    if(var_y > 0.008856) var_y = pow(var_y, 1.0/3.0);
    else var_y = (7.787*var_y)+(16.0/116.0);
    if(var_z > 0.008856) var_z = pow(var_z, 1.0/3.0);
    else var_z = (7.787*var_z)+(16.0/116.0);

    out.L = (116*var_y) - 16;
    out.a = 500*(var_x-var_y);
    out.b = 200*(var_y-var_z);

    return out;
}

hsv rgb2hsl(rgb in) {
   double themin,themax,delta;
   hsv out;

   themin = CMIN(in.r,CMIN(in.g,in.b));
   themax = CMAX(in.r,CMAX(in.g,in.b));
   delta = themax - themin;
   out.v = (themin + themax) / 2;
   out.s = 0;
   if (out.v > 0 && out.v < 1)
      out.s = delta / (out.v < 0.5 ? (2*out.v) : (2-2*out.v));
   out.h = 0;
   if (delta > 0) {
      if (themax == in.r && themax != in.g)
         out.h += (in.g - in.b) / delta;
      if (themax == in.g && themax != in.b)
         out.h += (2 + (in.b - in.r) / delta);
      if (themax == in.b && themax != in.r)
         out.h += (4 + (in.r - in.g) / delta);
      out.h *= 60;
   }
   if(out.h<0.0) out.h += 360;
   return(out);
}

hsv rgb2hsv(rgb in) {
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, v is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }

    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
        if( in.g >= max )
            out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
        else
            out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    if((out.h > 360.0 || out.h < 0.0) && !std::isnan(out.h)) {
        std::cerr << "COLOR CONVERSION ERROR" << std::endl;
    }
    return out;
}

rgb hsv2rgb(hsv in) {
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

#endif // COLORS_H
