

bool WORLD[64][64]; // Creation of the world
bool WORLD_BUFFER[64][64];
int step_GOL; //used to know the generation


void setup_conway() {
  for (byte i = 0; i < 64; i++) {
    for (byte j = 0; j < 64; j++) {
      WORLD[i][j] = random(0, 2);
    }
  }
  step_GOL = 0;
  
  //Serial.println ("Random Conway Bitches");
}  

void conway_step() {
   for (byte i = 0; i < 64; i++) {
    for (byte j = 0; j < 64; j++) {
      
        if (i==0 or i==63 or j==0 or j==63) {
          WORLD_BUFFER [i][j]= 0; //No border life.
        }
        else {
        byte num_alive = WORLD[i - 1][j - 1] + WORLD[i - 1][j] + WORLD[i - 1][j + 1] + WORLD[i][j - 1] + WORLD[i][j + 1] + WORLD[i + 1][j - 1] + WORLD[i + 1][j] + WORLD[i + 1][j + 1];
        bool state = WORLD[i][j];

        //RULE#1 if you are surrounded by 3 cells --> you live
        if (num_alive == 3) {
          WORLD_BUFFER [i][j]= 1;
        }
        //RULE#2 if you are surrounded by 2 cells --> you stay in your state
        else if (num_alive == 2) {
          WORLD_BUFFER [i][j]= state;
        }
        //RULE#3 otherwise you die from overpopulation or subpopulation
        else {
          WORLD_BUFFER [i][j]= 0;
        }
      }
    }
  }

  //Updating the World
  for (byte i = 0; i < 64; i++) {
    for (byte j = 0; j < 64; j++) {
      WORLD[i][j] = WORLD_BUFFER[i][j];
    }
  }
}

void  conway_draw()
{
  int16_t v = 0;
  uint8_t wibble = sin8(myTime);
  
  for (byte j = 0; j < 64; j++) {
    for (byte i = 0; i < 64; i++) {
      if (WORLD[i][j] == 0) {
        matrix.drawPixelRGB888(j, i, 0,0,0);
      }
      else
      {        
        v += sin16(i * wibble * 3 + myTime);
        v += cos16(j * (128 - wibble)  + myTime);
        v += sin16(i * j * cos8(-myTime) / 8);
        currentColor = ColorFromPalette(currentPalette, (v >> 8) + 127); //127, brightness, currentBlendType);
        matrix.drawPixelRGB888(j, i,  currentColor.r, currentColor.g,currentColor.b);
      }
    }
  }
}



void DoConway() {

  if (step_GOL == 200) { // This if reboot the world after 60 generation to avoid static world
    step_GOL = 0;
    int x=0;int y=0;int y1=63; int x1=63;
    
    for (x=0;x<64;x++) {
      matrix.drawRect(x, y, x1, y1, matrix.color444(0, 0, 0));
      y++;y1--;x1--;
      delay (5);
    }

    
    matrix.fillScreen(0);
    delay(100);
    setup_conway();
  }
  
  conway_step();
  conway_draw();
  delay (50);
  
  step_GOL++;
  
}
