#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include "board.h"
#include <iostream>
#include <cstdlib>
//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];

extern float image_width;
extern float image_height;

extern int stochastic_count;

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
extern int board_on;
extern int step_max;
extern int refract_on;
extern int stochastic_on;
extern int super_on;

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

RGB_float phong1(Point Intersect_point, Vector View_v, Vector surf_norm, Spheres *sph)
{

  RGB_float C = {0, 0, 0}; //initialize

  // Phong's local illumination model
  // illumination = Global_ambient + ambient + (decay * diffiuse) + (decay * specular)
  // C = (1/(a+bd+cd^2)) * (Id * Kd *(n*l)) + (1/(a+bd+cd^2)) * (Is * Ks *(r*v)^N)
  // A = Iga * Kga + Ia * Ka
  // I = C + A

  // turn p and light1 into vector l
  Vector l = get_vec(Intersect_point, light1);
  // d is the distance between the light source and the point on the object
  float d = vec_len(l);

  normalize(&l);

  // 1/(a+bd+cd^2)
  float abcd = 1 / (decay_a + decay_b * d + decay_c * pow(d, 2));

  // Diffuse with attenuation
  // (1/(a+bd+cd^2)) * (Id * Kd *(n*l))
  float nl = vec_dot(surf_norm, l); // (n*l)
  C.r += abcd * (light1_diffuse[0] * sph->mat_diffuse[0] * nl);
  C.g += abcd * (light1_diffuse[1] * sph->mat_diffuse[1] * nl);
  C.b += abcd * (light1_diffuse[2] * sph->mat_diffuse[2] * nl);

  // / get refected ray r
  float angle = vec_dot(surf_norm, l);
  if (angle < 0)
    angle = 0;
  Vector r = vec_minus(vec_scale(surf_norm, 2 * angle), l);
  normalize(&r);

  // N is the shininess parameter for the object
  int N = sph->mat_shineness;

  // compute (r*v)^N
  float rv = vec_dot(r, View_v);
  float rvn = pow(rv, N);

  // Specular with attenuation
  // (1/(a+bd+cd^2)) * (Is * Ks *(r*v)^N)
  C.r += abcd * (light1_specular[0] * sph->mat_specular[0] * rvn);
  C.g += abcd * (light1_specular[1] * sph->mat_specular[1] * rvn);
  C.b += abcd * (light1_specular[2] * sph->mat_specular[2] * rvn);

  RGB_float A = {0, 0, 0};

  // Global ambient: Iga * Kga
  // Apply thevalues contained in the global ambient array to the sphere
  A.r += global_ambient[0] * sph->reflectance;
  A.g += global_ambient[1] * sph->reflectance;
  A.b += global_ambient[2] * sph->reflectance;

  // Ambient: Ia * Ka
  A.r += light1_ambient[0] * sph->mat_ambient[0];
  A.g += light1_ambient[1] * sph->mat_ambient[1];
  A.b += light1_ambient[2] * sph->mat_ambient[2];

  C.r += A.r;
  C.g += A.g;
  C.b += A.b;

  // Check if shadows are enabled
  // if so, change I to A
  if (shadow_on && check_sphere_shadow(Intersect_point, l, scene))
  {
    // C = A;
    C = clr_scale(C, 0);
  }
  // otherwise I is C

  return C;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point eye_pos, Vector ray, int step)
{
  RGB_float color = background_clr;
  // get the intersection sphere_point and sphere
  Point *sphere_point = new Point;
  Spheres *sphere = intersect_scene(eye_pos, ray, scene, sphere_point);

  Point *board_point = new Point;

  if (board_on)
  {
    if (intersect_board(eye_pos, ray, board_point))
    {
      color = color_board(*board_point);
      Vector l = get_vec(*board_point, light1);

      Vector view = get_vec(*board_point, eye_pos);
      normalize(&view);
      Vector surf_norm = norm_board(*board_point);

      float theta = vec_dot(view, surf_norm);
      if (theta < 0)
      {
        theta = 0;
      }
      Vector reflect_view = {
          2 * theta * surf_norm.x - view.x,
          2 * theta * surf_norm.y - view.y,
          2 * theta * surf_norm.z - view.z};
      normalize(&reflect_view);

      if (step > 0 && reflect_on)
      {

        RGB_float reflect_color = recursive_ray_trace(*board_point, reflect_view, step - 1);
        reflect_color = clr_scale(reflect_color, 1);

        color = clr_add(color, reflect_color);
      }

      if (step > 0 && stochastic_on)
      {
        // randomly generated 5 rays
        for (int i = 0; i < stochastic_count; i++)
        {
          Vector diffuse_view = {
              ((float)rand() / (RAND_MAX)) * 2.0 + -1.0,
              ((float)rand() / (RAND_MAX)) * 2.0 + -1.0,
              ((float)rand() / (RAND_MAX)) * 2.0 + -1.0};
          normalize(&diffuse_view);
          if (vec_dot(diffuse_view, surf_norm) < 0)
          {
            diffuse_view = vec_scale(diffuse_view, -1);
          }
          RGB_float diffuse_color = recursive_ray_trace(*board_point, diffuse_view, step - 1);
          diffuse_color = clr_scale(diffuse_color, 0.3);
          color = clr_add(color, diffuse_color);
        }
      }

      if (shadow_on && check_sphere_shadow(*board_point, l, scene))
      {
        color = clr_scale(color, 0.3);
      }

      color = clr_scale(color, 0.3);
    }
  }

  if (sphere != NULL)
  {
    Vector view = get_vec(*sphere_point, eye_pos);
    normalize(&view);
    Vector surf_norm = sphere_normal(*sphere_point, sphere);
    color = phong1(*sphere_point, view, surf_norm, sphere);

    if (step > 0 && reflect_on)
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
      RGB_float reflect_color = recursive_ray_trace(*sphere_point, reflect_view, step - 1);
      reflect_color = clr_scale(reflect_color, sphere->reflectance);

      color = clr_add(color, reflect_color);
    }

    if (step > 0 && stochastic_on)
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
      // randomly generated 5 rays
      for (int i = 0; i < stochastic_count; i++)
      {
        Vector diffuse_view = {
            ((float)rand() / (RAND_MAX)) * 2.0 + -1.0,
            ((float)rand() / (RAND_MAX)) * 2.0 + -1.0,
            ((float)rand() / (RAND_MAX)) * 2.0 + -1.0};
        normalize(&diffuse_view);
        if (vec_dot(diffuse_view, surf_norm) < 0)
        {
          diffuse_view = vec_scale(diffuse_view, -1);
        }
        RGB_float diffuse_color = recursive_ray_trace(*sphere_point, diffuse_view, step - 1);
        diffuse_color = clr_scale(diffuse_color, 0.1);
        color = clr_add(color, diffuse_color);
      }
    }

    if (step > 0 && refract_on)
    {
      Point *refract_point = new Point;
      // std::cout << "1" << std::endl;
      Vector refract_view = refracted_out_sphere(*sphere_point, view, sphere, refract_point);
      // std::cout << "2" << std::endl;
      RGB_float refract_color = recursive_ray_trace(*refract_point, refract_view, step - 1);
      refract_color = clr_scale(refract_color, 0.32);
      // std::cout << refract_color.r << " " << refract_color.g << " " << refract_color.b << std::endl;
      color = clr_add(color, refract_color);
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
      normalize(&ray);
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
