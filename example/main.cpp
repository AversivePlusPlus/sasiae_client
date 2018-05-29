/*
    Aversive++                                                           
    Copyright (C) 2014 Eirbot                                            
                                                                         
    This program is free software: you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or    
    (at your option) any later version.                                  
                                                                         
    This program is distributed in the hope that it will be useful,      
    but WITHOUT ANY WARRANTY; without even the implied warranty of       
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        
    GNU General Public License for more details.                         
                                                                         
    You should have received a copy of the GNU General Public License    
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <sasiae/device/motor.hpp>
#include <sasiae/device/encoder.hpp>

using namespace SASIAE;

// Encoder devices
Encoder left_enc("leftEnc");
Encoder right_enc("rightEnc");

// Motor devices
Motor left_motor("leftMot");
Motor right_motor("rightMot");

// User code
int main(int, char**) {
  SASIAE::Aversive::init();

  left_motor.put(100);
  right_motor.put(100);

  while(SASIAE::Aversive::sync()) {
    int32_t left_dist = left_enc.get();
    int32_t right_dist = right_enc.get();
  }

  return 0;
}

