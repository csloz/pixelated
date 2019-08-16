float noiseScale =0.2;

float fractalNoise (float x, float y, float z) {
  float r=0;
  float amp=1.0;
  for (int octave=0; octave<4;octave ++){
      r+=noise(x)+noise(y)+noise(z) * amp;
      amp /=2;
      x*=2;
      y*=2;
      z*=2;
  }
  return r;
  
}

float noise (float val) {
    return (float (random( 0,1000)/1000.0));
}

float dx,dy,dz;

void DoRings() {

    long now = millis();
    float speed = 0.002;
    float zspeed= 0.1;
    float angle=sin (now * 0.001);
    float z= now * 0.00008;
    float hue= now *0.01;
    float scale=0.005;

    float saturation = 100* constrain(pow(1.15 * noise(now * 0.000122) ,2.5),0,1);
    float spacing = noise (now*0.000124) * 0.1;

    dx += cos(angle)*speed;
    dy += sin(angle)*speed;
    dz += (noise(now * 0.000014) -0.5) * zspeed;

    float centerx = noise (now * 0.000125) * 1.25 * COLUMNS;
    float centery = noise (now * -0.000125) * 1.25 * ROWS;

    for (int x=0; x< COLUMNS; x++) {
        for (int y=0; y< ROWS; y++){

            float dist = sqrt(pow(x - centerx,2) + pow(y - centery,2));
            float pulse = (sin (dz + dist* spacing) - 0.3) * 0.3;

            float n = fractalNoise (dx+ x*scale + pulse , dy+y*scale,z) - 0.75;
            float m = fractalNoise (dx + x*scale , dy + y* scale, z+10.0) - 0.75;


           // color c= color ( ( hue+ 40.0 * m ) % 100.0, saturation, 100* constrain(pow(3.0*n,1.5) ,0,0.9) );
           //matrix.drawPixelRGB888(x, y, red, green, blue);
           matrix.drawPixel (x,y,BLUE);
           // matrix.drawPixelRGB888 (x,y,int (( hue+ 40.0 * m ) % 100.0),int ( saturation) ,int ( 100* constrain(pow(3.0*n,1.5) ,0,0.9))  ); //c = color
        }
    }
  
}
