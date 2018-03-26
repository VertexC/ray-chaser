#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit, bool far = false)
{
  /*
    (1)P = O + ku
    (2)sphere: (Center - P).^2 = R^2
    solve the equation system above, get k1, k2.
    P = O + min(k1,k2)u 
  */

  float A = vec_dot(u, u);
  Vector SO = get_vec(sph->center, o);
  float B = 2.0 * vec_dot(SO, u);
  float C = vec_dot(SO, SO) - pow(sph->radius, 2);

  float delta = pow(B, 2) - 4 * A * C;
  if (delta < 0.01)
  {
    // no intersection
    return -1.0;
  }

  float k1 = (-B + sqrt(delta)) / (2 * A);
  float k2 = (-B - sqrt(delta)) / (2 * A);

  if (k2 < -0.01)
  {
    return -1.0;
  }
  // ? what if o inside the sphere
  // if (k2 < 0.01)
  // std::cout << k2 << std::endl;
  Vector ku;
  if (far)
  {
    ku = Vector{u.x * k1, u.y * k1, u.z * k1};
  }
  else
  {
    ku = Vector{u.x * k2, u.y * k2, u.z * k2};
  }

  // set hit
  if (hit != NULL)
  {
    hit->x = ku.x + o.x;
    hit->y = ku.y + o.y;
    hit->z = ku.z + o.z;
  }
  // calculate the distance
  return vec_len(ku);
}
/**********************************************************************
 * This function check wheter shadow ray itersect any sphere
 * if intersect, return ture
 **********************************************************************/
bool check_sphere_shadow(Point o, Vector u, Spheres *sph)
{
  Spheres *ptr = sph;
  while (ptr != NULL)
  {

    float A = vec_dot(u, u);
    Vector SO = get_vec(ptr->center, o);
    float B = 2.0 * vec_dot(SO, u);
    float C = vec_dot(SO, SO) - pow(ptr->radius, 2);

    float delta = pow(B, 2) - 4 * A * C;

    float k1 = (-B + sqrt(delta)) / (2 * A);
    float k2 = (-B - sqrt(delta)) / (2 * A);

    if (delta > 0 && k1 > 0.1 && k2 > 0.1)
    {
      return true;
    }
    ptr = ptr->next;
  }

  return false;
}
/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector u, Spheres *sph, Point *hit)
{
  // for every sph, find the closet distance of intersection
  Spheres *closest = NULL;
  Spheres *ptr = sph;
  float min_distance = 10000000.0;

  while (ptr != NULL)
  {
    float distance = intersect_sphere(o, u, ptr, NULL);
    if (distance > 0)
    {
      if (distance < min_distance)
      {
        min_distance = distance;
        closest = ptr;
      }
    }

    ptr = ptr->next;
  }

  // set the hit
  if (closest != NULL)
  {
    intersect_sphere(o, u, closest, hit);
  }

  return closest;
}

/*********************************************************************
 * This function is to get the refracted ray out of sphere
 **********************************************************************/
Vector refracted_out_sphere(Point p, Vector v, Spheres *sphere, Point *refract_point)
{
  Vector refracted_in = refract_sphere(p, v, sphere);
  normalize(&refracted_in);
  // std::cout << "3" << std::endl;
  Point *out_p = new Point;

  intersect_sphere(p, refracted_in, sphere, out_p, true);
  // std::cout << "4" << std::endl;

  Vector refracted_out = refract_sphere(*out_p, vec_scale(refracted_in, -1), sphere);
  normalize(&refracted_out);
  // std::cout << "5" << std::endl;
  refract_point->x = out_p->x;
  refract_point->y = out_p->y;
  refract_point->z = out_p->z;
  return refracted_out;
}

/*********************************************************************
 * A internal function for refraction.
 **********************************************************************/
Vector refract_sphere(Point p, Vector v, Spheres *sphere)
{
  // // std::cout << "11" << std::endl;

  // Vector n = sphere_normal(p, sphere);
  // // std::cout << "10" << std::endl;

  // float r = 1.0f / sphere->refraction;
  // // std::cout << "6" << std::endl;

  // if (vec_dot(n, v) > 0.0)
  // {
  //   // the refracted ray is tobe inside sphere
  //   // r = 1.0f / sphere->refraction;
  // }
  // else
  // {
  //   // the refracted ray is tobe outof sphere
  //   // r = sphere->refraction;
  //   n = vec_scale(n, -1);
  // }

  // float delta = 1.0 - pow(r, 2) * (1 - pow(vec_dot(v, n), 2));
  // // std::cout << "7" << std::endl;

  // Vector refracted_v = vec_minus(vec_scale(n, vec_dot(n, v) - sqrt(delta)), v);
  // normalize(&refracted_v);
  // // std::cout << "8" << std::endl;

  // return refracted_v;

  Vector outRay;
  normalize(&v);
  Vector surf_norm = sphere_normal(p, sphere);
  float r1;
  float r2;
  if (vec_dot(v, surf_norm) > 0)
  {
    // into object
    r1 = 1.0;
    r2 = 1.5;
  }
  else
  {
    // outside object
    surf_norm = vec_scale(surf_norm, -1);
    r1 = 1.5;
    r2 = 1.0;
  }

  float ratio = r1 / r2;

  float delta = 1 - pow(ratio, 2) * (1 - pow(vec_dot(v, surf_norm), 2));

  outRay = vec_minus(vec_scale(surf_norm, ratio * vec_dot(v, surf_norm) - sqrt(delta)), vec_scale(v, ratio));
  normalize(&outRay);
  return outRay;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
                    float dif[], float spe[], float shine,
                    float refr, float refl, int sindex)
{
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->refraction = refr;
  new_sphere->next = NULL;

  if (slist == NULL)
  { // first object
    slist = new_sphere;
  }
  else
  { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph)
{
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}
