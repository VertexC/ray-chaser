### Ray-chaser

This is a homework assignment of CMPT361/SFU

#### buil guide
```bash
make
./raycast [-u|-d] step_max <options>
```
specification:
+ u: user scene, d: default scenee
+ step_max: max depth of ray tracing recursion 
+ options:
  - +s: inclusion of shadows
  - +l: inclusion of reflection
  - +r: inclusion of refraction
  - +c: inclusion of chess board pattern
  - +f: enabling diffuse rendering using stochastic ray generations
  - +p: enabling super-sampling 

#### Technique used
- phong illumination
- shadow
- reflection
- refraction
- diffuse rendering using stochastic ray generation
- super-samling

#### screen shot

- ./raycast -d 1 +s +l 
<img src=".\screen_shot\default.bmp" width="40%">

- ./raycast -d 0 +s +c
<img src=".\screen_shot\shiny_board.bmp" width="40%">

- ./raycast -d 1 +s +l +r +c
<img src=".\screen_shot\mine.bmp" width="40%">

- ./raycast -d 1 +s +l +r +c +f
<img src=".\screen_shot\shiny_diffuse_reflection.bmp" width="40%">

- ./raycast -d 1 +s +l +r +c +f +p
<img src=".\screen_shot\shiny_supersampling.bmp" width="40%">

#### Tricks

##### creepy green shadow
In figure below, we can see there is green shadow on the board, which doesn't make sence as it should be the shadow of the red ball.
The reason is, in phong illumination, when apply 
$${diffuse =  light * diffuse_factor *n*l}$$
if the sign of n*l not checked, then the rgb color can be negative. In this case, the R&B light is subtracted, which leads G greater than the other two, then the color goes green.

<img src=".\screen_shot\bad_green.bmp" width="40%">

