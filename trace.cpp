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
    float nl = vec_dot(surf_norm, l) > 0 ? vec_dot(surf_norm, l) : 0;

    float theta = vec_dot(surf_norm, l);

    Vector r = Vector{
        2.0 * theta * surf_norm.x - l.x,
        2.0 * theta * surf_norm.y - l.y,
        2.0 * theta * surf_norm.z - l.z};
    normalize(&r);

    float rv = vec_dot(r, v) > 0 ? vec_dot(r, v) : 0;
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

/*********************************************************************
 * Phong illumination - for board
 *********************************************************************/
RGB_float phong_board(Point q, Vector v, Vector surf_norm)
{
  // I = global_ambient + local_ambient + f_decay(diffuse + specular)
  float reflectance = 0.32;
  int mat_shineness = 1;
  float mat_ambient[3] = {0.2, 0.2, 0.2};
  float mat_diffuse[3] = {0.5, 0.5, 0.5};
  float mat_specular[3] = {3, 3, 3};

  // global ambient
  RGB_float ga = {global_ambient[0] * reflectance,
                  global_ambient[1] * reflectance,
                  global_ambient[2] * reflectance};
  // local ambient
  RGB_float la = {light1_ambient[0] * mat_ambient[0],
                  light1_ambient[1] * mat_ambient[1],
                  light1_ambient[2] * mat_ambient[2]};

  // vector to light
  Vector l = get_vec(q, light1);
  float distance = vec_len(l);
  normalize(&l);

  // parameter for diffuse and specular
  float decay = decay_a + decay_b * distance + decay_c * pow(distance, 2);
  decay = 1.0;
  float nl = vec_dot(surf_norm, l) > 0 ? vec_dot(surf_norm, l) : 0;

  float theta = vec_dot(surf_norm, l);

  Vector r = Vector{
      2.0 * theta * surf_norm.x - l.x,
      2.0 * theta * surf_norm.y - l.y,
      2.0 * theta * surf_norm.z - l.z};
  normalize(&r);

  float rv = vec_dot(r, v) > 0 ? vec_dot(r, v) : 0;
  float rvN = pow(rv, mat_shineness);

  // diffuse
  RGB_float diffuse = {light1_diffuse[0] * mat_diffuse[0] * nl / decay,
                       light1_diffuse[1] * mat_diffuse[1] * nl / decay,
                       light1_diffuse[2] * mat_diffuse[2] * nl / decay};

  // specular
  RGB_float specular = {
      light1_specular[0] * mat_specular[0] * rvN / decay,
      light1_specular[1] * mat_specular[1] * rvN / decay,
      light1_specular[2] * mat_specular[2] * rvN / decay};

  RGB_float color = {ga.r + la.r + diffuse.r + specular.r,
                     ga.g + la.g + diffuse.g + specular.g,
                     ga.b + la.b + diffuse.b + specular.b};

  return color;
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
      color = clr_scale(color, 3.0);
      Vector l = get_vec(*board_point, light1);

      Vector view = get_vec(*board_point, eye_pos);
      normalize(&view);
      Vector surf_norm = norm_board(*board_point);
      // local phong
      color = clr_add(color, phong_board(*board_point, view, surf_norm));

      float theta = vec_dot(view, surf_norm);
      // if (theta < 0)
      // {
      //   theta = 0;
      // }
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
          diffuse_color = clr_scale(diffuse_color, 0.5);
          color = clr_add(color, diffuse_color);
        }
      }

      if (shadow_on && check_sphere_shadow(*board_point, l, scene))
      {
        color = clr_scale(color, 0.3);
      }

      color = clr_scale(color, 0.5);
    }
  }

  if (sphere != NULL)
  {
    Vector view = get_vec(*sphere_point, eye_pos);
    normalize(&view);
    Vector surf_norm = sphere_normal(*sphere_point, sphere);
    color = phong(*sphere_point, view, surf_norm, sphere);

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
        diffuse_color = clr_scale(diffuse_color, 0.05);
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
      refract_color = clr_scale(refract_color, 1.0);
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

      // super sampling
      if (super_on)
      {
        Point lt = {cur_pixel_pos.x - 0.25 * x_grid_size,
                    cur_pixel_pos.y + 0.25 * y_grid_size,
                    cur_pixel_pos.z};

        ray = get_vec(eye_pos, lt);
        normalize(&ray);
        ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, ray, step_max));

        Point rt = {cur_pixel_pos.x + 0.25 * x_grid_size,
                    cur_pixel_pos.y + 0.25 * y_grid_size,
                    cur_pixel_pos.z};

        ray = get_vec(eye_pos, rt);
        normalize(&ray);
        ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, ray, step_max));

        Point lb = {cur_pixel_pos.x - 0.25 * x_grid_size,
                    cur_pixel_pos.y - 0.25 * y_grid_size,
                    cur_pixel_pos.z};

        ray = get_vec(eye_pos, lb);
        normalize(&ray);
        ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, ray, step_max));

        Point rb = {cur_pixel_pos.x + 0.25 * x_grid_size,
                    cur_pixel_pos.y - 0.25 * y_grid_size,
                    cur_pixel_pos.z};

        ray = get_vec(eye_pos, rb);
        normalize(&ray);
        ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, ray, step_max));

        ret_color = clr_scale(ret_color, 0.2);
      }

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
