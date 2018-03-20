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
float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit)
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
  if (delta < 0)
  {
    // no intersection
    return -1.0;
  }

  float k1 = (-B + sqrt(delta)) / (2 * A);
  float k2 = (-B - sqrt(delta)) / (2 * A);
  // ? what if o inside the sphere
  // if (k2 < 0.01)
  // std::cout << k2 << std::endl;
  Vector ku = Vector{u.x * k2, u.y * k2, u.z * k2};
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
  std::cout << "u:" << u.x << " " << u.y << " " << u.z << std::endl;
  std::cout << "o:" << o.x << " " << o.y << " " << o.z << std::endl;
  Spheres *ptr = sph;
  while (ptr != NULL)
  {

    float A = vec_dot(u, u);
    Vector SO = get_vec(ptr->center, o);
    float B = 2.0 * vec_dot(SO, u);
    float C = vec_dot(SO, SO) - pow(ptr->radius, 2);

    // std::cout << "ABC" << A << " " << B << " " << C << std::endl;
    float delta = pow(B, 2) - 4 * A * C;

    float k1 = (-B + sqrt(delta)) / (2 * A);
    float k2 = (-B - sqrt(delta)) / (2 * A);
    // std::cout << " " << delta << " " << k1 << " " << k2 << std::endl;

    if (delta > 0 && B < -1.0)
    {
      // std::cout << "ABC" << A << " " << B << " " << C << std::endl;
      // std::cout << delta << " " << k1 << " " << k2 << std::endl;
    }
    if (delta > 0 && k1 > 0.001 && k2 > 0.001)
    {
      // intersection
      std::cout << " " << delta << " " << k1 << " " << k2 << std::endl;

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

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
                    float dif[], float spe[], float shine,
                    float refl, int sindex)
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
