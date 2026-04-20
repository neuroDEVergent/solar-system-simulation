#ifndef PLANETSPECIFICATION_HPP
#define PLANETSPECIFICATION_HPP

#include <glm/gtc/matrix_transform.hpp>
#include <LoadFiles.hpp>

typedef struct Planet
{
  float realDiameter;
  float realDistance;
  float normalizedDiameter;
  float normalizedDistance;
  float day;
  float normalizedDay;
  float year;
  float normalizedYear;
  unsigned int texture;
} Planet;

void initializePlanets(Planet*, unsigned int);

#endif
