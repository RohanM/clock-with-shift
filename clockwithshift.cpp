/*
Alternate firmware for the ginky synthese grains eurorack module
Code by a773 (atte.dk) and released under the GPL licence
*/

/* 11-9-2021 Adapted by Jesse Stevens of artist duo Cake Industries for Look Mum No Computer offbeat shift needs*/
/* 16-10-2021 Further changes to allow for longer gaps between incoming beats and logic to handle multi/div changes between beats for Look Mum No Computer */

/*
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
*/

#include <math.h>
#include "testing.h"

#define TRIGGER_LENGTH 20
#define UPPER_POT       2
#define MIDDLE_POT      1
#define LOWER_POT       0
#define CLOCK_IN        3
#define UPPER_POT_MAX   500
#define MIDDLE_POT_MAX   500
#define LOWER_POT_MAX   500
#define NB_POT_SLICES 4
#define MODE_SIMPLE 0
#define MODE_COMPLEX 1
#define SHIFTED_OUT       11
#define UNSHIFTED_OUT     10

long variable_trigger_length;
long last_trigger_in = 0;
long last_trigger_out = 0;
unsigned long last_unshifted_trigger_out = 0;
long now = 0;
long time_between_ins = 0;
long time_between_outs = 0;
long time_between_unshifted_trigger_out = 0;
int  nb_triggs = 0;
int  nb_unshifted_triggs = 0;
int  edge_skipper = 1;

//offbeat shift stuff:
int offbeat_input = 5; //analog input 5
unsigned long scheduledshiftbegin = 0;
int delaystart = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//variables for reading the input knobs faster than at each beat
//last values we read from them:
int lastupper = 0;
int lastmiddle = 0;
//last time we read the knobs: (we need to only read them so often or the loop will be too slow)
unsigned long lastknobread = 0;
unsigned long knobreadinginterval = 500;
int upperreading;
int middlereading;
int lowerreading;
int reset;
float beatshift;
//wow ok this is getting messy, but this is adapted code from someone else, we'll do a clean at some point
///////////////////////////////////////////////////////////////////////////////////////////////////////////

bool in_clock_high = false;
bool stopped = false;
bool out_clock_high = false;
bool unshifted_out_clock_high = false;
bool edge = false;
bool getting_triggers = true;


int mode = -1;
float factor = 0;

int simple_factors[10] = {1,2,4,8,16,32,64,128,256,512};
int complex_factors[10] = {1,3,5,7,11,13,17,19,23,29};


int slice2factor(int slice, int mode){
  if(mode==MODE_SIMPLE){
    return simple_factors[slice];
  } else {
    return complex_factors[slice];
  }
}

int get_mult(int mode){
  int slice = analogRead(UPPER_POT) * (NB_POT_SLICES-1) / UPPER_POT_MAX;
  return slice2factor(slice,mode);
}

int get_multfast(int mode, int upperpotread){
  int slice = upperpotread * (NB_POT_SLICES-1) / UPPER_POT_MAX;
  return slice2factor(slice,mode);
}

int get_div(int mode){
  int slice = analogRead(MIDDLE_POT) * (NB_POT_SLICES-1) / MIDDLE_POT_MAX;
//  return 4;
  return slice2factor(slice,mode);
}

int get_divfast(int mode, int middlepotread){
  int slice = middlepotread * (NB_POT_SLICES-1) / MIDDLE_POT_MAX;
//  return 4;
  return slice2factor(slice,mode);
}

long get_time(){
  return millis();
}

void unshift_trigger(){
  digitalWrite(UNSHIFTED_OUT, HIGH);
  unshifted_out_clock_high = true;
  last_unshifted_trigger_out = millis();
}

void trigger(){
  digitalWrite(SHIFTED_OUT, HIGH);
  out_clock_high = true;
  last_trigger_out = millis();
}


void setup() {
  pinMode(CLOCK_IN, INPUT_PULLUP);
  pinMode(SHIFTED_OUT, OUTPUT);
  pinMode(UNSHIFTED_OUT, OUTPUT);

  upperreading = analogRead(UPPER_POT);
  middlereading = analogRead(MIDDLE_POT);
  lowerreading = analogRead(LOWER_POT);
  reset = analogRead(LOWER_POT);

  time_between_outs = time_between_ins / get_multfast(mode, upperreading);

  Serial::begin(115200);
}


