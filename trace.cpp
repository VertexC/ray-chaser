#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include <iostream>
//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int reflect_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres *sph)
{

  // I = global_ambient + local_ambient + f_decay(diffuse + specular)

  // global ambient
  RGB_float ga = {global_ambient[0] * sph->reflectance,
                  global_ambient[1] * sph->reflectance,
                  global_ambient[2] * sph->reflectance};
  // local ambient
  RGB_float la = {light1_ambient[0] * sph->mat_ambient[0],
                  light1_ambient[1] * sph->mat_ambient[1],
                  light1_ambient[2] * sph->mat_ambient[2]};

  // vector to light
  Vector l = get_vec(q, light1);
  float distance = vec_len(l);
  normalize(&l);

  if (shadow_on && check_sphere_shadow(q, l, scene))
  {
    // shadow ray
    RGB_float color = {ga.r + la.r,
                       ga.g + la.g,
                       ga.b + la.b};
    return color;
  }
  else
  {
    // parameter for diffuse and specular
    float decay = decay_a + decay_b * distance + decay_c * pow(distance, 2);
    decay = 1.0;
    float nl = vec_dot(surf_norm, l);
    float theta = vec_dot(surf_norm, l);
    if (theta < 0)
      theta = 0;
    Vector r = Vector{
        2.0 * theta * surf_norm.x - l.x,
        2.0 * theta * surf_norm.y - l.y,
        2.0 * theta * surf_norm.z - l.z};
    normalize(&r);
    float rv = vec_dot(r, v);
    float rvN = pow(rv, sph->mat_shineness);

    // diffuse
    RGB_float diffuse = {light1_diffuse[0] * sph->mat_diffuse[0] * nl / decay,
                         light1_diffuse[1] * sph->mat_diffuse[1] * nl / decay,
                         light1_diffuse[2] * sph->mat_diffuse[2] * nl / decay};

    // specular
    RGB_float specular = {
        light1_specular[0] * sph->mat_specular[0] * rvN / decay,
        light1_specular[1] * sph->mat_specular[1] * rvN / decay,
        light1_specular[2] * sph->mat_specular[2] * rvN / decay};

    RGB_float color = {ga.r + la.r + diffuse.r + specular.r,
                       ga.g + la.g + diffuse.g + specular.g,
                       ga.b + la.b + diffuse.b + specular.b};

    return color;
  }
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point eye_pos, Vector ray, int step)
{
  RGB_float color = background_clr;
  // get the intersection point and sphere
  Point *point = new Point;
  Spheres *sphere = intersect_scene(eye_pos, ray, scene, point);

  if (sphere != NULL)
  {
    Vector view = get_vec(*point, eye_pos);
    normalize(&view);
    Vector surf_norm = sphere_normal(*point, sphere);
    color = phong(*point, view, surf_norm, sphere);

    if (step > 0)
    {
      // calculate the reflected ray
      float theta = vec_dot(view, surf_norm);
      if (theta < 0)
        theta = 0;
      Vector reflect_view = {
          2 * theta * surf_norm.x - view.x,
          2 * theta * surf_norm.y - view.y,
          2 * theta * surf_norm.z - view.z,
      };
      normalize(&reflect_view);
      RGB_float reflect_color = recursive_ray_trace(*point, reflect_view, step - 1);
      reflect_color = clr_scale(reflect_color, sphere->reflectance);

      color = clr_add(color, reflect_color);
    }
  }

  return color;
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace()
{
  int i, j;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  for (i = 0; i < win_height; i++)
  {
    for (j = 0; j < win_width; j++)
    {
      ray = get_vec(eye_pos, cur_pixel_pos);

      //
      // You need to change this!!!
      //
      ret_color = recursive_ray_trace(eye_pos, ray, step_max);
      // ret_color = background_clr; // just background for now

      // Parallel rays can be cast instead using below
      //
      // ray.x = ray.y = 0;
      // ray.z = -1.0;
      // ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

      // Checkboard for testing
      // RGB_float clr = {float(i / 32), 0, float(j / 32)};
      // ret_color = clr;

      frame[i][j][0] = GLfloat(ret_color.r);
      frame[i][j][1] = GLfloat(ret_color.g);
      frame[i][j][2] = GLfloat(ret_color.b);

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;
  }
}
