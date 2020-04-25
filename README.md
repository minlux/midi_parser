# MIDI Parser
Convert midi file to C-code lookup-table of signal duration and frequency.

```
const uint16_t fur_elise[] = { //2x16-bit value pair : Duration [1ms], Frequeny [1Hz]
  122, 110,   119, 164,   5, 0,       119, 220,   382, 0,     122, 82,   119, 164,   7, 0,
  122, 207,   377, 0,     122, 110,   2, 0,       122, 164,   2, 0,      122, 220,   1127, 0,
  122, 110,   119, 164,   5, 0,       119, 220,   382, 0,     122, 82,   2, 0,       122, 164,
  ...
```


## Build
The build process of *midi_parser* is based on cmake:
```
cd build
cmake ..
make
```

## Usage
```
midi_parser -i <input> [-o <output>]
```

__Example__
Download a "single track midi file" - for example, Beethoven's "Für Elise" [1]. Then call *midi_parser* to conver that mide file into an C-constant table for { frquency, duration }-tupels.
```
midi_parser -i elise.mid -o elise.c
```

See [elise.c](out/elise.c) for an example of the produced output!


## Demo file
[1] https://bitmidi.com/fur-elise-mid

