#version 460 core

uniform float scale;
uniform float maxIterations;
uniform vec2 loc;
uniform vec2 limits;

layout (location = 0) out vec4 fragColor;


vec2 complexSquared(vec2 z) {
  return vec2(
    z.x * z.x - z.y * z.y,
    2.0 * z.x * z.y
  );
}

#define GLX(n) n/255

void main() {
  vec2 limits = limits*scale;
  vec2 c = vec2((gl_FragCoord.x - limits.x / 2) * 4.0 / limits.x + loc.x,
                (gl_FragCoord.y - limits.y / 2) * 4.0 / limits.y + loc.y);
  
  
  vec2 z = c;
  int it = 0;
  while (z.x * z.x + z.y <= 4 && it < maxIterations) 
  {
    z = complexSquared(z) + c;

    it++;
  }

  
  fragColor = vec4(0.0, 0.0, int(maxIterations - it) % 256, 1.0);
}
