
#include "Arduino.h"
#include "ODriveArduino.h"
#include "ODriveErrors.h"

static const int kMotorOffsetFloat = 2;
static const int kMotorStrideFloat = 28;
static const int kMotorOffsetInt32 = 0;
static const int kMotorStrideInt32 = 4;
static const int kMotorOffsetBool = 0;
static const int kMotorStrideBool = 4;
static const int kMotorOffsetUint16 = 0;
static const int kMotorStrideUint16 = 2;

// Print with stream operator
template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }

ODriveArduino::ODriveArduino(Stream& serial)
    : serial_(serial) {}
    

void ODriveArduino::SetPosition(int motor_number, float position) {
    SetPosition(motor_number, position, 0.0f, 0.0f);
}

void ODriveArduino::SetPosition(int motor_number, float position, float velocity_feedforward) {
    SetPosition(motor_number, position, velocity_feedforward, 0.0f);
}

void ODriveArduino::SetPosition(int motor_number, float position, float velocity_feedforward, float current_feedforward) {
    serial_ << "p " << motor_number  << " " << position << " " << velocity_feedforward << " " << current_feedforward << "\n";
}

void ODriveArduino::SetVelocity(int motor_number, float velocity) {
    SetVelocity(motor_number, velocity, 0.0f);
}

void ODriveArduino::SetVelocity(int motor_number, float velocity, float current_feedforward) {
    serial_ << "v " << motor_number  << " " << velocity << " " << current_feedforward << "\n";
}

void ODriveArduino::SetCurrent(int motor_number, float current) {
    serial_ << "c " << motor_number << " " << current << "\n";
}

void ODriveArduino::TrapezoidalMove(int motor_number, float position){
    serial_ << "t " << motor_number << " " << position << "\n";
}

float ODriveArduino::readFloat() {
    return readString().toFloat();
}

float ODriveArduino::GetVelocity(int motor_number){
	serial_<< "r axis" << motor_number << ".encoder.vel_estimate\n";
	return ODriveArduino::readFloat();
}

int32_t ODriveArduino::readInt() {
    return readString().toInt();
}

bool ODriveArduino::run_state(int axis, int requested_state, bool wait) {
    int timeout_ctr = 100;
    serial_ << "w axis" << axis << ".requested_state " << requested_state << '\n';
    if (wait) {
        do {
            delay(100);
            serial_ << "r axis" << axis << ".current_state\n";
        } while (readInt() != AXIS_STATE_IDLE && --timeout_ctr > 0);
    }

    return timeout_ctr > 0;
}

String ODriveArduino::readString() {
    String str = "";
    static const unsigned long timeout = 1000;
    unsigned long timeout_start = millis();
    for (;;) {
        while (!serial_.available()) {
            if (millis() - timeout_start >= timeout) {
                return str;
            }
        }
        char c = serial_.read();
        if (c == '\n')
            break;
        str += c;
    }
    return str;
}

void  ODriveArduino::checkForErrors() {
  serial_ << "r axis0.error\n";
  axisError = readInt();
  serial_ << "r axis0.motor.error\n";
  motorError = readInt();
  serial_ << "r axis0.controller.error\n";
  controllerError = readInt();
  serial_ << "r axis0.encoder.error\n";
  encoderError = readInt();
  
   if(axisError != 0 || motorError != 0 || controllerError != 0 || encoderError != 0) {
    if(axisError != 0) {
      for(int i = 0;i < 12;i++) {
        if(axisError & 1<<i) {
          Serial << "Axis error: " << axisErrors[i] << '\n';
        }
      }
    }
    if(motorError != 0) {
      for(int i = 0;i < 13;i++) {
        if(motorError & 1<<i) {
          Serial << "Motor error: " << motorErrors[i] << '\n';
        }
      }
    }
    if(controllerError != 0) {
      Serial << "Controller error: " << controllerErrors[0] << '\n';
    }
    if(encoderError != 0) {
      for(int i = 0;i < 6;i++) {
        if(encoderError & 1<<i) {
          Serial << "Encoder error: " << encoderErrors[i] << '\n';
        }
      }
    }
  }
}

void  ODriveArduino::resetErrors(int motor_number) {
        serial_ << "w axis" << motor_number << ".error " << 0 << "\n";
        serial_ << "w axis" << motor_number <<".motor.error " << 0 << "\n";
        serial_ << "w axis" << motor_number << ".controller.error " << 0 << "\n";
        serial_ << "w axis" << motor_number << ".encoder.error " << 0 << "\n";
        //Serial.println(readString());
}