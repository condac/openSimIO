# openSimIO
Arduino based simulator I/O


## Goals
The goal with this project is to create a hardware layer that can chain multiple Arduinos together with serial communications to create a larger network. It allows you to have any kind of Arduino in the chain, lets say you have an DUE to do something it can relay it back to a master device that is a different kind, MEGA or UNO for example.

I also want to keep the code simple to make it easy for hoppyist like my self to contribute to this project. Code readability is prefered over performance in the early phase of this project.

The Arduinos should relay the information back to the PC without doing anything with the information to make things simple in the Arduino parts and to save on program memory. If for example a analog value need to be trimmed or calibrated it should be done in the plugin that relay the information to the game or software on the PC side.

## Example chain

```
Simulator Software
  |
plugin
  |
  | serial
  |
Master board (Arduino MEGA)
 |   |
 |   >--------------------
 |                        |
 |                        | serial
 |                        |
 |                      Arduino UNO with servos
 |                        |
 |                        |
 |                      Arduino NANO with buttons and lights
 |                        |                      
 |<------------------------
```

## Addressing
To make the system fully expandable there are 2 levels of addressing a arduino board. Master and Slave address.  
The Master is the board connected with serial or ethernet to the plugin, it have a uniqe id you must define in the code before uploading it to the arduino board.  
The slave address is created automatically by a ping that runs through all slaves on the serial chain. the first slave board on the chain get id 1.

The plugin or pc have id 99. 
