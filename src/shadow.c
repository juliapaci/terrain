#include "shadow.h"
#include "perlin.h"

#include <raylib.h>
#include <stdio.h>
#include <math.h>

void cast(int x, int y, int length, int interval, bool **map) {
    for(int i = 0; i < 360/interval; i++) {
        const float theta = i*interval*(PI/180.);
        float cos_theta = cosf(theta);
        if(theta > PI/2. && theta < 3*PI/2.)
            cos_theta = cosf(PI - theta);

        bool see = true;
        for(int j = 0; j < length; j++) {
            int py = sqrt(2*j*j-2*j*j*cos_theta);
            int px = j*cos_theta;
            if(theta > PI/2. && theta < 3*PI/2.)
                px *= -1;
            if(theta > PI && theta < 2*PI)
                py *= -1;
            px += x;
            py += y;

            DrawRectangle(px, py, 1, 1, see ? PURPLE : GRAY);
            if(see && (px > 0 && px < WIDTH) && (py > 0 && py < HEIGHT) && map[px][py]) {
                DrawCircle(px, py, 6, PURPLE);
                see = false;
            }
        }
    }
}