void loop()
{
  //check for the incoming clock signal EVERY loop:
  int gate = digitalRead(CLOCK_IN);



  //keep a check on the time and reset the edge detection:
  now = get_time();
  edge = false;

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  // detect gate in
  if (gate == LOW){ //my setup is reverse logic trigger (using NPN transistor as buffer on input)
    if (!in_clock_high) {
      if(last_trigger_in != 0){
        time_between_ins = now - last_trigger_in;
      }
      getting_triggers = time_between_ins < 2500 ? true:false;
      last_trigger_in = now;

      if(!getting_triggers){
        edge_skipper = 0;
      }
      else {
        edge_skipper = (edge_skipper + 1) % get_divfast(mode, middlereading);
      }
      if(edge_skipper == 0){
        edge = true;

      }

    }
    in_clock_high = true;

  }
  else {
    in_clock_high = false;
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  // detect reset/mode
  if (reset < LOWER_POT_MAX/3) {
    // CCW simple mode
    if(edge){
      mode = MODE_SIMPLE;
      stopped = false;
    }
  }
  else if(reset > LOWER_POT_MAX/3*2){
    // CW complex mode
    if(edge){
      mode = MODE_COMPLEX;
      stopped = false;
    }
  }
  else {
    // stopped
    stopped = true;
    last_trigger_out = 0;
  }

  // setup mult and div
  if(edge && getting_triggers){
    // only update if triggers faster than 2.5 seconds
    time_between_outs = time_between_ins / get_multfast(mode, upperreading);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // We'll only read our analog input knobs every so often so we don't slow down the timing loop:
  // Let's call this area the brain = burned toast zone
  // We use this to also detect if knobs have been changed and recalculate beat timing without losing
  // rhythm (hopefully)
  /////////////////////////////////////////////////////////////////////////////////////////////////
  if ( (millis() - lastknobread) >= knobreadinginterval){

    reset = analogRead(LOWER_POT);

    //may as well read from the offbeat shift input while the other analog inputs are being read:
    beatshift = analogRead(offbeat_input); //range of 0 - 1023

    // middle pot handles the division
    middlereading = analogRead(MIDDLE_POT);
    lastmiddle = middlereading;
    //there doesn't look to be anything to recalculate if this knob is turned, it gets calculated in the rest of the body as it rolls


    // upper pot handles the multiplier
    upperreading = analogRead(UPPER_POT);
    //if the upper knob has changed much:
    if ( (upperreading - lastupper) > 20 || (upperreading - lastupper < -20) ){
      lastupper = upperreading;

      //only do this recalc if we're not currently working on an incoming beat:
      if (!edge){

      //recalculate the time between outs cut into pieces by the multiplier
      time_between_outs = time_between_ins / get_multfast(mode, upperreading);
      //now we need to recalculate our triggers to keep synced with tempo:


      //we'll divide the time since the last beat input by the time between outs,
      //and round down to the nearest number to say what number trigger we should have recently passed
      nb_unshifted_triggs = 1 + floor( (millis() - last_trigger_in) / ( (time_between_outs) * get_divfast(mode, middlereading)) );

      //now we'll fake having triggered the last time so we keep in tempo:
      last_unshifted_trigger_out = last_trigger_in + ((nb_unshifted_triggs-1) * (time_between_outs * get_divfast(mode, middlereading)));

      Serial::print("unshifted trigs: ");
      Serial::println(nb_unshifted_triggs);

      Serial::print("last unshifted trigger out: ");
      Serial::print(last_unshifted_trigger_out);
      Serial::print("             Current time: ");
      Serial::println(millis());
      Serial::print("Calculated gap: ");
      Serial::println((time_between_outs * get_divfast(mode, middlereading)));


      if (beatshift < 30){  //if we're not shifting:
        //make the beatshift output match the regular unshifted output:
        nb_triggs = nb_unshifted_triggs;
        last_trigger_out = last_unshifted_trigger_out;
      }

      //if we are shifting:
      else if (beatshift >= 30 && delaystart == 0){ //if we're shifting the beat and we're not currently delaying the first output:
        //we want to:
        // number of triggers = round down to nearest whole number: [ current time - (last trigger in + shift amount) ] / (time_between_outs * get_divfast(mode, middlereading)
        nb_triggs = floor( (millis()- (last_trigger_in + ((time_between_outs * get_divfast(mode, middlereading)) * (float(map(beatshift, 30, 1023, 0, 99)) / 100)) )) / time_between_outs);
        //now we'll fake having triggered the last time so we keep in tempo:
        // last trigger out = number of triggers completed x time between outs (adjusted by divider knob) + shifting offset
        last_trigger_out = last_trigger_in + (nb_triggs * (time_between_outs * get_divfast(mode, middlereading))) + ((time_between_outs * get_divfast(mode, middlereading)) * (float(map(beatshift, 30, 1023, 0, 99)) / 100));
      }

      else if (beatshift >= 30 && delaystart == 1){ //if we're shifting the beat, but we also haven't yet hit the first scheduled delay beat:
        //we want to:
        //
        scheduledshiftbegin = last_trigger_in + ((time_between_outs * get_divfast(mode, middlereading)) * (float(map(beatshift, 30, 1023, 0, 99)) / 100) );
      }

      //nb_unshifted_triggs++;
      //nb_triggs++;
      }

    }



  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////////////
  // if we're getting sensible time (above zero) and we're not stopped,
  // then it's time to process triggers:
  ///////////////////////////////////////////////////////////////////////////////
  if(time_between_ins > 0 && !stopped){
  /*  if(nb_unshifted_triggs < 1){
      Serial::print("                                     FIRE! ");
     Serial::println(nb_unshifted_triggs);
    }
  */
    /*if( (last_unshifted_trigger_out - millis()) > time_between_outs){
      //Serial::println("                            WHY ARE WE WAITING!?");
      Serial::print("time between outs: ");
      Serial::println(time_between_outs);
      Serial::print("last trigger out: ");
      Serial::println(last_unshifted_trigger_out);
      Serial::print("       triggs: ");
      Serial::println(nb_unshifted_triggs);
    }*/


    ////////////////////////////////////
    //let's do the unshifted elements:
    ////////////////////////////////////
    if(nb_unshifted_triggs <= 1 && edge){
      unshift_trigger();
      nb_unshifted_triggs = get_multfast(mode, upperreading);
    }
    else if( (millis() - last_unshifted_trigger_out) >= time_between_outs ){
       if(nb_unshifted_triggs >= 1){
         unshift_trigger();
         nb_unshifted_triggs--;
         //last_unshifted_trigger_out = millis();
         Serial::print("       triggs: ");
         Serial::println(nb_unshifted_triggs);
         Serial::print("last trigger out: ");
         Serial::println(last_unshifted_trigger_out);
         Serial::print("current time: ");
         Serial::println(millis());
       }
     }


    /////////////////////////////////
    //let's do the shifted elements:
    /////////////////////////////////
    if(nb_triggs <= 1 && edge){
      //let's trigger on the beat if we've set the offbeat knob to zero
      //(with a tiny bit of slack at the bottom in case the potentiometer isn't perfect):
      if(beatshift < 30){
        delaystart = 0;
        trigger();
        nb_triggs = get_multfast(mode, upperreading);
      }
      //however, if the shift knob is turned up, let's schedule the first trigger according to shift time
      else if (delaystart == 0 && beatshift >= 30){
        //set this variable so we can trigger the delay start
        delaystart = 1;
        //time to start       current time + [ ( time between outs x division knob ) x fraction of 1 output beat (shift between 0 and 1)
        scheduledshiftbegin = millis() + ((time_between_outs * get_divfast(mode, middlereading)) * (float(map(beatshift, 30, 1023, 0, 99)) / 100) );
      }
    }

    //we want to only trigger when it's been the trigger time, multiplied by a
    //mapping of the input from 30-1023, to 1-2 so we should get a multiplication between 1 and 2
    //which in effect turns our input potentiometer into a way to phase shift between being on beat, to being a beat behind
    else if ( (now - last_trigger_out) >=  (time_between_outs * get_divfast(mode, middlereading)) ){
      if(nb_triggs > 1){
        trigger();
        nb_triggs--;
      }
    }
  ////////////////////////////////////////////
  //some logic to handle delayed first beat:
  ////////////////////////////////////////////
  if (delaystart == 1){
    if (millis() >= scheduledshiftbegin){
      trigger();
      nb_triggs = get_multfast(mode, upperreading);
      delaystart = 0;
    }
  }
  }




  /////////////////////////////////////////////////////
  //Trigger length handling
  /////////////////////////////////////////////////////
  // reduce trigger length if going real fast
  if(time_between_outs <= TRIGGER_LENGTH * 2){
    variable_trigger_length = time_between_outs / 2;
  } else {
    variable_trigger_length = TRIGGER_LENGTH;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //SHIFTED Trigger handling
  /////////////////////////////
  // turn trigger out low
  if( ((now - last_trigger_out) > variable_trigger_length) && out_clock_high){
    digitalWrite(SHIFTED_OUT, LOW);
    out_clock_high = false;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //UNSHIFTED Trigger handling
  /////////////////////////////

  // turn trigger out low
  if( ((millis() - last_unshifted_trigger_out) > variable_trigger_length) && unshifted_out_clock_high){
    digitalWrite(UNSHIFTED_OUT, LOW);
    unshifted_out_clock_high = false;
  }


}
