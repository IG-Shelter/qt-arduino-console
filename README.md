# Qt Arduino Console
1. Automatically send messages through serial port according to a JSON file.
2. Only search ports have "Arduino" in description (can be modified in `on_refresh_available_arduino_clicked()`)
```
Enviroment:
Qt 5.15.0
MSVC 2019 x64
```

## Setting Example
```
Serial.begin(19200,SERIAL_8E2);
Baud Rate: 19200
Data Bits: 8
Stop Bits: 2
Parity: Even Parity
```

## Arduino Console Mannual:
```
Type and press enter to send message.
Click Load to load work flow.
On: Start workflow
Off; Stop workflow
||: Pause
>|: Execute next step without waiting.
< : Step back.
> : Step forward.
```
## JSON workflow Example
```
{
  "Description":"Example",
  "Workflow":[
    {
      "Operation":"xxxx"
      "Time":0.5
      "Commands":[
        "xxxx",
        "xxxx"
      ]
    },
    {
      "Operation":"xxxx"
      "Time":100
      "Commands":[
        "xxxx",
        "xxxx",
        "xxxx"
      ]
    }
  ]
}
```
"Workflow" and "Commands" are scalable. For each operation/step, it will wait "Time" seconds to run next, while the commands are sent in batch.
