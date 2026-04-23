#include <PlanetSpecification.hpp>

void initializePlanets(Planet* planets, unsigned int size)
{
  /* NASA's reference guide
  Planet    Diameter (km) Distance from Sun (km)
  Sun       1,391,400     -
  Mercury   4,879         57,900,000
  Venus     12,104        108,200,000
  Earth     12,756        149,600,000
  Mars      6,792         227,900,000
  Jupiter   142,984       778,600,000
  Saturn    120,536       1,433,500,000
  Uranus    51,118        2,872,500,000
  Neptune   49,528        4,495,100,000
  */
  
  /*
    Day       Length
    Sun       648 hours
    Mercury 	1,408 hours
    Venus 	  5,832 hours
    Earth 	  24 hours
    Mars 	    25 hours
    Jupiter 	10 hours
    Saturn 	  11 hours
    Uranus 	  17 hours
    Neptune 	16 hours 
  */

  /*
   Year     Length
   Sun      0
   Mercury  88 days
   Venus    225 days
   Earth    365 days
   Mars     687 days
   Jupiter  4333 days
   Saturn   10759 days
   Uranus   30687 days
   Neptune  60190 days
  */

  Planet sun = {0};
  sun.realDiameter = 1391400.;
  sun.realDistance = 0.0;
  sun.day = 840.0;
  sun.year = 0.0f;
  sun.texture = loadTexture("./resources/textures/planets/sun-test.jpg");

  Planet mercury = {0};
  mercury.realDiameter = 4879.;
  mercury.realDistance = 57900000.0;
  mercury.day = 1408.0;
  mercury.year = 88.0;
  mercury.texture = loadTexture("./resources/textures/planets/mercury-texture.jpg");

  Planet venus = {0};
  venus.realDiameter = 12104.0;
  venus.realDistance = 108200000.0;
  venus.day = 5832.0;
  venus.year = 225.0;
  venus.texture = loadTexture("./resources/textures/planets/venus-texture.jpg");

  Planet earth = {0};
  earth.realDiameter = 12756.0;
  earth.realDistance = 149600000.0;
  earth.day = 24.0;
  earth.year = 365.0;
  earth.texture = loadTexture("./resources/textures/planets/earth-texture.jpg");
  earth.normalMap = loadTexture("./resources/textures/planets/earth-normal-map.jpg");
  earth.specularMap = loadTexture("./resources/textures/planets/earth-specular-map.jpg");
  earth.nightMap = loadTexture("./resources/textures/planets/earth-nightmap.jpg");
  earth.clouds = loadTexture("./resources/textures/planets/earth-clouds.jpg");

  Planet mars = {0};
  mars.realDiameter = 6792.0;
  mars.realDistance = 227900000.0;
  mars.day = 25.0;
  mars.year = 687.0;
  mars.texture = loadTexture("./resources/textures/planets/mars-texture.jpg");

  Planet jupiter = {0};
  jupiter.realDiameter = 142984.0;
  jupiter.realDistance = 778600000.0;
  jupiter.day = 10.0;
  jupiter.year = 4333.0;
  jupiter.texture = loadTexture("./resources/textures/planets/jupiter-texture.jpg");

  Planet saturn = {0};
  saturn.realDiameter = 120636.0;
  saturn.realDistance = 1433500000.0;
  saturn.day = 11.0;
  saturn.year = 10759.0;
  saturn.texture = loadTexture("./resources/textures/planets/saturn-texture.jpg");

  Planet uranus = {0};
  uranus.realDiameter = 51118.0;
  uranus.realDistance = 2872500000.0;
  uranus.day = 17.0;
  uranus.year = 30687.0;
  uranus.texture = loadTexture("./resources/textures/planets/uranus-texture.jpg");

  Planet neptune = {0};
  neptune.realDiameter = 49528.0;
  neptune.realDistance = 4495100000.0;
  neptune.day = 16.0;
  neptune.year = 60190.0;
  neptune.texture = loadTexture("./resources/textures/planets/neptune-texture.jpg");
  
  Planet planetsTemp[] = {sun, earth, mercury, venus, mars, jupiter, saturn, uranus, neptune};
  
    for (unsigned int i = 0; i < size; i++)
  {
    planetsTemp[i].normalizedDiameter = planetsTemp[i].realDiameter / 12756.0f;
    planetsTemp[i].normalizedDiameter = glm::pow(planetsTemp[i].normalizedDiameter, 0.5f);

    planetsTemp[i].normalizedDistance = planetsTemp[i].realDistance / 149600000.0f; 
    planetsTemp[i].normalizedDistance *= 1000.0f;
    planetsTemp[i].normalizedDistance = glm::pow(planetsTemp[i].normalizedDistance, 0.5f);

    planetsTemp[i].normalizedYear = planetsTemp[i].year / 365.0f;
    planetsTemp[i].normalizedDay = planetsTemp[i].day / 24.0f;
    planetsTemp[i].normalizedDay = planetsTemp[i].year / planetsTemp[i].normalizedDay * planetsTemp[i].normalizedYear;
    
    planets[i] = planetsTemp[i];
  }

}

