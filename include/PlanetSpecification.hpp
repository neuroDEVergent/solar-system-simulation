#ifndef PLANETSPECIFICATION_HPP
#define PLANETSPECIFICATION_HPP

#include <glm/gtc/matrix_transform.hpp>
#include <LoadFiles.hpp>

typedef struct Planet
{
  double realDiameter;
  double realDistance;
  double normalizedDiameter;
  double normalizedDistance;
  double day;
  double normalizedDay;
  double year;
  double normalizedYear;
  unsigned int texture;
  unsigned int normalMap;
  unsigned int specularMap;
  unsigned int nightMap;
  unsigned int clouds;
} Planet;

void initializePlanets(Planet*, unsigned int);

#endif
