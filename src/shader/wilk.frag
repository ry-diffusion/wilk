#version 400 core

uniform double scale;
uniform double maxIterations;
uniform dvec2 loc;
uniform dvec2 limits;

layout (location = 0) out vec4 fragColor;


dvec2 complexSquared(dvec2 z) {
  return dvec2(
    z.x * z.x - z.y * z.y,
    2.0 * z.x * z.y
  );
}

void main() {
  dvec2 limits = limits*scale;
  dvec2 c = dvec2((gl_FragCoord.x - limits.x / 2) * 4.0 / limits.x + loc.x,
                (gl_FragCoord.y - limits.y / 2) * 4.0 / limits.y + loc.y);
  
  dvec2 z = c;
  int it = 0;
  
  while (z.x * z.x + z.y <= 4 && it < maxIterations) 
  {
    z = complexSquared(z) + c;

    it++;
  }

  
  fragColor = vec4(it / maxIterations, 0.0, 0.0, 1.0);
}
