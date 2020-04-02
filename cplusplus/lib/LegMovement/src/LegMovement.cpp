#include "LegMovement.h"
#include "math.h"
#include <Arduino.h>

LegMovement::LegMovement(ODriveArduino &_odrive, int _leg_number)
    : odrive(_odrive), leg_number(_leg_number)
{
}

/**
 * returns the angle to put the motor in
 * @param x x value
 * @param y y value
 * @param motor motor 0 or 1
 * @param ODrive Odrive 0-3
*/
float LegMovement::compute(float x, float y, uint8_t motor)
{
  if ((leg_number == 0) || (leg_number == 2))
  {
    INNER = 0;
    OUTER = 1;
  }
  else
  {
    INNER = 1;
    OUTER = 0;
  }

  float alpha = 0;
  float r = sqrt((x * x) + (y * y));
  r = constrain(r, 80, 249);
  float theta = atan(x / y);
  float gamma = acos((8100 + (r * r) - 25600) / (180 * r));

  if (y < 0)
  {
    if (motor == INNER)
    {
      alpha = -gamma - theta;
    }
    else if (motor == OUTER)
    {
      alpha = gamma - theta;
    }
  }
  else if (y >= 0)
  {
    if (x == 0)
    {
      float offsetInRad;
      if ((leg_number == 0) || (leg_number == 2))
      {
        offsetInRad = -3.14;
      }
      else
      {
        offsetInRad = 3.14;
      }

      if (motor == INNER)
      {
        alpha = -gamma + offsetInRad;
      }
      else if (motor == OUTER)
      {
        alpha = gamma + offsetInRad;
      }
    }
    else if (x > 0)
    {
      if (motor == INNER)
      {
        alpha = -gamma + 1.57 + (1.57 - theta);
      }
      else if (motor == OUTER)
      {
        alpha = gamma + 1.57 + (1.57 - theta);
      }
    }
    else if (x < 0)
    {
      if (motor == INNER)
      {
        alpha = -gamma - 1.57 - (1.57 + theta);
      }
      else if (motor == OUTER)
      {
        alpha = gamma - 1.57 - (1.57 + theta);
      }
    }
  }
  alpha = alpha * 57.296;
  return alpha;
}

/**
 * Returns x
*/
float LegMovement::stepX(p &params, float phase_shift)
{
  if (leg_number == 0 || leg_number == 3)
  {
    this->x = params.step_right / 2 * sin(params.frequency * params.x + phase_shift);
  }
  else
  {
    this->x = params.step_left / 2 * sin(params.frequency * params.x + phase_shift);
  }

  return this->x;
}

/**
 * Returns y
*/
float LegMovement::stepY(p &params, float phase_shift)
{

  float wave = cos(params.frequency * params.x + phase_shift);
  if (wave > 0)
  {
    this->y = -params.height + params.amplitude_over * wave;
  }
  else
  {
    this->y = -params.height + params.amplitude_under * wave;
  }
  return this->y;
}

void LegMovement::linearMove(float x, float y, int velocity)
{
  for (int motor = 0; motor < 2; motor++)
  {
    double angle = this->compute(x, y, motor);
    double motor_count = map(angle, -360, 360, -6000, 6000);
    this->odrive.SetPosition1(motor, motor_count, 5000.000);
  }
}

void LegMovement::move(p &params, float phase_shift)
{
  double x = this->stepX(params, phase_shift);
  double y = this->stepY(params, phase_shift);
  for (int motor = 0; motor < 2; motor++)
  {
    double angle = this->compute(x, y, motor);
    double motor_count = map(angle, -360, 360, -6000, 6000);
    this->odrive.SetPosition1(motor, motor_count, 50000.000);
  }
}

float LegMovement::getX()
{
  return this->x;
}

float LegMovement::getY()
{
  return this->y;
}
